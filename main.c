#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "library/parson/parson.h"
#include "library/klib/khash.h"
#include "constants.h"
#include "data_structure.h"
#include "listening.h"
#include "util.h"
#include <errno.h>


void init_variables();
void load_data();
void print_users_status();
struct user_data* find_user_data(const char* id, const char* password);
struct connected_user* find_connected_user_by_access_token(const char* access_token);
struct connected_user* find_connected_user_by_pk(int pk);
void route_sign_up(JSON_Object *json, key_t mq_key, long target);
void route_sign_in(JSON_Object *json, key_t mq_key, long target);
void route_sign_out(JSON_Object *json, key_t mq_key, long target);
void route_chatting(JSON_Object *json, key_t mq_key, long target);

int main_server_quit;

khash_t(pk_int) *user_table;
khash_t(str) *connected_user_table;


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

	while(! main_server_quit) {
		struct message_buffer received;
		if (msgrcv( msg_queue_key_id, (void *)&received, sizeof(struct message_buffer), MQ_ID_MAIN_SERVER, IPC_NOWAIT) != -1) {
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
			}

			json_value_free(json_value);
		}
	}


	return 0;
}

void init_variables() {
	user_table = kh_init(pk_int);
	connected_user_table = kh_init(str);
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

void print_users_status() {
	printf("--users--\n");
	for (khint_t k = kh_begin(connected_user_table); k != kh_end(connected_user_table); ++k) {
		if (kh_exist(connected_user_table, k)) {
			struct connected_user* user = kh_value(connected_user_table, k);
			printf("%d %d %d %s\n", (int)user->mq_id, (int)user->pk, (int)user->status, user->access_token);
		}
	}
	printf("---------\n");
}

struct user_data* find_user_data(const char* id, const char* password) {
	for (khint_t k = kh_begin(user_table); k != kh_end(user_table); ++k) {
		if (kh_exist(user_table, k)) {
			struct user_data* userdata = kh_value(user_table, k);
			if( strcmp(userdata->id, id) == 0 && strcmp(userdata->password, password) == 0 ) {
				return userdata;
			}
		}
	}

	// not found
	return NULL;
}

struct connected_user* find_connected_user_by_access_token(const char* access_token) {
	khint_t k = kh_get(str, connected_user_table, access_token);
	if( k == kh_end(connected_user_table) ) {
		return NULL;
	}

	return kh_value(connected_user_table, k);
}

struct connected_user* find_connected_user_by_pk(int pk) {
	for (khint_t k = kh_begin(connected_user_table); k != kh_end(connected_user_table); ++k) {
		if (kh_exist(connected_user_table, k)) {
			struct connected_user* userdata = kh_value(connected_user_table, k);
			if( userdata->pk == pk ) {
				return userdata;
			}
		}
	}

	return NULL;
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
	build_simple_response(response, REUSLT_OK_REQUEST_LOBBY_UPDATE);
	send_message_to_queue(mq_key, MQ_ID_MAIN_SERVER, target, response);
	printf("(main) send_message_to_queue\n");
}

void route_chatting(JSON_Object *json, key_t mq_key, long target) {
	
}