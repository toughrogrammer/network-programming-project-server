#ifndef _GAME_H_
#define _GAME_H_

#include "util.h"
#include "data_structure.h"
#include "library/klib/khash.h"
#include "library/parson/parson.h"


void get_room_list(JSON_Array *arr);
void update_game_rooms(long dt, khash_t(pk_room) *game_room_table);
void notify_round_end(struct game_room* room);


#endif