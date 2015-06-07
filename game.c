#include "game.h"
#include "user.h"
#include "constants.h"


void get_room_list(JSON_Array *arr) {

}

void update_game_rooms(long dt) {
	for (khint_t k = kh_begin(game_room_table); k != kh_end(game_room_table); ++k) {
		if (kh_exist(game_room_table, k)) {
			struct game_room* room = kh_value(game_room_table, k);
			room->timer += dt;

			switch(room->status) {
			case GAME_ROOM_STATUS_READY:
				handle_game_room_ready(room);
				break;
			case GAME_ROOM_STATUS_PLAYING:
				handle_game_room_playing(room);
				break;
			case GAME_ROOM_STATUS_SHOWING_ROUND_RESULT:
				handle_game_room_showing_round_result(room);
				break;
			case GAME_ROOM_STATUS_SHOWING_TOTAL_RESULT:
				handle_game_room_showing_total_result(room);
				break;
			}
		}
	}
}

void handle_game_room_ready(struct game_room* room) {

}

void handle_game_room_playing(struct game_room* room) {
	if( room->timer > MAX_ROUND_TIMER ) {
		room->timer = 0;

		notify_round_end(room);
	}
}

void handle_game_room_showing_round_result(struct game_room* room) {

}

void handle_game_room_showing_total_result(struct game_room* room) {

}

void notify_round_end(struct game_room* room) {
	long winner_pk = room->winner_of_round[room->curr_round];
	for( int i = 0; i < room->num_of_users; i ++ ) {

	}
}