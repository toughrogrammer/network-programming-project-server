#include "user.h"
#include "constants.h"


struct user_data* find_user_data(const char* id, const char* password) {
	for (khint_t k = kh_begin(user_table); k != kh_end(user_table); ++k) {
		if (kh_exist(user_table, k)) {
			struct user_data* userdata = kh_value(user_table, k);
			if( strcmp(userdata->id, id) == 0 && strcmp(userdata->password, password) == 0 ) {
				return userdata;
			}
		}
	}

	// not found
	return NULL;
}

struct user_data* find_user_data_by_pk(long pk) {
	for (khint_t k = kh_begin(user_table); k != kh_end(user_table); ++k) {
		if (kh_exist(user_table, k)) {
			struct user_data* userdata = kh_value(user_table, k);
			if( userdata->pk == pk ) {
				return userdata;
			}
		}
	}

	// not found
	return NULL;
}

struct connected_user* find_connected_user_by_access_token(const char* access_token) {
	khint_t k = kh_get(str, connected_user_table, access_token);
	if( k == kh_end(connected_user_table) ) {
		return NULL;
	}

	return kh_value(connected_user_table, k);
}

struct connected_user* find_connected_user_by_pk(int pk) {
	for (khint_t k = kh_begin(connected_user_table); k != kh_end(connected_user_table); ++k) {
		if (kh_exist(connected_user_table, k)) {
			struct connected_user* userdata = kh_value(connected_user_table, k);
			if( userdata->pk == pk ) {
				return userdata;
			}
		}
	}

	return NULL;
}

char* find_user_id_by_access_token(const char* access_token) {
	struct connected_user* connected_user = find_connected_user_by_access_token(access_token);
	if( connected_user == NULL ) {
		return NULL;
	}

	struct user_data* userdata = find_user_data_by_pk(connected_user->pk);
	if( userdata == NULL ) {
		return NULL;
	}

	return userdata->id;
}

char* find_user_id_by_pk(int pk) {
	for (khint_t k = kh_begin(user_table); k != kh_end(user_table); ++k) {
		if (kh_exist(user_table, k)) {
			struct user_data* userdata = kh_value(user_table, k);
			if( userdata->pk == pk ) {
				return userdata->id;
			}
		}
	}

	return NULL;
}

void get_user_info_by_pk(long pk, JSON_Object *user_info) {
	for (khint_t k = kh_begin(user_table); k != kh_end(user_table); ++k) {
		if (kh_exist(user_table, k)) {
			struct user_data* userdata = kh_value(user_table, k);
			if( userdata->pk == pk ) {
				json_object_set_string(user_info, "user_id", userdata->id);
				json_object_set_number(user_info, "character_type", userdata->character_type);
				json_object_set_number(user_info, "exp", userdata->exp);
			}
		}
	}
}

void print_users_status() {
	printf("--users--\n");
	for (khint_t k = kh_begin(connected_user_table); k != kh_end(connected_user_table); ++k) {
		if (kh_exist(connected_user_table, k)) {
			struct connected_user* user = kh_value(connected_user_table, k);
			printf("%d %d %d %s %d\n", (int)user->mq_id, (int)user->pk, (int)user->status, user->access_token, (int)user->pk_room);
		}
	}
	printf("---------\n");
}

void get_lobby_user_list(JSON_Array *arr) {
	// lobby users
	for (khint_t k = kh_begin(connected_user_table); k != kh_end(connected_user_table); ++k) {
		if (kh_exist(connected_user_table, k)) {
			struct connected_user* userdata = kh_value(connected_user_table, k);
			if( userdata->status == USER_STATUS_LOBBY ) {
				JSON_Value *user_info = json_value_init_object();
				JSON_Object *user_object = json_value_get_object(user_info);
				get_user_info_by_pk(userdata->pk, user_object);
				json_array_append_value(arr, user_info);
			}
		}
	}
}

void get_room_user_list(long room_id, JSON_Array *arr) {
	//room users
	for (khint_t k = kh_begin(connected_user_table); k != kh_end(connected_user_table); ++k) {
		if (kh_exist(connected_user_table, k)) {
			struct connected_user* userdata = kh_value(connected_user_table, k);
			if( userdata->status == USER_STATUS_IN_ROOM 
				&& userdata->pk_room == room_id ) {
				JSON_Value *user_info = json_value_init_object();
				JSON_Object *user_object = json_value_get_object(user_info);
				get_user_info_by_pk(userdata->pk, user_object);
				json_array_append_value(arr, user_info);
			}
		}
	}
}

void broadcast_lobby(key_t mq_key, char* message) {
	// lobby users
	for (khint_t k = kh_begin(connected_user_table); k != kh_end(connected_user_table); ++k) {
		if (kh_exist(connected_user_table, k)) {
			struct connected_user* userdata = kh_value(connected_user_table, k);
			if( userdata->status == USER_STATUS_LOBBY ) {
				//broadcasting
				send_message_to_queue(mq_key, MQ_ID_MAIN_SERVER, userdata->mq_id, message);
			}
		}
	}
}

void broadcast_room(key_t mq_key, char* message, int pk_room) {
	printf("broadcast_room : %s", message);

	// room users
	for (khint_t k = kh_begin(connected_user_table); k != kh_end(connected_user_table); ++k) {
		if (kh_exist(connected_user_table, k)) {
			struct connected_user* userdata = kh_value(connected_user_table, k);
			if( userdata->status == USER_STATUS_IN_ROOM 
				&& userdata->pk_room == pk_room ) {
				//broadcasting
				send_message_to_queue(mq_key, MQ_ID_MAIN_SERVER, userdata->mq_id, message);
			}
		}
	}	
}
