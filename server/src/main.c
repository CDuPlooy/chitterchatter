#include <stdlib.h>
#include <stdio.h>
#include "../lib/chttp.h"

int main(){
	p_custom_http chttp = chttp_init();
	chttp_add(chttp, "Mode: Relay\r\n", 13);
	chttp_add(chttp, "Target: abc\r\n", 14);
	chttp_finalise(chttp);

	printf("%s\n", chttp->buffer);
	chttp_destroy(chttp);
	return 0;
}
