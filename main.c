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


void route_sign_up(JSON_Object *json);

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
		struct message_buffer msg;
		if (msgrcv( msg_queue_key_id, (void *)&msg, sizeof(struct message_buffer), MQ_ID_MAIN_SERVER, IPC_NOWAIT) != -1) {
	        JSON_Value *json_value = json_parse_string(msg.buffer);
			if( json_value == NULL ) {
				// failed to parsing message
				printf("(sock:%d) failed to parsing\n", msg.sock);
				continue;
			}

			JSON_Object *json_body = json_value_get_object(json_value);
			int msg_target = (int)json_object_get_number(json_body, "target");
			switch(msg_target) {
			case MSG_TARGET_SIGN_UP:
				route_sign_up(json_body);
				break;
			}

			json_value_free(json_value);
        }
	}


	return 0;
}

void route_sign_up(JSON_Object *json) {
	printf("(main) route_sign_up\n");
}