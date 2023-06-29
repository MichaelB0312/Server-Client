#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <limits.h>
#include <iostream>
#include "connections.h"

#define WRQ_OP 0x02
#define ACK_OP 0x04
#define DATA_OP 0x03



int get_msg(char* buffer, unsigned int cliAddrLen) {

	// first find
	//see opcode:
	// if( buffer[0] == WRQ_OP)
	return 0;
}
