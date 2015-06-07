#include "game.h"
#include "user.h"
#include "constants.h"


void get_room_list(JSON_Array *arr) {

}

void update_game_rooms(key_t mq_key, long dt) {
	for (khint_t k = kh_begin(game_room_table); k != kh_end(game_room_table); ++k) {
		if (kh_exist(game_room_table, k)) {
			struct game_room* room = kh_value(game_room_table, k);
			room->timer += dt;

			switch(room->status) {
			case GAME_ROOM_STATUS_READY:
				handle_game_room_ready(mq_key, room);
				break;
			case GAME_ROOM_STATUS_PLAYING:
				handle_game_room_playing(mq_key, room);
				break;
			case GAME_ROOM_STATUS_SHOWING_ROUND_RESULT:
				handle_game_room_showing_round_result(mq_key, room);
				break;
			case GAME_ROOM_STATUS_SHOWING_TOTAL_RESULT:
				handle_game_room_showing_total_result(mq_key, room);
				break;
			}
		}
	}
}

void handle_game_room_ready(key_t mq_key, struct game_room* room) {

}

void handle_game_room_playing(key_t mq_key, struct game_room* room) {
	if( room->timer > MAX_ROUND_TIMER ) {
		room->timer = 0;

		notify_round_end(mq_key, room);
	}
}

void handle_game_room_showing_round_result(key_t mq_key, struct game_room* room) {

}

void handle_game_room_showing_total_result(key_t mq_key, struct game_room* room) {

}

void notify_game_start(key_t mq_key, struct game_room* room) {
	char response[MAX_LENGTH];
	build_simple_response(response, REUSLT_OK_GAME_START);

	for( int i = 0; i < room->num_of_users; i ++ ) {
		int user_pk = room->member_pk_list[i];
		struct connected_user *user = find_connected_user_by_pk(user_pk);
		if( user != NULL ) {
			send_message_to_queue(mq_key, MQ_ID_MAIN_SERVER, user->mq_id, response);
		}
	}
}

void notify_round_end(key_t mq_key, struct game_room* room) {
	long winner_pk = room->winner_of_round[room->curr_round];

	char response[MAX_LENGTH];
	sprintf(response, "%s\r\n", "");

	for( int i = 0; i < room->num_of_users; i ++ ) {
		int user_pk = room->member_pk_list[i];
		struct connected_user *user = find_connected_user_by_pk(user_pk);
		if( user != NULL ) {
			send_message_to_queue(mq_key, MQ_ID_MAIN_SERVER, user->mq_id, response);
		}
	}
}