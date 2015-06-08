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
	build_simple_response(response, RESULT_OK_GAME_START);

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

long create_game_room(const char* title) {
	struct game_room* new_game_room = (struct game_room*)malloc(sizeof(struct game_room));
	memset(new_game_room, 0, sizeof(struct game_room));

	new_game_room->pk_room = next_pk_room++;
	new_game_room->capacity = MAX_GAME_ROOM_CAPACITY;
	strcpy(new_game_room->title, title);
	new_game_room->status = GAME_ROOM_STATUS_READY;
	new_game_room->total_round = MAX_GAME_ROUND;

	int ret;
	khint_t k = kh_put(pk_room, game_room_table, new_game_room->pk_room, &ret);
	kh_value(game_room_table, k) = new_game_room;

	return new_game_room->pk_room;
}

int remove_game_room(long pk) {
	khint_t k = kh_get(pk_room, game_room_table, pk);
	if( k == kh_end(game_room_table) ) {
		return -1;
	}
	free(kh_value(game_room_table, k));
	kh_del(pk_room, game_room_table, k);

	return 0;
}

struct game_room* find_game_room_by_pk(long pk) {
	khint_t k = kh_get(pk_room, game_room_table, pk);
	if( k == kh_end(game_room_table) ) {
		return NULL;
	}

	return kh_value(game_room_table, k);
}

int join_game_room(long pk_room, long pk_user) {
	struct game_room* room = find_game_room_by_pk(pk_room);
	if( room == NULL ) {
		return -1;
	}

	if( room->num_of_users == room->capacity ) {
		return -2;
	}

	if( room->status != GAME_ROOM_STATUS_READY ) {
		return -3;
	}

	room->member_pk_list[room->num_of_users] = pk_user;
	room->num_of_users++;

	struct connected_user* user = find_connected_user_by_pk(pk_user);
	user->status = USER_STATUS_IN_ROOM;
	user->pk_room = pk_room;

	return 0;
}

void request_room_update(key_t mq_key, long pk_room) {
	struct game_room* room = find_game_room_by_pk(pk_room);
	if( room == NULL ) {
		return;
	}

	char response[MAX_LENGTH];
	build_simple_response(response, RESULT_OK_REQUEST_ROOM_MEMBER_UPDATE);

	for( int i = 0; i < room->num_of_users; i ++ ) {
		long pk_user = room->member_pk_list[i];
		struct connected_user* user = find_connected_user_by_pk(pk_user);
		if( user != NULL ) {
			send_message_to_queue(mq_key, MQ_ID_MAIN_SERVER, user->mq_id, response);
		}
	}
}