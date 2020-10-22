#include "skat/skat.h"
#include "skat/server.h"
#include "skat/util.h"
#include <skat/client.h>
#include <string.h>

#undef SKAT_HDR
#define GAME_PHASE_HDR_TO_STRING

#include "skat/skat.h"

void
skat_state_notify_disconnect(skat_server_state *ss, player *pl, server *s) {
  DTODO_PRINTF("TODO: implement notify_disconnect");// TODO: implement
}

void
skat_state_notify_join(skat_server_state *ss, player *pl, server *s) {
  DTODO_PRINTF("TODO: implement notify_join");// TODO: implement
}

void
skat_calculate_game_result(skat_server_state *ss, int *score) {
  DTODO_PRINTF("TODO: implement calculate_game_result");// TODO: implement
}

static void
get_player_hand(skat_server_state *ss, player *pl, card_collection *col) {
  for (int i = 0; i < 3; ++i) {
	if (ss->sgs.active_players[i] == pl->index) {
	  *col = ss->player_hands[i];
	  return;
	}
  }

  card_collection_empty(col);
}

// Conforming to the rules. Poggers.
static int
distribute_cards(skat_server_state *ss) {
  card_collection draw_pile;
  card_collection_fill(&draw_pile);

  card_id cid;
  for (int i = 0; i < 3; i++) {
	for (int j = 0; j < 3; j++) {
	  card_collection_draw_random(&draw_pile, &cid);
	  card_collection_add_card(&ss->player_hands[i], &cid);
	  card_collection_remove_card(&draw_pile, &cid);
	}
  }

  for (int i = 0; i < 2; i++) {
	card_collection_draw_random(&draw_pile, &cid);
	ss->skat[i] = cid;
	card_collection_remove_card(&draw_pile, &cid);
  }

  for (int i = 0; i < 3; i++) {
	for (int j = 0; j < 4; j++) {
	  card_collection_draw_random(&draw_pile, &cid);
	  card_collection_add_card(&ss->player_hands[i], &cid);
	  card_collection_remove_card(&draw_pile, &cid);
	}
  }

  for (int i = 0; i < 3; i++) {
	for (int j = 0; j < 3; j++) {
	  card_collection_draw_random(&draw_pile, &cid);
	  card_collection_add_card(&ss->player_hands[i], &cid);
	  card_collection_remove_card(&draw_pile, &cid);
	}
  }

  return 0;
}
static int
debug_distribute_cards(skat_server_state *ss) {
  card_get_id(&(card){.cc = COLOR_KARO, .ct = CARD_TYPE_7}, &ss->skat[0]);
  card_get_id(&(card){.cc = COLOR_KARO, .ct = CARD_TYPE_8}, &ss->skat[1]);

  card_id hand[10];

  card_get_id(&(card){.cc = COLOR_KARO, .ct = CARD_TYPE_9}, &hand[0]);
  card_get_id(&(card){.cc = COLOR_KARO, .ct = CARD_TYPE_D}, &hand[1]);
  card_get_id(&(card){.cc = COLOR_KARO, .ct = CARD_TYPE_K}, &hand[2]);
  card_get_id(&(card){.cc = COLOR_KARO, .ct = CARD_TYPE_10}, &hand[3]);
  card_get_id(&(card){.cc = COLOR_KARO, .ct = CARD_TYPE_A}, &hand[4]);
  card_get_id(&(card){.cc = COLOR_HERZ, .ct = CARD_TYPE_7}, &hand[5]);
  card_get_id(&(card){.cc = COLOR_HERZ, .ct = CARD_TYPE_8}, &hand[6]);
  card_get_id(&(card){.cc = COLOR_HERZ, .ct = CARD_TYPE_9}, &hand[7]);
  card_get_id(&(card){.cc = COLOR_HERZ, .ct = CARD_TYPE_D}, &hand[8]);
  card_get_id(&(card){.cc = COLOR_HERZ, .ct = CARD_TYPE_K}, &hand[9]);
  card_collection_add_card_array(&ss->player_hands[0], hand, 10);

  card_get_id(&(card){.cc = COLOR_HERZ, .ct = CARD_TYPE_10}, &hand[0]);
  card_get_id(&(card){.cc = COLOR_HERZ, .ct = CARD_TYPE_A}, &hand[1]);
  card_get_id(&(card){.cc = COLOR_PIK, .ct = CARD_TYPE_7}, &hand[2]);
  card_get_id(&(card){.cc = COLOR_PIK, .ct = CARD_TYPE_8}, &hand[3]);
  card_get_id(&(card){.cc = COLOR_PIK, .ct = CARD_TYPE_9}, &hand[4]);
  card_get_id(&(card){.cc = COLOR_PIK, .ct = CARD_TYPE_D}, &hand[5]);
  card_get_id(&(card){.cc = COLOR_PIK, .ct = CARD_TYPE_K}, &hand[6]);
  card_get_id(&(card){.cc = COLOR_PIK, .ct = CARD_TYPE_10}, &hand[7]);
  card_get_id(&(card){.cc = COLOR_PIK, .ct = CARD_TYPE_A}, &hand[8]);
  card_get_id(&(card){.cc = COLOR_KREUZ, .ct = CARD_TYPE_7}, &hand[9]);
  card_collection_add_card_array(&ss->player_hands[1], hand, 10);

  card_get_id(&(card){.cc = COLOR_KREUZ, .ct = CARD_TYPE_8}, &hand[0]);
  card_get_id(&(card){.cc = COLOR_KREUZ, .ct = CARD_TYPE_9}, &hand[1]);
  card_get_id(&(card){.cc = COLOR_KREUZ, .ct = CARD_TYPE_D}, &hand[2]);
  card_get_id(&(card){.cc = COLOR_KREUZ, .ct = CARD_TYPE_K}, &hand[3]);
  card_get_id(&(card){.cc = COLOR_KREUZ, .ct = CARD_TYPE_10}, &hand[4]);
  card_get_id(&(card){.cc = COLOR_KREUZ, .ct = CARD_TYPE_A}, &hand[5]);
  card_get_id(&(card){.cc = COLOR_KARO, .ct = CARD_TYPE_B}, &hand[6]);
  card_get_id(&(card){.cc = COLOR_HERZ, .ct = CARD_TYPE_B}, &hand[7]);
  card_get_id(&(card){.cc = COLOR_PIK, .ct = CARD_TYPE_B}, &hand[8]);
  card_get_id(&(card){.cc = COLOR_KREUZ, .ct = CARD_TYPE_B}, &hand[9]);
  card_collection_add_card_array(&ss->player_hands[2], hand, 10);

  return 0;
}

static game_phase
apply_action_setup(skat_server_state *ss, action *a, player *pl, server *s) {
  event e;
  e.answer_to = a->id;
  e.acting_player = pl->index;
  switch (a->type) {
	case ACTION_READY:
	  if (s->ncons < 3) {
		DEBUG_PRINTF("Rejecting action ACTION_READY with id %ld by player %s "
					 "because "
					 "s->ncons = %d < 3",
					 a->id, pl->name.name, s->ncons);
		return GAME_PHASE_INVALID;
	  }

	  e.type = EVENT_START_GAME;
	  server_distribute_event(s, &e, NULL);

	  return GAME_PHASE_BETWEEN_ROUNDS;
	case ACTION_RULE_CHANGE:
	default:
	  return GAME_PHASE_INVALID;
  }
}

static game_phase
apply_action_between_rounds(skat_server_state *ss, action *a, player *pl,
							server *s) {
  int pm, ix;
  event e;
  e.answer_to = a->id;
  e.acting_player = pl->index;
  switch (a->type) {
	case ACTION_READY:
	  if (s->ncons < 3) {
		DEBUG_PRINTF("Rejecting action ACTION_READY with id %ld by player %s "
					 "because "
					 "s->ncons = %d < 3",
					 a->id, pl->name.name, s->ncons);

		return GAME_PHASE_INVALID;
	  }

	  e.answer_to = -1;
	  e.acting_player = -1;
	  e.type = EVENT_START_ROUND;

	  if (ss->sgs.active_players[0] == -1) {
		for (int i = 0, j = 0; i < 4; i++)
		  if ((s->playermask >> i) & 1)
			ss->sgs.active_players[j++] = s->ps[i]->index;
	  } else if (s->ncons == 3) {// we don't have a spectator
		perm(ss->sgs.active_players, 3, 0x12);
	  } else {
		pm = 0;
		for (int i = 0; i < 3; i++)
		  pm |= 1 << ss->sgs.active_players[i];
		ix = __builtin_ctz(~pm);
		perm(ss->sgs.active_players, 3, 0x12);
		ss->sgs.active_players[2] = s->ps[ix]->index;
	  }

	  memcpy(e.current_active_players, ss->sgs.active_players, 3 * sizeof(int));

	  server_distribute_event(s, &e, NULL);

	  ss->sgs.curr_stich =
			  (stich){.played_cards = 0, .vorhand = 0, .winner = -1};
	  ss->sgs.last_stich =
			  (stich){.played_cards = 0, .vorhand = -1, .winner = -1};
	  ss->sgs.stich_num = 0;
	  ss->sgs.alleinspieler = -1;
	  ss->spielwert = -1;
	  memset(ss->stiche, '\0', 3 * sizeof(ss->stiche[0]));
	  card_collection_empty(&ss->stiche_buf[0]);
	  card_collection_empty(&ss->stiche_buf[1]);
	  card_collection_empty(&ss->stiche_buf[2]);

	  // distribute_cards(ss);
	  // distributing manually for debugging:
	  debug_distribute_cards(ss);

	  DEBUG_PRINTF("Player hands: %#x, %#x, %#x", ss->player_hands[0],
				   ss->player_hands[1], ss->player_hands[2]);
	  DEBUG_PRINTF("Skat: %u & %u", ss->skat[0], ss->skat[1]);

	  e.type = EVENT_DISTRIBUTE_CARDS;

	  void mask_hands(event * ev, player * pl) {
		get_player_hand(ss, pl, &ev->hand);
	  }

	  server_distribute_event(s, &e, mask_hands);

	  DTODO_PRINTF("TODO: implement reizen");// TODO: implement reizen
	  // return GAME_PHASE_REIZEN_BEGIN;
	  ss->spielwert = 18;
	  ss->stiche[0] = &ss->stiche_buf[0];
	  ss->stiche[1] = &ss->stiche_buf[1];
	  ss->stiche[2] = &ss->stiche_buf[1];
	  card_collection_add_card_array(ss->stiche[0], ss->skat, 2);
	  ss->sgs.alleinspieler = 0;
	  ss->sgs.gr = (game_rules){.type = GAME_TYPE_COLOR, .trumpf = COLOR_KREUZ};
	  ss->sgs.rr = (reiz_resultat){.reizwert = 18,
								   .hand = 0,
								   .schneider = 0,
								   .schwarz = 0,
								   .ouvert = 0,
								   .contra = 0,
								   .re = 0};

	  e.type = EVENT_TEMP_REIZEN_DONE;
	  e.skat[0] = 0;
	  e.skat[1] = 0;

	  void mask_skat(event * ev, player * pl) {
		if (ss->sgs.active_players[ss->sgs.alleinspieler] == pl->index) {
		  memcpy(ev->skat, ss->skat, sizeof(ev->skat));
		}
	  }

	  server_distribute_event(s, &e, mask_skat);

	  return GAME_PHASE_PLAY_STICH_C1;
	default:
	  return GAME_PHASE_INVALID;
  }
}

static game_phase
apply_action_reizen_begin(skat_server_state *ss, action *a, player *pl,
						  server *s) {
  // remember to initialize stiche!
  event e;
  e.answer_to = a->id;
  e.acting_player = pl->index;
  DTODO_PRINTF("TODO: implement reizen");// TODO: implement reizen
  switch (a->type) {
	default:
	  return GAME_PHASE_INVALID;
  }
}

static int
next_active_player(int player, int off) {
  return (player + off) % 3;
}

static game_phase
apply_action_stich(skat_server_state *ss, action *a, player *pl, server *s,
				   int ind) {
  event e;
  int curr, result;
  int winnerv;// indexed by vorhand + ap
  int winner; // indexed by ap

  switch (a->type) {
	case ACTION_PLAY_CARD:
	  curr = next_active_player(ss->sgs.curr_stich.vorhand, ind);
	  if (!player_equals_by_name(pl, server_get_player_by_gupid(
											 s, ss->sgs.active_players[curr])))
		return GAME_PHASE_INVALID;
	  if (stich_card_legal(&ss->sgs.gr, ss->sgs.curr_stich.cs, ind, &a->card,
						   &ss->player_hands[curr], &result)
		  || !result)
		return GAME_PHASE_INVALID;

	  e.type = EVENT_PLAY_CARD;
	  e.answer_to = a->id;
	  e.acting_player = pl->index;
	  e.card = a->card;
	  server_distribute_event(s, &e, NULL);

	  if (!ind) {
		ss->sgs.curr_stich.cs[0] = a->card;
		ss->sgs.curr_stich.played_cards = 1;
		return GAME_PHASE_PLAY_STICH_C2;
	  } else if (ind == 1) {
		ss->sgs.curr_stich.cs[1] = a->card;
		ss->sgs.curr_stich.played_cards = 2;
		return GAME_PHASE_PLAY_STICH_C3;
	  }

	  ss->sgs.curr_stich.cs[2] = a->card;
	  ss->sgs.curr_stich.played_cards = 3;

	  stich_get_winner(&ss->sgs.gr, &ss->sgs.curr_stich, &winnerv);// Sue me

	  winner = next_active_player(ss->sgs.curr_stich.vorhand, winnerv);
	  ss->sgs.curr_stich.winner = winner;

	  card_collection_add_card_array(ss->stiche[winner], ss->sgs.curr_stich.cs,
									 3);

	  e.type = EVENT_STICH_DONE;
	  e.answer_to = -1;
	  e.acting_player = -1;
	  e.stich_winner = ss->sgs.active_players[winner];
	  server_distribute_event(s, &e, NULL);

	  ss->sgs.last_stich = ss->sgs.curr_stich;
	  ss->sgs.curr_stich =
			  (stich){.vorhand = ss->sgs.last_stich.winner, .winner = -1};

	  if (ss->sgs.stich_num++ < 9)
		return GAME_PHASE_PLAY_STICH_C1;

	  skat_calculate_game_result(ss, e.score_round);

	  for (int i = 0; i < 3; i++)
		ss->sgs.score[ss->sgs.active_players[i]] += e.score_round[i];

	  DTODO_PRINTF("Send scores");// TODO: send scores

	  e.answer_to = -1;
	  e.type = EVENT_ROUND_DONE;
	  server_distribute_event(s, &e, NULL);

	  return GAME_PHASE_BETWEEN_ROUNDS;
	default:
	  return GAME_PHASE_INVALID;
  }
}

static game_phase
apply_action(skat_server_state *ss, action *a, player *pl, server *s) {
  DEBUG_PRINTF("Applying action %s in skat state %s",
			   action_name_table[a->type],
			   game_phase_name_table[ss->sgs.cgphase]);
  switch (ss->sgs.cgphase) {
	case GAME_PHASE_SETUP:
	  return apply_action_setup(ss, a, pl, s);
	case GAME_PHASE_BETWEEN_ROUNDS:
	  return apply_action_between_rounds(ss, a, pl, s);
	case GAME_PHASE_REIZEN_BEGIN:
	  return apply_action_reizen_begin(ss, a, pl, s);
	case GAME_PHASE_PLAY_STICH_C1:
	  return apply_action_stich(ss, a, pl, s, 0);
	case GAME_PHASE_PLAY_STICH_C2:
	  return apply_action_stich(ss, a, pl, s, 1);
	case GAME_PHASE_PLAY_STICH_C3:
	  return apply_action_stich(ss, a, pl, s, 2);
	default:
	  return GAME_PHASE_INVALID;
  }
}

int
skat_server_state_apply(skat_server_state *ss, action *a, player *pl,
						server *s) {
  DEBUG_PRINTF("Applying action %s by player '%s'", action_name_table[a->type],
			   pl->name.name);

  game_phase new;
  new = apply_action(ss, a, pl, s);
  if (new == GAME_PHASE_INVALID)
	return 0;
  ss->sgs.cgphase = new;
  return 1;
}

void
skat_server_state_tick(skat_server_state *ss, server *s) {}

int
skat_client_state_apply(skat_client_state *cs, event *e, client *c) {
  DTODO_PRINTF("Insert sanity checks.");
  char card_name[4];
  int my_active_player_index;
  switch (e->type) {
	case EVENT_START_GAME:
	  DEBUG_PRINTF("Starting game");

	  cs->sgs.cgphase = GAME_PHASE_BETWEEN_ROUNDS;
	  return 1;
	case EVENT_START_ROUND:
	  DEBUG_PRINTF("Starting round");

	  memcpy(cs->sgs.active_players, e->current_active_players,
			 sizeof(cs->sgs.active_players));

	  for (int i = 0; i < 3; ++i) {
		if (cs->sgs.active_players[i] == cs->my_index) {
		  cs->my_active_player_index = i;
		  break;
		}
	  }

	  return 1;
	case EVENT_DISTRIBUTE_CARDS:
	  DEBUG_PRINTF("Distributing cards");

	  cs->my_hand = e->hand;

	  cs->sgs.curr_stich =
			  (stich){.played_cards = 0, .vorhand = 0, .winner = -1};
	  cs->sgs.last_stich =
			  (stich){.played_cards = 0, .vorhand = -1, .winner = -1};
	  cs->sgs.stich_num = 0;
	  cs->sgs.alleinspieler = -1;

	  card_collection_empty(&cs->my_stiche);
	  cs->my_partner = -1;
	  cs->ist_alleinspieler = -1;

	  DTODO_PRINTF("TODO: implement reizen");// TODO: implement reizen
	  cs->sgs.cgphase = GAME_PHASE_REIZEN_BEGIN;
	  return 1;
	case EVENT_TEMP_REIZEN_DONE:
	  DEBUG_PRINTF("(Temp) Reizen done");

	  memcpy(cs->skat, e->skat, sizeof(cs->skat));

	  cs->sgs.alleinspieler = 0;
	  cs->sgs.gr = (game_rules){.type = GAME_TYPE_COLOR, .trumpf = COLOR_KREUZ};
	  cs->sgs.rr = (reiz_resultat){.reizwert = 18,
								   .hand = 0,
								   .schneider = 0,
								   .schwarz = 0,
								   .ouvert = 0,
								   .contra = 0,
								   .re = 0};

	  if (cs->my_active_player_index == cs->sgs.alleinspieler
		  || cs->sgs.gr.type == GAME_TYPE_RAMSCH) {
		cs->ist_alleinspieler = 1;
		cs->my_partner = cs->my_active_player_index;
	  } else {
		cs->ist_alleinspieler = 0;

		// 0,1 | 1,0, sum=1 -> 2
		// 0,2 | 2,0, sum=2 -> 1
		// 1,2 | 2,1, sum=3 -> 3

		int sum = cs->my_active_player_index + cs->sgs.alleinspieler;
		if (sum == 1)
		  cs->my_partner = 2;
		else if (sum == 2)
		  cs->my_partner = 1;
		else
		  cs->my_partner = 3;
	  }

	  if (c->cs.ist_alleinspieler && cs->sgs.gr.type != GAME_TYPE_RAMSCH)
		card_collection_add_card_array(&cs->my_stiche, cs->skat, 2);

	  cs->sgs.cgphase = GAME_PHASE_PLAY_STICH_C1;

	  return 1;
	case EVENT_PLAY_CARD:
	  card_get_name(&e->card, card_name);
	  DEBUG_PRINTF("%s (%d) played card %s",
				   c->pls[e->acting_player]->name.name, e->acting_player,
				   card_name);
	  if (c->cs.my_index == e->acting_player) {
		card_collection_remove_card(&cs->my_hand, &e->card);
	  }

	  if (cs->sgs.cgphase == GAME_PHASE_PLAY_STICH_C1) {
		cs->sgs.curr_stich.cs[0] = e->card;
		cs->sgs.curr_stich.played_cards = 1;
		for (int i = 0; i < 3; ++i) {
		  if (cs->sgs.active_players[i] == e->acting_player) {
			cs->sgs.curr_stich.vorhand = i;
			break;
		  }
		}
		cs->sgs.cgphase = GAME_PHASE_PLAY_STICH_C2;
	  } else if (cs->sgs.cgphase == GAME_PHASE_PLAY_STICH_C2) {
		cs->sgs.curr_stich.cs[1] = e->card;
		cs->sgs.curr_stich.played_cards = 2;
		cs->sgs.cgphase = GAME_PHASE_PLAY_STICH_C3;
	  } else if (cs->sgs.cgphase == GAME_PHASE_PLAY_STICH_C3) {
		cs->sgs.curr_stich.cs[2] = e->card;
		cs->sgs.curr_stich.played_cards = 3;
		cs->sgs.cgphase = GAME_PHASE_CLIENT_WAIT_STICH_DONE;
	  } else {
		DERROR_PRINTF("Invalid game phase %s for PLAY_CARD",
					  game_phase_name_table[cs->sgs.cgphase]);
		return 0;
	  }
	  return 1;
	case EVENT_STICH_DONE:
	  DEBUG_PRINTF("Stich done");

	  if (cs->sgs.cgphase != GAME_PHASE_CLIENT_WAIT_STICH_DONE) {
		DERROR_PRINTF("Invalid game phase %s for STICH_DONE",
					  game_phase_name_table[cs->sgs.cgphase]);
		return 0;
	  }

	  for (int i = 0; i < 3; ++i) {
		if (cs->sgs.active_players[i] == e->stich_winner) {
		  cs->sgs.curr_stich.winner = i;
		  break;
		}
	  }

	  if (cs->my_index == e->stich_winner
		  || (!cs->ist_alleinspieler
			  && cs->sgs.active_players[cs->my_partner] == e->stich_winner)) {
		card_collection_add_card_array(&cs->my_stiche, cs->sgs.curr_stich.cs,
									   3);
	  }

	  if (cs->sgs.stich_num++ < 9)
		cs->sgs.cgphase = GAME_PHASE_PLAY_STICH_C1;
	  else
		cs->sgs.cgphase = GAME_PHASE_CLIENT_WAIT_ROUND_DONE;

	  cs->sgs.last_stich = cs->sgs.curr_stich;
	  cs->sgs.curr_stich =
			  (stich){.vorhand = cs->sgs.last_stich.winner, .winner = -1};

	  return 1;
	case EVENT_ROUND_DONE:
	  DEBUG_PRINTF("Round done");

	  if (cs->sgs.cgphase != GAME_PHASE_CLIENT_WAIT_ROUND_DONE) {
		DERROR_PRINTF("Invalid game phase %s for ROUND_DONE",
					  game_phase_name_table[cs->sgs.cgphase]);
		return 0;
	  }

	  cs->sgs.cgphase = GAME_PHASE_BETWEEN_ROUNDS;

	  return 1;
	default:
	  DERROR_PRINTF(
			  "Trying to apply event %s, but it isn't implemented or illegal",
			  event_name_table[e->type]);
	  return 0;
  }
}

void
skat_client_state_tick(skat_client_state *cs, client *c) {}

void
skat_resync_player(skat_server_state *ss, skat_client_state *cs, player *pl) {
  memset(cs, '\0', sizeof(skat_client_state));

  cs->sgs = ss->sgs;

  get_player_hand(ss, pl, &cs->my_hand);

  cs->my_index = pl->index;

  cs->my_active_player_index = -1;
  for (int i = 0; i < 3; i++) {
	if (cs->sgs.active_players[i] == cs->my_index) {
	  cs->my_active_player_index = i;
	  break;
	}
  }

  if (cs->my_active_player_index != 0) {
	cs->my_stiche = *ss->stiche[cs->my_active_player_index];
  }

  if (cs->my_active_player_index == -1) {
	cs->ist_alleinspieler = -1;
	cs->my_partner = -1;
  } else if (cs->my_active_player_index == cs->sgs.alleinspieler
			 || cs->sgs.gr.type == GAME_TYPE_RAMSCH) {
	cs->ist_alleinspieler = 1;
	cs->my_partner = cs->my_active_player_index;
  } else {
	cs->ist_alleinspieler = 0;

	// 0,1 | 1,0, sum=1 -> 2
	// 0,2 | 2,0, sum=2 -> 1
	// 1,2 | 2,1, sum=3 -> 3

	int sum = cs->my_active_player_index + cs->sgs.alleinspieler;
	if (sum == 1)
	  cs->my_partner = 2;
	else if (sum == 2)
	  cs->my_partner = 1;
	else
	  cs->my_partner = 3;
  }

  if (cs->ist_alleinspieler == 1 && cs->sgs.gr.type != GAME_TYPE_RAMSCH) {
	memcpy(cs->skat, ss->skat, sizeof(cs->skat));
  }
}

void
server_skat_state_init(skat_server_state *ss) {
  ss->sgs.cgphase = GAME_PHASE_SETUP;
  memset(ss->sgs.score, 0, sizeof(ss->sgs.score));
  memset(ss->sgs.active_players, -1, sizeof(ss->sgs.active_players));
}

void
client_skat_state_init(skat_client_state *cs) {
  cs->sgs.cgphase = GAME_PHASE_SETUP;
  memset(cs->sgs.score, '\0', sizeof(cs->sgs.score));
  memset(cs->sgs.active_players, -1, sizeof(cs->sgs.active_players));

  cs->my_partner = cs->my_active_player_index = -1;
}

void
client_skat_state_notify_join(skat_client_state *cs,
							  payload_notify_join *pl_nj) {}

void
client_skat_state_notify_leave(skat_client_state *cs,
							   payload_notify_leave *pl_nl) {}
