#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include "../lib/mnl.h"

#define N_THREADS 50
struct userData{
p_logger log;
int *sock;
};

void *func(void *data){
	p_threadInfo ti = data;
	struct userData *u = ti->reserved;
	struct sockaddr_in sa_in;
	memset(&sa_in,0,sizeof(sa_in));
	socklen_t sizeAddr = 0;
	while(1){
		int client = accept(*(int *)u->sock, (struct sockaddr *)&sa_in, &sizeAddr);
		printf("%ul||%ul||%ul\n", INET_ADDRSTRLEN, INET6_ADDRSTRLEN,sizeAddr);

		char *buffer = malloc(250);
		get_ip_str((struct sockaddr *)&sa_in, buffer, sizeAddr);

		puts(buffer);
		close(client);
	}
}


int main(int argc , char **argv){
	p_threadController tc = threadController_init();
	p_logger log = logger_init(NULL);
	assert(tc != NULL);


	int Sock = socket(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(95);
	sa.sin_addr.s_addr = 0;

	bind(Sock, (const struct sockaddr *)&sa, sizeof(sa));

	listen(Sock, 5);
	struct userData u[N_THREADS];

	for(size_t i = 0 ; i < N_THREADS ; i++){
		u[i].log = log;
		u[i].sock = &Sock;
		threadController_pushback(tc, func, &u[i]);
	}


	threadController_destroy(tc);
	logger_destroy(log);
	return 0;
}
