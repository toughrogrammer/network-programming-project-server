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
void route_sign_up(JSON_Object *json, key_t mq_key, long target);
void route_sign_in(JSON_Object *json, key_t mq_key, long target);
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

}

void print_users_status() {
	printf("--users--\n");
	for (khint_t k = kh_begin(connected_user_table); k != kh_end(connected_user_table); ++k) {
		if (kh_exist(connected_user_table, k)) {
			struct connected_user* user = kh_value(connected_user_table, k);
			printf("%d %d %s\n", (int)user->mq_id, (int)user->status, user->access_token);
		}
	}
	printf("---------\n");
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

	khint_t k;
	for (k = kh_begin(connected_user_table); k != kh_end(connected_user_table); ++k) {
		if (kh_exist(connected_user_table, k)) {
			struct connected_user* userdata = kh_value(connected_user_table, k);
			if( userdata->pk == 999 ) {
				break;
			}
		}
	}
	if( k == kh_end(connected_user_table) ) {
		// new connection
		struct connected_user* new_connected_user = (struct connected_user*)malloc(sizeof(struct connected_user));
		memset(new_connected_user, 0, sizeof(struct connected_user));
		new_connected_user->mq_id = target;
		new_connected_user->status = USER_STATUS_LOBBY;
		strcpy(new_connected_user->access_token, access_token);

		int ret;
		k = kh_put(str, connected_user_table, access_token, &ret);
		kh_value(connected_user_table, k) = new_connected_user;
	} else {
		// there is prev connection of this user
		struct connected_user* new_connected_user = kh_value(connected_user_table, k);
		new_connected_user->mq_id = target;
		new_connected_user->status = USER_STATUS_LOBBY;
		strcpy(new_connected_user->access_token, access_token);
	}

	print_users_status();

	strcat(access_token, "\r\n");
	if( ! send_message_to_queue(mq_key, MQ_ID_MAIN_SERVER, target, access_token) ) {
		// success
	} else {
		// error
	}
}

void route_chatting(JSON_Object *json, key_t mq_key, long target) {
	
}