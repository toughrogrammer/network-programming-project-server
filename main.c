#include <stdio.h>
#include <unistd.h>
#include "library/parson/parson.h"
#include "library/klib/khash.h"
#include "listening.h"


int main() {
	pid_t pid = fork();
	if( pid != 0 ) {
		return listening(pid, 10101);
	}


	return 0;
}