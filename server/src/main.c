#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "../lib/mnl.h"

#define N_THREADS 25

void *threadHandle(void *data){
	p_logger log = data;
	logIt(log, "Hello from thread!\n");
	return NULL;
}

int main(int argc , char **argv){
	logger *log = logger_init("log.log");
	logIt(log, "Application started\n");
	p_threadController tc = threadController_init();
	for(size_t i = 0 ; i < N_THREADS ; i++)
		threadController_pushback(tc, threadHandle, log);

	threadQueue_enqueue(vvector_at(tc->threadQueues, 0), "ok");

	char b = getchar();
	b = 'a';
	threadController_destroy(tc);
	logger_destroy(log);
	return 0;
}
