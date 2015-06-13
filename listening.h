#ifndef _LISTENING_H_
#define _LISTENING_H_

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>


int listening(int port);
int init_listening_socket(struct sockaddr_in* addr, int port);

#endif