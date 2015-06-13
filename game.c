#include "game.h"
#include "user.h"
#include "constants.h"
#include "MemLog.h"


void get_room_list(JSON_Array *arr) {
	for (khint_t k = kh_begin(game_room_table); k != kh_end(game_room_table); ++k) {
		if (kh_exist(game_room_table, k)) {
			struct game_room* room = kh_value(game_room_table, k);

			JSON_Value *room_info = json_value_init_object();
			JSON_Object *room_object = json_value_get_object(room_info);
			json_object_set_number(room_object, "room_id", room->pk_room);
			json_object_set_string(room_object, "title", room->title);
			json_object_set_number(room_object, "num_of_member", room->num_of_users);
			json_object_set_number(room_object, "state", room->status);
			json_array_append_value(arr, room_info);
		}
	}
}

struct game_room* find_game_room_by_pk(long pk) {
	khint_t k = kh_get(pk_room, game_room_table, pk);
	if( k == kh_end(game_room_table) ) {
		return NULL;
	}

	return kh_value(game_room_table, k);
}

void print_game_rooms_status() {
	printf("--game rooms--\n");
	for (khint_t k = kh_begin(game_room_table); k != kh_end(game_room_table); ++k) {
		if (kh_exist(game_room_table, k)) {
			struct game_room* room = kh_value(game_room_table, k);
			printf("%ld\t%ld\t%ld/%ld\t%s\t%ld/%ld\n", room->pk_room, 
				room->status,
				room->num_of_users, room->capacity,
				room->title,
				room->curr_round, room->total_round);
		}
	}
	printf("--------------\n");
}

void update_game_rooms(key_t mq_key, long dt) {
	for (khint_t k = kh_begin(game_room_table); k != kh_end(game_room_table); ++k) {
		if (kh_exist(game_room_table, k)) {
			struct game_room* room = kh_value(game_room_table, k);
			room->timer += dt;

			switch(room->status) {
			case GAME_ROOM_STATUS_WAITING:
				handle_game_room_waiting(mq_key, room);
				break;
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
			default:
				printf("error - handling game room %ld\n", room->pk_room);
				break;
			}
		}
	}
}

void handle_game_room_waiting(key_t mq_key, struct game_room* room) {

}

void handle_game_room_ready(key_t mq_key, struct game_room* room) {
	if( room->timer > MAX_ROUND_TIMER_READY ) {
		room->timer = 0;

		room->status = GAME_ROOM_STATUS_PLAYING;
		notify_round_start(mq_key, room);
	}
}

void handle_game_room_playing(key_t mq_key, struct game_room* room) {
	if( room->timer > MAX_ROUND_TIMER_SUBMIT ) {
		room->timer = 0;

		room->curr_round = room->curr_round + 1;
		notify_round_end(mq_key, room);
		if( room->curr_round >= room->total_round ) {
			room->status = GAME_ROOM_STATUS_SHOWING_TOTAL_RESULT;
		} else {
			room->status = GAME_ROOM_STATUS_SHOWING_ROUND_RESULT;
		}
	}
}

void handle_game_room_showing_round_result(key_t mq_key, struct game_room* room) {
	if( room->timer > MAX_ROUND_TIMER_SHOW_ROUND_RESULT ) {
		room->timer = 0;

		room->status = GAME_ROOM_STATUS_PLAYING;
		notify_round_start(mq_key, room);
	}
}

void handle_game_room_showing_total_result(key_t mq_key, struct game_room* room) {
	int i;
	struct user_data *usr;

	for( i = 0 ; i < room->num_of_users ; ++ i ){
		usr = find_user_data_by_pk( room->member_pk_list[i] );
		usr->exp += 100;
		UpdateExp( usr->pk, usr->exp );
	}

	if( room->timer > MAX_ROUND_TIMER_SHOW_TOTAL_RESULT ) {
		room->timer = 0;

		end_game(room->pk_room);
		notify_game_end(mq_key, room);
	}
}

void notify_game_start(key_t mq_key, struct game_room* room) {
	char response[MAX_LENGTH];
	build_simple_response(response, RESULT_OK_NOTIFYING_START_GAME);

	for( int i = 0; i < room->num_of_users; i ++ ) {
		int user_pk = room->member_pk_list[i];
		struct connected_user *user = find_connected_user_by_pk(user_pk);
		if( user != NULL ) {
			send_message_to_queue(mq_key, MQ_ID_MAIN_SERVER, user->mq_id, response);
			printf("send_message_to_queue %ld user(%s) : %s", user->mq_id, user->access_token, response);
		}
	}
}

void notify_round_start(key_t mq_key, struct game_room* room) {
	int random_index = rand() % num_of_problems;
	strcpy(room->problem, problems[random_index]);

	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);
	json_object_set_number(root_object, "result", RESULT_OK_MAKE_QUIZ);
	json_object_set_number(root_object, "time", MAX_ROUND_TIMER_SUBMIT);
	json_object_set_string(root_object, "quiz_string", room->problem);

	char response[MAX_LENGTH];
	serialize_json_to_response(response, root_value);
	json_value_free(root_value);

	for( int i = 0; i < room->num_of_users; i ++ ) {
		int user_pk = room->member_pk_list[i];
		struct connected_user *user = find_connected_user_by_pk(user_pk);
		if( user != NULL ) {
			send_message_to_queue(mq_key, MQ_ID_MAIN_SERVER, user->mq_id, response);
		}
	}
}

void notify_round_end(key_t mq_key, struct game_room* room) {
	printf("(main) notify_round_end\n");

	long winner_pk = room->winner_of_round[room->curr_round - 1];
	struct user_data* user = find_user_data_by_pk(winner_pk);
	char *winner_id = "";
	if( user != NULL ) {
		winner_id = user->id;
	}

	printf("(main) notify_round_end winner(%s)\n", winner_id);

	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);
	json_object_set_number(root_object, "result", RESULT_OK_ROUND_RESULT);
	json_object_set_string(root_object, "winner_id", winner_id);
	json_object_set_number(root_object, "remain_round", room->total_round - room->curr_round);

	char response[MAX_LENGTH];
	serialize_json_to_response(response, root_value);
	json_value_free(root_value);

	for( int i = 0; i < room->num_of_users; i ++ ) {
		int user_pk = room->member_pk_list[i];
		struct connected_user *user = find_connected_user_by_pk(user_pk);
		if( user != NULL ) {
			send_message_to_queue(mq_key, MQ_ID_MAIN_SERVER, user->mq_id, response);
		}
	}
}

void notify_game_end(key_t mq_key, struct game_room* room) {
	int win_count[MAX_GAME_ROOM_CAPACITY] = { 0, };
	int total_winner = 0;
	int max_win_round = -1;

	for( int i = 0; i < room->total_round; i ++ ) {
		for( int j = 0; j < room->num_of_users; j ++ ) {
			if( room->member_pk_list[j] == room->winner_of_round[i] ) {
				win_count[j]++;
			}
		}
	}

	for( int i = 0; i < room->num_of_users; i ++ ) {
		if( win_count[i] > max_win_round ) {
			max_win_round = win_count[i];
			total_winner = room->member_pk_list[i];
		}
	}

	struct user_data* userdata = find_user_data_by_pk(total_winner);
	if( userdata == NULL )  {
		// error
		return;
	}

	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);
	json_object_set_number(root_object, "result", RESULT_OK_TOTAL_RESULT);
	json_object_set_string(root_object, "winner_id", userdata->id);

	char response[MAX_LENGTH];
	serialize_json_to_response(response, root_value);
	json_value_free(root_value);

	for( int i = 0; i < room->num_of_users; i ++ ) {
		int user_pk = room->member_pk_list[i];
		struct connected_user *user = find_connected_user_by_pk(user_pk);
		if( user != NULL ) {
			send_message_to_queue(mq_key, MQ_ID_MAIN_SERVER, user->mq_id, response);
		}
	}
	build_simple_response(response, RESULT_OK_REQUEST_LOBBY_UPDATE);
	broadcast_lobby(mq_key, response);
}

long create_game_room(const char* title) {
	struct game_room* new_game_room = (struct game_room*)malloc(sizeof(struct game_room));
	memset(new_game_room, 0, sizeof(struct game_room));

	new_game_room->pk_room = next_pk_room++;
	new_game_room->capacity = MAX_GAME_ROOM_CAPACITY;
	strcpy(new_game_room->title, title);
	new_game_room->status = GAME_ROOM_STATUS_WAITING;
	new_game_room->total_round = MAX_GAME_ROUND;

	int ret;
	khint_t k = kh_put(pk_room, game_room_table, new_game_room->pk_room, &ret);
	kh_value(game_room_table, k) = new_game_room;

	return new_game_room->pk_room;
}

int remove_game_room(long pk) {
	printf("(main) remove_game_room\n");
	khint_t k = kh_get(pk_room, game_room_table, pk);
	if( k == kh_end(game_room_table) ) {
		return -1;
	}
	free(kh_value(game_room_table, k));
	kh_del(pk_room, game_room_table, k);

	return 0;
}

int join_game_room(long pk_room, long pk_user) {
	struct game_room* room = find_game_room_by_pk(pk_room);
	if( room == NULL ) {
		return -1;
	}

	if( room->num_of_users == room->capacity ) {
		return -2;
	}

	if( room->status != GAME_ROOM_STATUS_WAITING ) {
		return -3;
	}

	room->member_pk_list[room->num_of_users] = pk_user;
	room->num_of_users++;

	struct connected_user* user = find_connected_user_by_pk(pk_user);
	user->status = USER_STATUS_IN_ROOM;
	user->pk_room = pk_room;

	return 0;
}

int leave_game_room(struct connected_user* user) {
	struct game_room* room = find_game_room_by_pk(user->pk_room);
	if( room == NULL ) {
		return -1;
	}

	if( room->status != GAME_ROOM_STATUS_WAITING ) {
		return -2;
	}

	int index = 0;
	for( index = 0; index < room->num_of_users; index ++ ){
		if( room->member_pk_list[index] == user->pk ) {
			for( int j = index; j < room->num_of_users; j ++ ) {
				room->member_pk_list[j] = room->member_pk_list[j + 1];
			}
			room->member_pk_list[room->num_of_users - 1] = 0;
			room->num_of_users--;

			user->pk_room = 0;
			user->status = USER_STATUS_LOBBY;
			break;
		}
	}

	if( room->num_of_users == 0 ) {
		remove_game_room(room->pk_room);
	}
	return 0;
}

int start_game(long pk_room) {
	struct game_room* room = find_game_room_by_pk(pk_room);
	if( room == NULL ) {
		return -1;
	}

	room->status = GAME_ROOM_STATUS_READY;
	room->curr_round = 0;
	room->timer = 0;
	for( int i = 0; i < room->total_round; i ++ ) {
		room->winner_of_round[i] = -1;
	}
	strcpy(room->problem, "");

	print_game_rooms_status();

	return 0;
}

int end_game(long pk_room) {
	struct game_room* room = find_game_room_by_pk(pk_room);
	if( room == NULL ) {
		return -1;
	}

	room->status = GAME_ROOM_STATUS_WAITING;

	print_game_rooms_status();

	return 0;
}

void request_room_update(key_t mq_key, long pk_room) {
	printf("(game.c) request_room_update\n");
	struct game_room* room = find_game_room_by_pk(pk_room);
	if( room == NULL ) {
		return;
	}

	char response[MAX_LENGTH];
	build_simple_response(response, RESULT_OK_REQUEST_ROOM_MEMBER_UPDATE);
	broadcast_room(mq_key, response, pk_room);
}
