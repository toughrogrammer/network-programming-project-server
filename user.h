#ifndef _USERS_H_
#define _USERS_H_

#include "data_structure.h"
#include "util.h"
#include "library/parson/parson.h"

struct user_data* find_user_data(const char* id, const char* password);
struct connected_user* find_connected_user_by_access_token(const char* access_token);
struct connected_user* find_connected_user_by_pk(int pk);
char* find_user_id_by_access_token(const char* access_token);
void print_users_status();
void get_user_list(int location, JSON_Array *arr);


#endif
