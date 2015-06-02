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


void route_sign_up(JSON_Object *json, key_t mq_key, long target);
void route_sign_in(JSON_Object *json, key_t mq_key, long target);
void route_chatting(JSON_Object *json, key_t mq_key, long target);

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

	pid_t pid = fork();
	if( pid != 0 ) {
		return listening(pid, 10101);
	}

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

	if( ! send_message_to_queue(mq_key, MQ_ID_MAIN_SERVER, target, "response") ) {
		// success
	} else {
		// error
	}
}

void route_chatting(JSON_Object *json, key_t mq_key, long target) {
	
}