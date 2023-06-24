#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <limits.h>
#include <iostream>

using namespace std;

///////********* THIS A TEMPLATE FOR UDP SERVER FROM TUT.9 P.56-58 **************///////////
///////////////////////////////////////////////////////////////////////////////////////////
#define ECHOMAX 516 /* Longest string to echo */

int main(int argc, char* argv[]) {
	int sock; /* Socket */
	struct sockaddr_in echoServAddr; /* Local address */
	struct sockaddr_in echoClntAddr; /* Client address */
	unsigned int cliAddrLen; /* Length of incoming message */
	char echoBuffer[ECHOMAX]; /* Buffer for echo string */
	/* Recieve Parameters*/
	if (argc < 4) {
		cerr << "TTFTP_ERROR: illegal arguments" << endl;
		exit(1);
	}
	if ((atoi(argv[1]) > USHRT_MAX || atoi(argv[1]) < 0) ||
		(atoi(argv[2]) > USHRT_MAX || atoi(argv[2]) < 0) ||
		(atoi(argv[3]) > USHRT_MAX || atoi(argv[3]) < 0)) {
		cerr << "TTFTP_ERROR: illegal arguments" << endl;
		exit(1);
	}
	unsigned short echoServPort = atoi(argv[1]); /* Server port */
	unsigned short timeout = atoi(argv[2]);
	unsigned short max_num_of_resends = atoi(argv[3]);
	
	int WRQ_flag = 0; int ACK_flag = 0;
	int recvMsgSize; /* Size of received message */
	
	/* Create socket for sending/receiving datagrams */
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		perror("TTFTP_ERROR: socket() failed");

	/* Construct local address structure */
	/* Zero out structure */
	memset(&echoServAddr, 0, sizeof(echoServAddr));
	/* Internet address family */
	echoServAddr.sin_family = AF_INET;
	/* Any incoming interface */
	echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	/* Local port */
	echoServAddr.sin_port = htons(echoServPort);
	/* Bind to the local address */
	if (bind(sock, (struct sockaddr*)&echoServAddr,
		sizeof(echoServAddr)) < 0)
		error("bind() failed");

	for (;;) { /* Run forever */
		/* Set the size of the in-out parameter */
		cliAddrLen = sizeof(echoClntAddr);
		/* Block until receive message from a client */
		if ((recvMsgSize = recvfrom(sock, echoBuffer, ECHOMAX, 0,
			(struct sockaddr*)&echoClntAddr, &cliAddrLen)) < 0) //TODO: maybe we need to use select here or in errors.cpp
			error("TTFTP_ERROR: recvfrom() failed"); //TODO: handle error situations in errors.cpp
		//TODO: need to examine between first message WRQ--> open an inode
		//      and other messages(DATA)
		
		// check if it's WRQ packet:(out of ASCII range)
		if (echoBuffer[0] < 0x20 && echoBuffer[0] > 0x7E) {
			WRQ_flag = 1;
		}
		else ACK_flag = 1;
		/* Send received datagram back to the client */
		//TODO: send ACK{0..1..2} to client
		if (sendto(sock, echoBuffer, recvMsgSize, 0,
			(struct sockaddr*)&echoClntAddr,
			sizeof(echoClntAddr)) != recvMsgSize)
			error("sendto() sent a different number of bytes than expected");
	}
	/* NOT REACHED */
}

///////********* END OF TEMPLATE FOR UDP SERVER **************///////////
///////////////////////////////////////////////////////////////////////////////////////////