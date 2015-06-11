#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include<time.h>

#define maxstr 200

#define RESULT_REGISTER_EXIST_ID 0
#define RESULT_REGISTER_SUCCESS 1


int RegMem( const char*, const char*, const int );
int UpdateExp( const int, const int );
void PushLog( const char * );
