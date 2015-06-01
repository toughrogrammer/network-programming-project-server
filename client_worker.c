#include "client_worker.h"


int client_worker_main_loop(pid_t pid, int sock) {
	char buffer[MAX_LENGTH];
	while(1) {
		memset(buffer, 0, sizeof(buffer));
		ssize_t length = read_line(sock, buffer, MAX_LENGTH);
		if( length > 0 ) {
			// send message that is received from client to main server
			send_message_to_main_server(buffer);
		}

		// check message queue
		int msg_exist = check_message_queue(sock);

		// if exist message to send, write message to client
		if( msg_exist ) {
			if( ! send_message_to_client(sock, "") ) {

			} else {
				// error
			}
		}
	}

	return 0;
}

int send_message_to_main_server(char message[MAX_LENGTH]) {
	return 0;
}

int check_message_queue(int sock) {
	return 0;
}

int send_message_to_client(int sock, char message[MAX_LENGTH]) {
	write(sock, message, strlen(message));
	return 0;
}