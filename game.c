#include "game.h"
#include "user.h"


void get_room_list(JSON_Array *arr) {

}

void update_game_rooms(long dt, khash_t(pk_room) *game_room_table) {
	for (khint_t k = kh_begin(game_room_table); k != kh_end(game_room_table); ++k) {
		if (kh_exist(game_room_table, k)) {
			struct game_room* room = kh_value(game_room_table, k);
			room->timer += dt;

			if( room->timer > MAX_ROUND_TIMER ) {
				room->timer = 0;

				notify_round_end(room);
			}
		}
	}
}

void judge_round_winner(struct game_room* room) {

}

void notify_round_end(struct game_room* room) {
	long winner_pk = room->winner_of_round[room->curr_round];
	for( int i = 0; i < room->num_of_users; i ++ ) {

	}
}