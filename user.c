#include "user.h"


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
	khint_t k = kh_get(str, connected_user_table, access_token);
	if( k == kh_end(connected_user_table) ) {
		return NULL;
	}
	struct user_data* userdata = kh_value(user_table, k);

	return userdata->id;
}

void print_users_status() {
	printf("--users--\n");
	for (khint_t k = kh_begin(connected_user_table); k != kh_end(connected_user_table); ++k) {
		if (kh_exist(connected_user_table, k)) {
			struct connected_user* user = kh_value(connected_user_table, k);
			printf("%d %d %d %s\n", (int)user->mq_id, (int)user->pk, (int)user->status, user->access_token);
		}
	}
	printf("---------\n");
}

void get_user_list(int location, JSON_Array *arr) {
	if( location == 0 ) {
		// lobby users
	} else {
		// room users
	}
}