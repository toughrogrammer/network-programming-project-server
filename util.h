#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define ERROR_LOGGING(content) { fprintf(stderr, "%d : %s\n", __LINE__, (content)); exit(1); }

#define MAX_LENGTH 4096


void clear_recv_buffer(int sock_client);
char* tokenizing_multi_character_delim(char* dst, char* src, char* delim);
ssize_t read_line(int fd, void *buffer, size_t n);
void str_tolower(char* str);

#endif
