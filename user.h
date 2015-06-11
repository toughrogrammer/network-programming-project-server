#ifndef _USERS_H_
#define _USERS_H_

#include "data_structure.h"
#include "util.h"
#include "library/parson/parson.h"

struct user_data* find_user_data(const char* id, const char* password);
struct user_data* find_user_data_by_pk(long pk);
struct connected_user* find_connected_user_by_access_token(const char* access_token);
struct connected_user* find_connected_user_by_pk(int pk);
char* find_user_id_by_access_token(const char* access_token);
char* find_user_id_by_pk(int pk);
void get_user_info_by_pk(long pk, JSON_Object *user_info);
void print_users_status();
void get_lobby_user_list(JSON_Array *arr);
void get_room_user_list(long room_id, JSON_Array *arr);
void broadcast_lobby(key_t mq_key, char* message);
void broadcast_room(key_t mq_key, char* message, int pk_room);


#endif
