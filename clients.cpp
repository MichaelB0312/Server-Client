#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <limits.h>
#include <iostream>

bool isWRQ(char* clientMSG, unsigned MSGlen) {

	for (size_t i = 0; i < MSGlen; ++i) {
		char character = clientMSG[i];
		//check it
		if (character >= 0x20 && character <= 0x7E)
}