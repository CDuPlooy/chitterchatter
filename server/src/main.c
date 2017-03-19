#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <sys/stat.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>

#include "../lib/mnl.h"
#include "../lib/server.h"

#define N_THREADS 20
struct userData{
	p_logger log;
	int *sock;
};

void *func(void *data){
	p_threadInfo ti = data;
	struct userData *u = ti->reserved;
	struct sockaddr_in sa_in;
	memset(&sa_in,0,sizeof(sa_in));
	socklen_t sizeAddr = sizeof(struct sockaddr);

	// use select to see if I can accept.
	struct timeval t;
	struct timeval t2;
	t2.tv_sec = 4;
	t2.tv_usec = 0;
	t.tv_sec = 2;
	t.tv_usec = 0;



	fd_set accept_set, read_set;
	while(1){
		FD_ZERO(&accept_set);
		FD_SET(*((int *)u->sock),&accept_set);
		int n = *((int *)u->sock) + 1;
		int rv = select(n, &accept_set, NULL, NULL, &t);
		if(rv == 0){ // If a timeout occurs on initial connect ; check if the main thread sent something.

		}else if(rv == -1){ // Some sort of other error.
			threadQueue_enqueue(ti->controllerQueue, "Client-Notify: Connection Timeout!");
			continue;
		}
		else{ // A connection is waiting! handle it!
			int client = accept(*((int *)u->sock), (struct sockaddr *)&sa_in, &sizeAddr);
			FD_ZERO(&read_set);
			FD_SET(client,&read_set);

			t.tv_sec = 4;
			n = client + 1;
			rv = select(n, &read_set, NULL, NULL, &t);
			if(rv == 0){ // Another timeout better check the thread queue
				threadQueue_enqueue(ti->controllerQueue, "Client-Notify: Read Timeout!");
				close(client);
			}else if(rv == -1){
				threadQueue_enqueue(ti->controllerQueue, "Client-Notify: Select error!");
				close(client);
				continue;
			}else if(rv != -1){
				char *buffer = getPeerAddr((struct sockaddr *)&sa_in, sizeAddr);
				threadQueue_enqueue(ti->controllerQueue, buffer);
			 	p_custom_http p = httpProcess(client);
				puts(chttp_getData(p));
				chttp_destroy(p);
				// At this point handle the data by calling recv and send on client
				close(client);
			}
		}
	}

	return NULL;
}

int main(int argc , char **argv){
	p_threadController tc = threadController_init();
	p_logger log = logger_init(NULL);
	assert(tc != NULL);


	int Sock = socket(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(8080);
	sa.sin_addr.s_addr = 0;
	int opt = 1;
	setsockopt(Sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
	bind(Sock, (const struct sockaddr *)&sa, sizeof(sa));
	listen(Sock, 5);
	struct userData u[N_THREADS];

	for(size_t i = 0 ; i < N_THREADS ; i++){
		u[i].log = log;
		u[i].sock = &Sock;
		threadController_pushback(tc, func, &u[i]);
	}

	while(1){
			char *data = threadQueue_dequeue(tc->controllerQueue);
			if(data){
				puts(data);
				if(!strstr(data,"Client-Notify: "))
					free(data);
			}
	}

	threadController_destroy(tc);
	logger_destroy(log);
	return 0;
}
