#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

///////********* THIS A TEMPLATE FOR UDP SERVER FROM TUT.9 P.56-58 **************///////////
///////////////////////////////////////////////////////////////////////////////////////////
#define ECHOMAX 255 /* Longest string to echo */
int main(int argc, char* argv[]) {
	int sock; /* Socket */
	struct sockaddr_in echoServAddr; /* Local address */
	struct sockaddr_in echoClntAddr; /* Client address */
	unsigned int cliAddrLen; /* Length of incoming message */
	char echoBuffer[ECHOMAX]; /* Buffer for echo string */
	unsigned short echoServPort; /* Server port */
	int recvMsgSize; /* Size of received message */
	
	/* Create socket for sending/receiving datagrams */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		error("socket() failed");

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
			error("recvfrom() failed"); //TODO: handle error situations in errors.cpp
		printf("Handling client %s\n", //TODO: need to examine between first message WRQ--> open an inode
									   //      and other messages(DATA)
			inet_ntoa(echoClntAddr.sin_addr));
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