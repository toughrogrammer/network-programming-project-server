#ifndef _DATA_STRUCTURES_H_
#define _DATA_STRUCTURES_H_

#ifndef MAX_LENGTH
#define MAX_LENGTH 4096
#endif

struct message_buffer {
	long type;
	int sock;
	char buffer[1024];
};

#endif