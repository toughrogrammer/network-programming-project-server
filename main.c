#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include "library/parson/parson.h"
#include "library/klib/khash.h"
#include "constants.h"
#include "data_structure.h"
#include "listening.h"
#include "util.h"
#include "user.h"
#include "game.h"


void init_variables();
void load_data();
void route_sign_up(JSON_Object *json, key_t mq_key, long target);
void route_sign_in(JSON_Object *json, key_t mq_key, long target);
void route_sign_out(JSON_Object *json, key_t mq_key, long target);
void route_check_lobby(JSON_Object *json, key_t mq_key, long target);
void route_chatting(JSON_Object *json, key_t mq_key, long target);
void route_create_room(JSON_Object *json, key_t mq_key, long target);
void route_join_room(JSON_Object *json, key_t mq_key, long target);
void route_check_room(JSON_Object *json, key_t mq_key, long target);
void route_game_start(JSON_Object *json, key_t mq_key, long target);
void broadcast_lobby(key_t mq_key, char* message);
void broadcast_room(key_t mq_key, char* message, int pk_room);

int main_server_quit;


int main() {
	key_t msg_queue_key_id = msgget((key_t)MQ_KEY, IPC_CREAT | 0666);
	if( msg_queue_key_id == -1 ) {
		char error_log[MAX_LENGTH];
		sprintf(error_log, "failed to create message queue. errno = %d", errno);
		ERROR_LOGGING(error_log)
		return 1;
	}
	printf("(main) msg_queue_key_id : %d\n", msg_queue_key_id);
	// clear message queue
	int removed_msg_count = 0;
	while(1) {
		struct message_buffer msg;
		if( msgrcv(msg_queue_key_id, (void*)&msg, sizeof(struct  message_buffer), 0, IPC_NOWAIT) == -1 ) {
			break;
		}
		printf("\r(main) cleared message count : %d", ++removed_msg_count);
	}
	printf("\n");

	pid_t pid = fork();
	if( pid != 0 ) {
		return listening(pid, 10101);
	}

	init_variables();
	load_data();

	long curr = get_time_in_millisec();
	long prev = curr;

	while(! main_server_quit) {
		struct message_buffer received;
		if (msgrcv( msg_queue_key_id, (void *)&received, sizeof(struct message_buffer), MQ_ID_MAIN_SERVER, IPC_NOWAIT) != -1) {
			printf("(main) msgrcv : %s", received.buffer);

			JSON_Value *json_value = json_parse_string(received.buffer);
			if( json_value == NULL ) {
				// failed to parsing message
				printf("(from:%ld) failed to parsing\n%s", received.from, received.buffer);
				continue;
			}

			JSON_Object *json_body = json_value_get_object(json_value);
			int msg_target = (int)json_object_get_number(json_body, "target");
			switch(msg_target) {
			case MSG_TARGET_SIGN_UP:
				route_sign_up(json_body, msg_queue_key_id, received.from);
				break;
			case MSG_TARGET_SIGN_IN:
				route_sign_in(json_body, msg_queue_key_id, received.from);
				break;
			case MSG_TARGET_CHATTING:
				route_chatting(json_body, msg_queue_key_id, received.from);
				break;
			case MSG_TARGET_QUIT:
				route_sign_out(json_body, msg_queue_key_id, received.from);
				break;
			case MSG_TARGET_CHECK_LOBBY:
				route_check_lobby(json_body, msg_queue_key_id, received.from);
				break;
			case MSG_TARGET_CREATE_ROOM:
				route_create_room(json_body, msg_queue_key_id, received.from);
				break;
			case MSG_TARGET_JOIN_ROOM:
				route_join_room(json_body, msg_queue_key_id, received.from);
				break;
			case MSG_TARGET_CHECK_ROOM:
				route_check_room(json_body, msg_queue_key_id, received.from);
				break;
			case MSG_TARGET_GAME_START:
				route_game_start(json_body, msg_queue_key_id, received.from);
				break;
			}

			json_value_free(json_value);
		}

		curr = get_time_in_millisec();
		long diff = curr - prev;
		prev = curr;

		update_game_rooms(msg_queue_key_id, diff);
	}


	return 0;
}

void init_variables() {
	user_table = kh_init(pk_int);
	connected_user_table = kh_init(str);
	game_room_table = kh_init(pk_room);
}

void load_data() {
	int ret;
	khint_t k;

	// add user data for debugging
	for( int i = 0; i < 5; i ++ ) {
		struct user_data* new_user_data = (struct user_data*)malloc(sizeof(struct user_data));
		memset(new_user_data, 0, sizeof(struct user_data));
		new_user_data->pk = i;
		sprintf(new_user_data->id, "testuser%d", i);
		sprintf(new_user_data->password, "testuser%d", i);
		k = kh_put(pk_int, user_table, i, &ret);
		kh_value(user_table, k) = new_user_data;
	}
}

void route_sign_up(JSON_Object *json, key_t mq_key, long target) {
	printf("(main) route_sign_up\n");

	if( ! send_message_to_queue(mq_key, MQ_ID_MAIN_SERVER, target, "response") ) {
		// success
	} else {
		// error
	}
}

void route_sign_in(JSON_Object *json, key_t mq_key, long target) {
	printf("(main) route_sign_in\n");

	static int connected = 0;
	char access_token[MAX_LENGTH];
	sprintf(access_token, "user%d", connected);
	connected++;

	// find user
	const char* submitted_id = json_object_get_string(json, "id");
	const char* submitted_password = json_object_get_string(json, "password");

	struct user_data* user_data = find_user_data(submitted_id, submitted_password);
	if( user_data == NULL ) {
		char response[MAX_LENGTH];
		build_simple_response(response, RESULT_ERROR_INVALID_AUTH);
		send_message_to_queue(mq_key, MQ_ID_MAIN_SERVER, target, response);
		return;
	}

	struct connected_user* connected_user = find_connected_user_by_pk(user_data->pk);
	if( connected_user == NULL ) {
		// new connection
		struct connected_user* new_connected_user = (struct connected_user*)malloc(sizeof(struct connected_user));
		fill_connected_user(new_connected_user, user_data->pk, target, USER_STATUS_LOBBY, access_token);

		int ret;
		khint_t k = kh_put(str, connected_user_table, access_token, &ret);
		kh_value(connected_user_table, k) = new_connected_user;
	} else {
		fill_connected_user(connected_user, user_data->pk, target, USER_STATUS_LOBBY, access_token);
	}

	// logging
	print_users_status();


	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);
	json_object_set_number(root_object, "result", RESULT_OK_SIGN_IN);
	json_object_set_string(root_object, "access_token", access_token);

	char response[MAX_LENGTH];
	sprintf(response, "%s\r\n", json_serialize_to_string(root_value));
	json_value_free(root_value);

	if( send_message_to_queue(mq_key, MQ_ID_MAIN_SERVER, target, response) != -1 ) {
		// success
	} else {
		// error
	}
}

void route_sign_out(JSON_Object *json, key_t mq_key, long target) {
	printf("(main) route_sign_out\n");

	// validate user
	const char* access_token = json_object_get_string(json, "access_token");
	khint_t k = kh_get(str, connected_user_table, access_token);
	if( k == kh_end(connected_user_table) ) {
		char response[MAX_LENGTH];
		build_simple_response(response, RESULT_ERROR_INVALID_CONNECTION);
		send_message_to_queue(mq_key, MQ_ID_MAIN_SERVER, target, response);
		return;
	}
	free(kh_value(connected_user_table, k));
	kh_del(str, connected_user_table, k);

	// logging
	print_users_status();

	char response[MAX_LENGTH];
	build_simple_response(response, RESULT_OK_REQUEST_LOBBY_UPDATE);
	send_message_to_queue(mq_key, MQ_ID_MAIN_SERVER, target, response);
	printf("(main) send_message_to_queue\n");
}

void route_chatting(JSON_Object *json, key_t mq_key, long target) {
	long destination = 0;
	printf("(main) route_chatting\n");

	// access token과 message를 json에서 가져옴
	const char* access_token = json_object_get_string(json, "access_token");
	const char* message = json_object_get_string(json, "message");

	printf("access_token : %s\n", access_token);
	const char* sender_id = find_user_id_by_access_token(access_token);

	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);
	json_object_set_number(root_object, "result", RESULT_OK_CHATTING);
	json_object_set_value(root_object, "message", json_value_init_object());
	JSON_Value *value_data = json_object_get_value(root_object, "message");

	json_object_set_string(json_object(value_data), "sender_id", sender_id);
	json_object_set_string(json_object(value_data), "content", message);

	char response[MAX_LENGTH];
	sprintf(response, "%s\r\n", json_serialize_to_string(root_value));
	json_value_free(root_value);

	// 현재 접속중인 user들에게 메세지 전송 
	for (khint_t k = kh_begin(connected_user_table); k != kh_end(connected_user_table); ++k) {
		if (kh_exist(connected_user_table, k)) {
			struct connected_user* user = kh_value(connected_user_table, k);
			if( send_message_to_queue(mq_key, MQ_ID_MAIN_SERVER, user->mq_id, response) != -1 ) {
				// success
			} else {
				// error
			}
		}
	}
}

void route_check_lobby(JSON_Object *json, key_t mq_key, long target) {
	printf("(main) route_check_lobby\n");

	const char* access_token = json_object_get_string(json, "access_token");
	// check this user exist in connected user list
	// if not exist, respones error to target

	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);
	json_object_set_number(root_object, "result", RESULT_OK_STATE_LOBBY);

	json_object_set_value(root_object, "data", json_value_init_object());
	JSON_Value *value_data = json_object_get_value(root_object, "data");

	json_object_set_value(json_object(value_data), "user_list", json_value_init_array());
	JSON_Array *users = json_object_get_array(json_object(value_data), "users");
	get_lobby_user_list(users);

	json_object_set_value(json_object(value_data), "room_list", json_value_init_array());
	JSON_Array *rooms = json_object_get_array(json_object(value_data), "rooms");
	get_room_list(rooms);


	char response[MAX_LENGTH];
	sprintf(response, "%s\r\n", json_serialize_to_string(root_value));
	send_message_to_queue(mq_key, MQ_ID_MAIN_SERVER, target, response);

	json_value_free(root_value);
}

void route_create_room(JSON_Object *json, key_t mq_key, long target) {
	printf("(main) route_create_room\n");

	// validate user
	const char* access_token = json_object_get_string(json, "access_token");
	if( validate_user(access_token) != 0 ) {
		char response[MAX_LENGTH];
		build_simple_response(response, RESULT_ERROR_INVALID_CONNECTION);
		send_message_to_queue(mq_key, MQ_ID_MAIN_SERVER, target, response);
		return;
	}

	const char* title = json_object_get_string(json, "title");
	long pk_room = create_game_room(title);


	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);
	json_object_set_number(root_object, "result", RESULT_OK_CREATE_ROOM);
	json_object_set_number(root_object, "room_id", pk_room);

	char response[MAX_LENGTH];
	sprintf(response, "%s\r\n", json_serialize_to_string(root_value));
	json_value_free(root_value);

	send_message_to_queue(mq_key, MQ_ID_MAIN_SERVER, target, response);

	struct connected_user* user = find_connected_user_by_access_token(access_token);
	join_game_room(pk_room, user->pk);

	request_room_update(mq_key, pk_room);

	//broadcasting to users in lobby
	broadcast_lobby(mq_key, response);

	print_users_status();
}

void route_join_room(JSON_Object *json, key_t mq_key, long target) {
	printf("(main) route_join_room\n");

	// validate user
	const char* access_token = json_object_get_string(json, "access_token");
	if( validate_user(access_token) != 0 ) {
		char response[MAX_LENGTH];
		build_simple_response(response, RESULT_ERROR_INVALID_CONNECTION);
		send_message_to_queue(mq_key, MQ_ID_MAIN_SERVER, target, response);
		return;
	}

	struct connected_user* user = find_connected_user_by_access_token(access_token);

	long pk_room = json_object_get_number(json, "room_id");
	int result = join_game_room(pk_room, user->pk);
	if( result < 0 ) {
		// failed to join game room for many reasons
	}

	char response[MAX_LENGTH];
	build_simple_response(response, RESULT_OK_JOIN_ROOM);
	send_message_to_queue(mq_key, MQ_ID_MAIN_SERVER, target, response);

	// broadcasting to users in lobby and room
	broadcast_room(mq_key, response, pk_room);

	request_room_update(mq_key, pk_room);
}

void route_check_room(JSON_Object *json, key_t mq_key, long target) {
	printf("(main) route_check_room\n");

	// validate user
	const char* access_token = json_object_get_string(json, "access_token");
	if( validate_user(access_token) != 0 ) {
		char response[MAX_LENGTH];
		build_simple_response(response, RESULT_ERROR_INVALID_CONNECTION);
		send_message_to_queue(mq_key, MQ_ID_MAIN_SERVER, target, response);
		return;
	}

	struct connected_user* user = find_connected_user_by_access_token(access_token);
	if( user->status != USER_STATUS_IN_ROOM ) {
		// error
		return;
	}

	struct game_room* room = find_game_room_by_pk(user->pk_room);

	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);
	json_object_set_number(root_object, "result", RESULT_OK_STATE_GAME_ROOM);

	json_object_set_value(root_object, "data", json_value_init_object());
	JSON_Value *value_data = json_object_get_value(root_object, "data");
	json_object_set_number(json_object(value_data), "room_id", room->pk_room);
	json_object_set_value(json_object(value_data), "user_list", json_value_init_array());
	JSON_Array *json_array_user_list = json_object_get_array(json_object(value_data), "user_list");
	get_room_user_list(room->pk_room, json_array_user_list);

	char response[MAX_LENGTH];
	sprintf(response, "%s\r\n", json_serialize_to_string(root_value));
	json_value_free(root_value);

	send_message_to_queue(mq_key, MQ_ID_MAIN_SERVER, target, response);
}

void route_game_start(JSON_Object *json, key_t mq_key, long target) {
	printf("(main) route_game_start\n");

	char response[MAX_LENGTH];

	// validate user
	const char* access_token = json_object_get_string(json, "access_token");
	if( validate_user(access_token) != 0 ) {
		build_simple_response(response, RESULT_ERROR_INVALID_CONNECTION);
		send_message_to_queue(mq_key, MQ_ID_MAIN_SERVER, target, response);
		return;
	}

	struct connected_user* user = find_connected_user_by_access_token(access_token);
	if( user->status != USER_STATUS_IN_ROOM ) {
		// error
		return;
	}

	if( start_game(user->pk_room) < 0 ) {
		// error
	} else {
		build_simple_response(response, RESULT_OK_START_GAME);
		send_message_to_queue(mq_key, MQ_ID_MAIN_SERVER, target, response);
	}

	struct game_room* room = find_game_room_by_pk(user->pk_room);
	if( room == NULL ) {
		// error
		return;
	}

	notify_game_start(mq_key, room);
}