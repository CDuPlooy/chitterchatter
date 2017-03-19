#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "../lib/mnl.h"

int main(int argc, char **argv){
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in sa;
	sa.sin_port = htons(8080);
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr("127.0.0.1");

	connect(sock, (const struct sockaddr *)&sa, sizeof(sa));
	p_custom_http p = chttp_init();
	chttp_finalise(p, "RagingGrim", 10);
	sendAllFixed(sock, p->buffer, p->size, 0);
	chttp_destroy(p);

	p = chttp_init();
	chttp_add_header(p, "Server-Action: Broadcast", 24);
	chttp_finalise(p, "Hello", 5);
	sendAllFixed(sock, p->buffer, p->size, 0);
	
	return 0;
}
