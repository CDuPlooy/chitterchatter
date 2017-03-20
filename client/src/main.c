#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <assert.h>
#include <arpa/inet.h>
#include "../lib/mnl.h"
#include "../lib/server.h"

int KEEP_GOING = 1;

void *f(void *d){
	p_threadInfo ti = d;
	while(1){
		p_custom_http p = httpProcess(*(int *)ti->reserved);
		client_react(*(int *)ti->reserved, p);
	}
	return NULL;
}

int main(int argc,const char **argv){
	basic_map *map= mapCreate_fromParams(argc, argv);
	assert(map != NULL);
	assert(mapKeyLookup(map, "--nick") != NULL);
	assert(mapKeyLookup(map,"--port") != NULL);
	assert(mapKeyLookup(map,"--host") != NULL);

	char buffer[1024];
	p_threadController tc = threadController_init();

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in sa;
	sa.sin_port = htons(atoi(mapKeyLookup(map,"--port")));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr(mapKeyLookup(map,"--host"));

	connect(sock, (const struct sockaddr *)&sa, sizeof(sa));
	p_custom_http p = chttp_init();
	chttp_finalise(p, mapKeyLookup(map, "--nick"), strlen(mapKeyLookup(map, "--nick")));
	sendAllFixed(sock, p->buffer, p->size, 0);
	chttp_destroy(p);

	threadController_pushback(tc, f, &sock);
	while(KEEP_GOING == 1){
		fgets(buffer, 1024, stdin);
		if(buffer[0] != '/'){
			p = chttp_init();
			chttp_add_header(p, "Server-Action: Broadcast", 24);
			chttp_finalise(p, buffer, strlen(buffer));
			sendAllFixed(sock, p->buffer, p->size, 0);
			chttp_destroy(p);
		}else
			handle(buffer, sock);

	}
	threadController_destroy(tc);
	mapDestroy(map);
	return 0;
}
