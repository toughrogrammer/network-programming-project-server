#ifndef _DATA_STRUCTURES_H_
#define _DATA_STRUCTURES_H_

#include "library/klib/khash.h"


struct message_buffer {
	long type;
	long from;
	char buffer[1024];
};

struct user_data {
	long status;
	long character_type;
	long level;
	char access_token[32];
};

KHASH_MAP_INIT_STR(str, struct user_data*);

#endif