#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include "../lib/mnl.h"

#define N_THREADS 25

void *threadHandle(void *data){
	p_threadInfo ti = data;
	threadQueue_enqueue(ti->controllerQueue, "Starting!");
	return NULL;
}

int main(int argc , char **argv){
	p_threadController tc = threadController_init();
	assert(tc != NULL);

	for(size_t i = 0 ; i < N_THREADS ; i++)
		threadController_pushback(tc, threadHandle, NULL);

	while(1 == 1){
		puts("Checking Queue");
		char *val = threadQueue_dequeue(tc->controllerQueue);
		puts("Ere");
		if(val)
			puts(val);
		else{
			puts("No new data! Sleeping for a bit!");
		}
	}

	threadController_destroy(tc);
	return 0;
}
