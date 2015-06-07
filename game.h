#ifndef _GAME_H_
#define _GAME_H_

#include "util.h"
#include "data_structure.h"
#include "library/klib/khash.h"
#include "library/parson/parson.h"


void get_room_list(JSON_Array *arr);
void update_game_rooms(long dt);
void handle_game_room_ready(struct game_room* room);
void handle_game_room_playing(struct game_room* room);
void handle_game_room_showing_round_result(struct game_room* room);
void handle_game_room_showing_total_result(struct game_room* room);
void notify_round_end(struct game_room* room);


#endif