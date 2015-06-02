#ifndef _CLIENT_WORKER_H_
#define _CLIENT_WORKER_H_

#include <unistd.h>
#include <sys/types.h>
#include "util.h"
#include "data_structure.h"


int client_worker_main_loop(pid_t pid, int sock);
int send_message_to_main_server(key_t mq_key, int sock, char message[MAX_LENGTH]);
int send_message_to_client(int sock, char message[MAX_LENGTH]);

#endif