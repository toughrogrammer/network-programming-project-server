#include "listening.h"
#include "util.h"

int listening(pid_t pid, int port) {
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	int sock_listen = init_listening_socket(&servaddr, port);

	bind(sock_listen, (struct sockaddr *) &servaddr, sizeof(servaddr));
	
	listen(sock_listen, 10);

	while(1) {
		int sock_client = accept(sock_listen, (struct sockaddr*) NULL, NULL);
		printf("(%d) accpet : %d\n", pid, sock_client);
		if( sock_client < 0 ) {
			fprintf(stderr, "failed to accept client socket\n");
			continue;
		}
	}

	return 0;
}

int init_listening_socket(struct sockaddr_in* addr, int port) {
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = htons(INADDR_ANY);
	addr->sin_port = htons(port);

	int sock_listen = socket(AF_INET, SOCK_STREAM, 0);
	if( sock_listen < 0 ) {
		ERROR_LOGGING("failed to create listening socket")
	}

	int opt_sock_reuse = 1;
	if( setsockopt(sock_listen, SOL_SOCKET, SO_REUSEADDR, (char *)&opt_sock_reuse, (int)sizeof(opt_sock_reuse)) ) {
		ERROR_LOGGING("failed to setsockopt")
	}

	return sock_listen;
}