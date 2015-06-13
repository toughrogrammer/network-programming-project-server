#include "util.h"
#include "library/parson/parson.h"
#include <sys/time.h>


void clear_recv_buffer(int sock_client) {
	char buffer[MAX_LENGTH];
	memset(buffer, 0, sizeof(buffer));
	read(sock_client, buffer, MAX_LENGTH);
}

char* tokenizing_multi_character_delim(char* dst, char* src, char* delim) {
	char *next = strstr(src, delim);
	if( next == NULL ) {
		strcpy(dst, src);
		return NULL;
	}
	strncpy(dst, src, next - src);
	return next + strlen(delim);
}

ssize_t read_line(int fd, void *buffer, size_t n)
{
	ssize_t numRead;                    /* # of bytes fetched by last read() */
	size_t totRead;                     /* Total bytes read so far */
	char *buf;
	char ch;
	
	if (n <= 0 || buffer == NULL) {
		errno = EINVAL;
		return -1;
	}
	
	buf = buffer;                       /* No pointer arithmetic on "void *" */
	
	totRead = 0;
	for (;;) {
		numRead = read(fd, &ch, 1);
		if (numRead == -1) {
			if (errno == EINTR)         /* Interrupted --> restart read() */
				continue;
			else
				return -1;              /* Some other error */
		} else if (numRead == 0) {      /* EOF */
			if (totRead == 0)           /* No bytes read; return 0 */
				return 0;
			else                        /* Some bytes read; add '\0' */
				break;
		} else {                        /* 'numRead' must be 1 if we get here */
			if (totRead < n - 1) {      /* Discard > (n - 1) bytes */
				totRead++;
				*buf++ = ch;
			}
			if (ch == '\n')
				break;
		}
	}
	
	*buf = '\0';
	return totRead;
}

void str_tolower(char* str) {
	int length = (int)strlen(str);
	for( int i = 0; i < length; i ++ ) {
		if( str[i] >= 'A' && str[i] <= 'Z' ) {
			str[i] += 'a' - 'A';
		}
	}
}

int send_message_to_queue(key_t mq_key, long from, long to, const char* message) {
	struct message_buffer msg;
	memset(&msg, 0, sizeof(msg));
	msg.type = to;
	msg.from = from;
	strcpy(msg.buffer, message);

	return msgsnd(mq_key, (void *)&msg, sizeof(struct message_buffer), IPC_NOWAIT);
}

int check_message_queue(key_t mq_key, int id, struct message_buffer* msg) {
	return msgrcv( mq_key, (void *)msg, sizeof(struct message_buffer), id, IPC_NOWAIT);
}

void fill_connected_user(struct connected_user* user, int pk, int mq_id, int status, const char* access_token) {
	memset(user, 0, sizeof(struct connected_user));
	user->pk = pk;
	user->mq_id = mq_id;
	user->status = status;
	strcpy(user->access_token, access_token);
}

void build_simple_response(char* dest, int code) {
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);
	json_object_set_number(root_object, "result", code);

	serialize_json_to_response(dest, root_value);
	
	json_value_free(root_value);
}

long get_time_in_millisec() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
}

int validate_user(const char* access_token) {
	khint_t k = kh_get(str, connected_user_table, access_token);
	if( k == kh_end(connected_user_table) ) {
		return -1;
	}

	return 0;
}

void serialize_json_to_response(char *response, JSON_Value* json) {
	char *str = json_serialize_to_string(json);
	sprintf(response, "%s\r\n", str);
	json_free_serialized_string(str);
}