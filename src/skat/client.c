#include "skat/client.h"
#include "skat/connection.h"
#include "skat/util.h"
#include <netdb.h>
#include <netinet/in.h>
#include <skat/ctimer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void
client_acquire_state_lock(client *c) {
  DPRINTF_COND(DEBUG_LOCK, "Acquiring client state lock from thread %ld",
			   pthread_self());
  pthread_mutex_lock(&c->lock);
  DPRINTF_COND(DEBUG_LOCK, "Acquired client state lock from thread %ld",
			   pthread_self());
}

void
client_release_state_lock(client *c) {
  DPRINTF_COND(DEBUG_LOCK, "Releasing client state lock from thread %ld",
			   pthread_self());
  pthread_mutex_unlock(&c->lock);
  DPRINTF_COND(DEBUG_LOCK, "Released client state lock from thread %ld",
			   pthread_self());
}

typedef struct {
  client *c;
  int socket_fd;
  int resume;
} client_conn_args;

static void *
client_conn_thread(void *args) {
  connection_c2s *conn;
  client_conn_args *cargs = args;
  conn = establish_connection_client(cargs->c, cargs->socket_fd, pthread_self(),
									 cargs->resume);
  if (!conn) {
	close(cargs->socket_fd);
	return NULL;
  }
  for (;;) {
	if (!conn_handle_incoming_packages_client(cargs->c, conn)) {
	  return NULL;
	}
	conn_handle_events_client(conn);
  }
}

static void
start_client_conn(client *c, const char *host, int p, int resume) {
  /* Obtain address(es) matching host/port */

  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;       /* Allow IPv4 */
  hints.ai_socktype = SOCK_STREAM; /* TCP socket */
  hints.ai_flags = 0;
  hints.ai_protocol = 0; /* Any protocol */

  char port_str[6];
  sprintf(port_str, "%d", p);

  struct addrinfo *result;
  int error = getaddrinfo(host, port_str, &hints, &result);
  if (error != 0) {
	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
	exit(EXIT_FAILURE);
  }

  /* getaddrinfo() returns a list of address structures.
	  Try each address until we successfully connect(2).
	  If socket(2) (or connect(2)) fails, we (close the socket
	  and) try the next address. */

  int socket_fd;
  struct addrinfo *rp;
  for (rp = result; rp != NULL; rp = rp->ai_next) {
	socket_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
	if (socket_fd == -1) {
	  continue;
	}

	if (connect(socket_fd, rp->ai_addr, rp->ai_addrlen) != -1) {
	  break; /* Success */
	}

	close(socket_fd);
  }

  if (rp == NULL) { /* No address succeeded */
	fprintf(stderr, "Could not connect\n");
	exit(EXIT_FAILURE);
  }

  freeaddrinfo(result); /* No longer needed */

  DEBUG_PRINTF("Established connection on socket %d", socket_fd);

  client_conn_args *args = malloc(sizeof(client_conn_args));
  args->c = c;
  args->socket_fd = socket_fd;
  args->resume = resume;

  pthread_create(&c->conn_thread, NULL, client_conn_thread, args);
}

void
client_tick(client *c) {
  DPRINTF_COND(DEBUG_TICK, "Client tick");

  client_acquire_state_lock(c);

  /*
  action a;
  event err_ev;
  for (int i = 0; i < s->ncons; i++) {
	if (!s->conns[i].c.active)
	  continue;

	while (conn_dequeue_action(&s->conns[i].c, &a)) {
	  if (!skat_state_apply(&s->skat_state, &a, &s->ps[i], s)) {
		DEBUG_PRINTF("Received illegal action of type %s from player %s with "
					 "id %ld, rejecting",
					 action_name_table[a.type], s->ps[i].id.str, a.id);
		err_ev.type = EVENT_ILLEGAL_ACTION;
		err_ev.answer_to = a.id;
		copy_player_id(&err_ev.player, &s->ps[i].id);
		conn_enqueue_event(&s->conns[i].c, &err_ev);
	  }
	}
	skat_state_tick(&s->skat_state, s);
  }
   */
  client_release_state_lock(c);
}

void
client_disconnect_connection(client *c, connection_c2s *conn) {
  DERROR_PRINTF("Lost connection to server");
  exit(EXIT_FAILURE);
}

void
client_handle_resync(package *p) {
  DTODO_PRINTF("TODO: implement resync on client side");// TODO: implement
}

void
client_init(client *c, char *host, int port, char *name) {
  DEBUG_PRINTF("Initializing client '%s' for server '%s:%d'", name, host, port);
  c->host = host;
  c->port = port;
  c->name = name;
}

static void
client_tick_wrap(void *c) {
  client_tick(c);
}

_Noreturn void
client_run(client *c, int resume) {
  ctimer t;

  ctimer_create(&t, c, client_tick_wrap,
				(1000 * 1000 * 1000) / CLIENT_REFRESH_RATE);// in Hz

  DEBUG_PRINTF("Running client with with connection mode '%s'",
			   resume ? "resume" : "new");
  client_acquire_state_lock(c);
  start_client_conn(c, c->host, c->port, resume);
  client_release_state_lock(c);

  ctimer_run(&t);
}
