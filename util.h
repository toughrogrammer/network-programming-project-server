#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "data_structure.h"


#define ERROR_LOGGING(content) { fprintf(stderr, "%d : %s\n", __LINE__, (content)); exit(1); }

#define MAX_LENGTH 4096


void clear_recv_buffer(int sock_client);
char* tokenizing_multi_character_delim(char* dst, char* src, char* delim);
ssize_t read_line(int fd, void *buffer, size_t n);
void str_tolower(char* str);
int send_message_to_queue(key_t mq_key, long from, long to, const char* message);
int check_message_queue(key_t mq_key, int id, struct message_buffer* msg);
void fill_connected_user(struct connected_user* user, int pk, int mq_id, int status, const char* access_token);
void build_simple_response(char* dest, int code);
long get_time_in_millisec();
int validate_user(const char* access_token);

#endif
