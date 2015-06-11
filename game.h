#ifndef _GAME_H_
#define _GAME_H_

#include "util.h"
#include "data_structure.h"
#include "library/klib/khash.h"
#include "library/parson/parson.h"


void get_room_list(JSON_Array *arr);
struct game_room* find_game_room_by_pk(long pk);

void update_game_rooms(key_t mq_key, long dt);
void handle_game_room_waiting(key_t mq_key, struct game_room* room);
void handle_game_room_ready(key_t mq_key, struct game_room* room);
void handle_game_room_playing(key_t mq_key, struct game_room* room);
void handle_game_room_showing_round_result(key_t mq_key, struct game_room* room);
void handle_game_room_showing_total_result(key_t mq_key, struct game_room* room);
void notify_game_start(key_t mq_key, struct game_room* room);
void notify_round_start(key_t mq_key, struct game_room* room);
void notify_round_end(key_t mq_key, struct game_room* room);
void notify_game_end(key_t mq_key, struct game_room* room);
long create_game_room(const char* title);
int remove_game_room(long pk);
int join_game_room(long pk_room, long pk_user);
int leave_game_room(struct connected_user* user);
int start_game(long pk_room);
int end_game(long pk_room);
void request_room_update(key_t mq_key, long pk_room);

static long next_pk_room = 1;

#endif