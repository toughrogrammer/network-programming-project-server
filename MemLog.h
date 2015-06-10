#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include<time.h>

#define maxstr 200

int RegMem( const char*, const char*, const int );
int UpdateExp( const int, const int );
void PushLog( const char * );
