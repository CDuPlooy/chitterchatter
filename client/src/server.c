#include "../lib/server.h"

p_custom_http httpProcess(int Socket){
	char *header = malloc(LARGEST_HEADER_SIZE);
	if(!header)
		return NULL;
	ssize_t rv = recvAllFixed(Socket, header, LARGEST_HEADER_SIZE, MSG_PEEK);
	if( rv == -1)
		goto clean_one;

	struct chttp c;
	c.buffer = header;
	c.size  = rv;
	char *eol = strstr(header, "\r\n\r\n");
	if(!eol){
		goto clean_one;
	}
	char *cl = chttp_lookup(&c, "Content-Length: ");
	if(!cl){
		goto clean_one;
	}

	size_t size = atol(cl);
	free(cl);
	size_t headerSize = eol + 4  - header;

	char *buffer = malloc((size + headerSize) * sizeof(char));
	if(!buffer)
		goto clean_two;

	p_custom_http p = malloc(sizeof(struct chttp));
	if(!p)
		goto clean_two;
	p->buffer = buffer;
	p->size = size + headerSize;


	rv = recvAllFixed(Socket, buffer, size + headerSize, 0);
	if( rv == -1)
		goto clean_two;
	else
		return p;



	clean_two:
		free(buffer);
	clean_one:
		free(header);
		return NULL;

}


void react( int Socket, const p_threadInfo ti, char *action, char* data){
	struct userData *u = ti->reserved;
	if(strcmp(action, "Broadcast") == 0){
		p_custom_http p = chttp_init();
		chttp_add_header(p, "Client-Action: Display", 22);

		char *username = getUsername(Socket, u->handles, u->usernames);
		if(!username){
			chttp_destroy(p);
			return;
		}
		size_t dataSize = strlen(data);
		size_t usernameSize = strlen(username);
		char *msg = malloc(dataSize + usernameSize + 10);
		if(!msg){
			chttp_destroy(p);
			return;
		}
		memcpy(msg, username, usernameSize);
		memcpy(msg+usernameSize+1,": ",2);
		memcpy(msg+usernameSize+3,data,dataSize);
		msg[usernameSize+3+dataSize+1] = '\0';

		chttp_finalise(p, msg, usernameSize+3+dataSize+1);


		size_t size = threadVector_getSize(u->handles);
		for(size_t i = 0 ; i < size ; i++){
			int *sock = threadVector_at(u->handles, i);
			if(!sock)
				continue; // Ignore
			else
				sendAllFixed(*sock, p->buffer, p->size, 0);
		}
		return;
	}
}

char *getUsername(int Sock, const p_threadVector handles,const p_threadVector usernames){
	size_t handlesSize = threadVector_getSize(handles);

	for(size_t i = 0 ; i < handlesSize ; i++){
			int sock = *(int *)threadVector_at(handles, i);
			if(sock == Sock)
				return threadVector_at(usernames, i);
	}
	return NULL;
}

void client_react(int Socket, const p_custom_http p){
	if(p == NULL)
		return;
	char *action = chttp_lookup(p, "Client-Action: ");
	if(action){
		if(strcmp("Display", action) == 0){
			char *d = chttp_getData(p);
			printf("%s",d);
			free(d);
		}
	}
	free(p);
}
