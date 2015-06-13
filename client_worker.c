#include "client_worker.h"
#include "constants.h"
#include "data_structure.h"
#include "MemLog.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/ioctl.h>
#include <sys/time.h>


int client_worker_main_loop(int sock) {
	key_t msg_queue_key_id = msgget((key_t)MQ_KEY, IPC_CREAT | 0666);
	if( msg_queue_key_id == -1 ) {
		// error!
		return 1;
	}

	pid_t pid = getpid();

	char tmp[maxstr];
	sprintf( tmp, "Client Accept ; Process : %d\n", pid);
	PushLog(tmp);

	long prev, curr;
	prev = get_time_in_millisec();

	int ping_sent = 0;

	while(1) {
		sleep(0);

		curr = get_time_in_millisec();
		
		// ping pong check
		if( curr - prev > 3000 && ping_sent == 0 ) {
			prev = curr;
			ping_sent = 1;
			send_ping(sock);
		}
		if( ping_sent && curr - prev > 5000 ) {
			break;
		}
		
		// read when there are bytes in receive queue
		int readable_bytes;
		ioctl(sock, FIONREAD, &readable_bytes);
		if( readable_bytes > 0 ) {
			char buffer[MAX_LENGTH] = { 0, };
			ssize_t length = read_line(sock, buffer, MAX_LENGTH);
			if( length > 0 ) {
				if( ping_sent && strcmp(buffer, "pong\r\n") == 0 ) {
					ping_sent = 0;
				} else {
					// send message that is received from client to main server
					send_message_to_main_server(msg_queue_key_id, sock, buffer);
				}
			} else {
			}
		}

		// check message queue
		struct message_buffer msg_response;
		memset(&msg_response, 0, sizeof(struct message_buffer));
		int msg_exist = check_message_queue(msg_queue_key_id, sock, &msg_response);

		// if exist message to send, write message to client
		if( msg_exist != -1 ) {
			if( ! send_message_to_client(sock, msg_response.buffer) ) {

			} else {
				// failed to message over socket to client
			}
		}
	}

	printf("(cw) quit %d\n", pid);
	return 0;
}

int send_message_to_main_server(key_t mq_key, int sock, char message[MAX_LENGTH]) {
	if( ! send_message_to_queue(mq_key, sock, MQ_ID_MAIN_SERVER, message) ) {
		// success
		return 0;
	}

	return -1;
}

int send_message_to_client(int sock, char message[MAX_LENGTH]) {
	printf("send_message_to_client : %s", message);
	int result = write(sock, message, strlen(message));
	printf("\t(%d)write : %d\n", sock, result);
	return 0;
}

void send_ping(int sock) {
	send_message_to_client(sock, "ping\r\n");
}