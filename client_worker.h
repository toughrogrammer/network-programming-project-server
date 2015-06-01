#ifndef _CLIENT_WORKER_H_
#define _CLIENT_WORKER_H_

#include <unistd.h>
#include "util.h"


int client_worker_main_loop(pid_t pid, int sock);
int send_message_to_main_server(char message[MAX_LENGTH]);
int check_message_queue(int sock);
int send_message_to_client(int sock, char message[MAX_LENGTH]);

#endif