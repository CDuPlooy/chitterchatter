#ifndef SERVER_H
#define SERVER_H
// Includes
#include "mnl.h"
// Constants
#define LARGEST_HEADER_SIZE  1024
// Functions
p_custom_http httpProcess(int Socket);
void react( int Socket, const p_threadInfo ti, char *action, char* data, const p_custom_http x);
char *getUsername(int Sock, const p_threadVector handles,const p_threadVector usernames);
// Structures
struct userData{
	p_threadVector usernames;
	p_threadVector handles;
	int *sock;
};
#endif
