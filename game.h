#ifndef _GAME_H_
#define _GAME_H_

#include "util.h"
#include "data_structure.h"
#include "library/klib/khash.h"
#include "library/parson/parson.h"


void get_room_list(JSON_Array *arr);
void update_game_rooms(key_t mq_key, long dt);
void handle_game_room_ready(key_t mq_key, struct game_room* room);
void handle_game_room_playing(key_t mq_key, struct game_room* room);
void handle_game_room_showing_round_result(key_t mq_key, struct game_room* room);
void handle_game_room_showing_total_result(key_t mq_key, struct game_room* room);
void notify_game_start(key_t mq_key, struct game_room* room);
void notify_round_end(key_t mq_key, struct game_room* room);


#endif