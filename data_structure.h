#ifndef _DATA_STRUCTURES_H_
#define _DATA_STRUCTURES_H_

#include "library/klib/khash.h"

#define MAX_LENGTH_ID 32
#define MAX_LENGTH_PASSWORD 128
#define MAX_LENGTH_ACCESS_TOKEN 32
#define MAX_LENGTH_GAME_ROOM_TITLE 32
#define MAX_GAME_ROOM_CAPACITY 4
#define MAX_GAME_ROUND 3
#define MAX_LENGTH_PROBLEM_SENTENCE 128

struct message_buffer {
	long type;
	long from;
	char buffer[512];
};

struct user_data {
	long pk;
	char id[MAX_LENGTH_ID];
	char password[MAX_LENGTH_PASSWORD];
	long character_type;
	long exp;
};

struct connected_user {
	long pk;
	long mq_id;
	long status;
	long pk_room;
	char access_token[MAX_LENGTH_ACCESS_TOKEN];
};

struct game_room {
	long pk_room;
	long capacity;
	long num_of_users;
	char title[MAX_LENGTH_GAME_ROOM_TITLE];
	long status;
	long member_pk_list[MAX_GAME_ROOM_CAPACITY];
	long total_round;
	long curr_round;
	long winner_of_round[MAX_GAME_ROUND];
	char problem[MAX_LENGTH_PROBLEM_SENTENCE];
	long timer;
};

KHASH_MAP_INIT_INT(pk_int, struct user_data*);
KHASH_MAP_INIT_STR(str, struct connected_user*);

KHASH_MAP_INIT_INT(pk_room, struct game_room*);

khash_t(pk_int) *user_table;
khash_t(str) *connected_user_table;
khash_t(pk_room) *game_room_table;

int num_of_problems;
char* problems[1024];

#endif
