#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "../lib/mnl.h"

#define N_THREADS 25

void *threadHandle(void *data){
	p_threadInfo ti = data;
	sleep(5);

	logIt(ti->reserved, threadQueue_dequeue(ti->queue));
	logIt(ti->reserved, threadQueue_dequeue(ti->queue)); // Log the dequeued message by using logger object ( thread safe )

	// Cleanup? Trampoline function handles this!
	return NULL;
}

int main(int argc , char **argv){
	logger *log = logger_init("log.log"); // Create a logger object which logs to log.log

	logIt(log, "Application started\n");
	p_threadController tc = threadController_init(); // Create a thread Controller
	for(size_t i = 0 ; i < N_THREADS ; i++){
		threadController_pushback(tc, threadHandle, log); // Create a thread which executes on threadHandle and passes log as a reserved object.
		threadController_messsage(tc, i, "Hello world\n");
		threadController_messsage(tc, i, "Bye world\n");
	}

	threadController_destroy(tc); // Cleanup of all threads ; ( Disconnects the controller ; threads can keep running though )
	logger_destroy(log); // Destroy the log object.
	return 0;
}
