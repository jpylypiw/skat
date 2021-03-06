#include "skat/ctimer.h"
#include "skat/util.h"
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <time.h>

static void
ctimer_tick(sigval_t sv) {
  ctimer *t = sv.sival_ptr;
  int noverruns = timer_getoverrun(t->timer_id);
  for (int i = 0; i < noverruns + 1; i++)
	sem_post(&t->activations);
}

void
ctimer_create(ctimer *t, const char *name, void *arg, void (*timerf)(void *),
			  long nsecs) {
  struct sigevent sev;

  DEBUG_PRINTF("Creating timer");

  if (nsecs < 0 || nsecs > 999999999) {
	DERROR_PRINTF("nsecs out of range");
	exit(EXIT_FAILURE);
  }

  strncpy(t->name, name, THREAD_NAME_SIZE);
  t->name[THREAD_NAME_SIZE - 1] = '\0';

  t->close = 0;
  t->timerf = timerf;
  t->arg = arg;
  t->nsecs = nsecs;
  sem_init(&t->activations, 0, 0);

  sev.sigev_notify = SIGEV_THREAD;
  sev.sigev_value.sival_ptr = t;
  sev.sigev_notify_function = ctimer_tick;
  sev.sigev_notify_attributes = NULL;

  ERRNO_CHECK(timer_create(CLOCK_REALTIME, &sev, &t->timer_id));
}

static void *
ctimer_handler(void *arg) {
  ctimer *t = arg;
  while (!t->close) {
	sem_wait(&t->activations);
	t->timerf(t->arg);
  }
  return NULL;
}

void
ctimer_run(ctimer *t) {
  struct itimerspec itspec;

  DEBUG_PRINTF("Setting timer to %ld ns", t->nsecs);
  itspec.it_interval.tv_nsec = t->nsecs;
  itspec.it_interval.tv_sec = 0;
  itspec.it_value.tv_nsec = 0;
  itspec.it_value.tv_sec = 1;

  ERRNO_CHECK(timer_settime(t->timer_id, 0, &itspec, NULL));

  pthread_create(&t->tid, NULL, ctimer_handler, t);
  thread_set_name(t->tid, "%s", t->name);

  DEBUG_PRINTF("Started Timer");
}

void
ctimer_stop(ctimer *t) {
  struct itimerspec itspec;

  t->close = 1;
  pthread_join(t->tid, NULL);

  itspec.it_interval.tv_nsec = 0;
  itspec.it_interval.tv_sec = 0;
  itspec.it_value.tv_nsec = 0;
  itspec.it_value.tv_sec = 0;

  ERRNO_CHECK(timer_settime(t->timer_id, 0, &itspec, NULL));
  timer_delete(t->timer_id);

  sem_destroy(&t->activations);
}
