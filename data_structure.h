#ifndef _DATA_STRUCTURES_H_
#define _DATA_STRUCTURES_H_

#include "library/klib/khash.h"

#define MAX_LENGTH_ACCESS_TOKEN 32


struct message_buffer {
	long type;
	long from;
	char buffer[1024];
};

struct user_data {
	long pk;
	long character_type;
	long level;
};

struct connected_user {
	long pk;
	long mq_id;
	long status;
	char access_token[MAX_LENGTH_ACCESS_TOKEN];
};

KHASH_MAP_INIT_INT(pk_int, struct user_data*);
KHASH_MAP_INIT_STR(str, struct connected_user*);

#endif