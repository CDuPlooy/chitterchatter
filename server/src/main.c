
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

#define N_THREADS 50


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
		sleep(1); // This sleep can be omitted if the server is under constant strain and can safely handle N_THREADS
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
			rv = select(n, &read_set, NULL, NULL, &t2);
			if(rv == 0){ // Another timeout better check the thread queue

			}else if(rv == -1){
				threadQueue_enqueue(ti->controllerQueue, "Client-Notify: Select Error!");
				close(client);
				continue;
			}else if(rv != -1){
				char *buffer = getPeerAddr((struct sockaddr *)&sa_in, sizeAddr);
				char *msg = malloc(250);
				snprintf(msg, 250, "Server-Notify: Connection from %s accepted!", buffer);
				free(buffer);
				buffer = msg;
				threadQueue_enqueue(ti->controllerQueue, buffer);
			 	p_custom_http p = httpProcess(client);
				if(p){
					char *username = chttp_getData(p);
					threadVector_push(u->usernames, username);		//Omitted error checking here.
					threadVector_push(u->handles, &client);
					chttp_destroy(p);
					react(client, ti, "Notify-Client-Connect", username,NULL);
					// react(968, ti, "ddos", "127.0.0.1", NULL);
					// Add a loop to parse the http while httpProcess is valid.
					do{
						p = httpProcess(client);
						if(p){
							char *action = chttp_lookup(p, "Server-Action: ");
							react(client, ti, chttp_lookup(p, "Server-Action: "), chttp_getData(p),p);

							threadQueue_enqueue(ti->controllerQueue, action);
							chttp_destroy(p);
						}else{
							threadQueue_enqueue(ti->controllerQueue, "Client-Notify: Client Disconnected! :(");
							react(client, ti, "Notify-Client-Disconnect", username, NULL);

							size_t size = threadVector_getSize(u->handles);
							for(size_t i = 0 ; i < size ; i++){
								int *p_handle = threadVector_at(u->handles, i);
								if(*p_handle == client){
									void *handle = threadVector_delete(u->handles, i);
									close(*(int *)handle);
									free(threadVector_delete(u->usernames, i));
									break;
								}
							}

						}
					}while(p != NULL);
					// close(client); //While loop will close the client
				}
				else{
					threadQueue_enqueue(ti->controllerQueue, "Client-Notify: Initial Handshake Failure!");
					close(client);
					return NULL;
				}

				// At this point handle the data by calling recv and send on client
				close(client);
			}
		}
	}

	return NULL;
}

int main(int argc , char **argv){
	p_threadController tc = threadController_init();
	p_threadVector usernames = threadVector_init();
	p_threadVector socketHandles = threadVector_init();

	assert(tc != NULL);
	assert(usernames != NULL);
	assert(socketHandles != NULL);


	int Sock = socket(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(8080);
	sa.sin_addr.s_addr = 0;
	int opt = 1;
	setsockopt(Sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
	bind(Sock, (const struct sockaddr *)&sa, sizeof(sa));
	listen(Sock, 5);
	struct userData u;
	u.sock = &Sock;
	u.usernames = usernames;
	u.handles = socketHandles;

	for(size_t i = 0 ; i < N_THREADS ; i++)
		threadController_pushback(tc, func, &u);


	while(1){
			char *data = threadQueue_dequeue(tc->controllerQueue);
			if(data){
				puts(data);
				if(!strstr(data,"Client-Notify: "))
					free(data);
			}
	}

	threadController_destroy(tc);

	threadVector_free(usernames); // Add a loop to delete all usernames here.
	threadVector_free(socketHandles);

	return 0;
}
