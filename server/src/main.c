#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "../lib/mnl.h"

#define N_THREADS 25

void *threadHandle(void *data){
	p_threadInfo ti = data;
	printf("t:%p\n", (void *)ti->queue);
	char a = getchar();
	a='a';
	void *data2 = NULL;
	while(!data2){
		
		data2 = threadQueue_dequeue(ti->queue);
	}

	logIt(ti->reserved, data2);
	return NULL;
}

int main(int argc , char **argv){
	logger *log = logger_init("log.log");

	logIt(log, "Application started\n");
	p_threadController tc = threadController_init();
	for(size_t i = 0 ; i < N_THREADS ; i++){
		threadController_pushback(tc, threadHandle, log);
	}

	threadController_messsage(tc, 1, "Hello world");

	char b = getchar();
	b = 'a';
	threadController_destroy(tc);
	logger_destroy(log);
	return 0;
}
