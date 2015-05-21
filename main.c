#include <stdio.h>
#include "library/parson/parson.h"
#include "library/klib/khash.h"


void json_parser_test() {
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);
	char *serialized_string = NULL;
	json_object_set_string(root_object, "name", "John Smith");
	json_object_set_number(root_object, "age", 25);
	json_object_dotset_string(root_object, "address.city", "Cupertino");
	json_object_dotset_value(root_object, "contact.emails", json_parse_string("[\"email@example.com\",\"email2@example.com\"]"));
	serialized_string = json_serialize_to_string(root_value);
	puts(serialized_string);
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);
}

KHASH_MAP_INIT_INT(int, char);

void khash_test() {
	int ret, is_missing;
	khint_t k;
	khash_t(int) *hash_table = kh_init(int);  // allocate a hash table

	for( int i = 1; i <= 10; i += 2 ) {
		k = kh_put(int, hash_table, i, &ret);
	}

	for( int i = 1; i <= 10; i ++ ) {
		k = kh_get(int, hash_table, i);
		printf("kh_get : %d\n", k);
		if( k == kh_end(hash_table) ) {
			printf("%d is missing\n", i);
		}
		kh_del(int, hash_table, k);
	}

	for (k = kh_begin(hash_table); k != kh_end(hash_table); ++k)  // traverse
		if (kh_exist(hash_table, k))          // test if a bucket contains data
			kh_value(hash_table, k) = 1;

	kh_destroy(int, hash_table);              // deallocate the hash table
}

int main() {
	json_parser_test();
	khash_test();

	return 0;
}