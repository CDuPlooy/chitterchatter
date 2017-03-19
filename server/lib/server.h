#ifndef SERVER_H
#define SERVER_H
// Includes
#include "mnl.h"
// Constants
#define LARGEST_HEADER_SIZE  1024
// Functions
p_custom_http httpProcess(int Socket);
#endif
