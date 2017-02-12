#include "../lib/chttp.h"

void *chttp_init(){
	p_custom_http chttp = malloc(sizeof(custom_http));
	if( !chttp )
		return NULL;

	chttp->buffer = NULL;
	chttp->size = 0;
	return chttp;
}

void chttp_destroy(p_custom_http chttp){
	memset(chttp->buffer, 0, chttp->size);
	free(chttp->buffer);
	chttp->buffer = NULL;
	chttp->size = 0;

	free(chttp);
	chttp = NULL;
}

const short chttp_add(p_custom_http chttp, const char *data, const size_t size){
	size_t oldSize = chttp->size;
	short err = chttp_grow(chttp, size);
	if( err != CHTTPE_OK )
		return CHTTPE_GROW;

	memcpy(chttp->buffer + oldSize, data, size);
	return CHTTPE_OK;
}


const short chttp_grow(p_custom_http chttp, const size_t size){
	char *old = chttp->buffer;
	chttp->buffer = realloc(chttp->buffer, (chttp->size + size) * sizeof(char));

	if( !chttp->buffer ){
		chttp->buffer = old;
		return CHTTPE_GROW;
	}

	chttp->size += size;
	return CHTTPE_OK;
}

char *chttp_lookup(const p_custom_http chttp, const char *key){
	char *location = strstr(chttp->buffer, key);
	if( !location )
		return NULL;

	location += strlen(key);
	char *end = strstr(location, "\r\n");
	if( !end )
		return NULL;

	size_t size = (end - location + 1) * sizeof(char);

	char *newBuffer = malloc( size );

	if( !newBuffer )
		return NULL;

	memcpy(newBuffer, location, size - 1);


	newBuffer[size - 1] = '\0';

	return newBuffer;
}
