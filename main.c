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
void print_users_status();
void route_sign_up(JSON_Object *json, key_t mq_key, long target);
void route_sign_in(JSON_Object *json, key_t mq_key, long target);
void route_chatting(JSON_Object *json, key_t mq_key, long target);

int main_server_quit;

khash_t(str) *users_table;


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
	users_table = kh_init(str);
}

void print_users_status() {
	printf("--users--\n");
	for (khint_t k = kh_begin(users_table); k != kh_end(users_table); ++k) {
		if (kh_exist(users_table, k)) {
			struct user_data* userdata = kh_value(users_table, k);
			printf("%d %s\n", (int)userdata->status, userdata->access_token);
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

	struct user_data* userdata = (struct user_data*)malloc(sizeof(struct user_data));
	memset(userdata, 0, sizeof(struct user_data));
	userdata->status = USER_STATUS_LOBBY;
	strcpy(userdata->access_token, access_token);

	int ret;
	khint_t k = kh_put(str, users_table, access_token, &ret);
	kh_value(users_table, k) = userdata;

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