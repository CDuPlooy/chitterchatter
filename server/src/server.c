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
