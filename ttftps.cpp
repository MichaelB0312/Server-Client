#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <limits.h>
#include <iostream>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fstream>

using namespace std;

#define WRQ_OP 0x02
#define ACK_OP 0x04
#define DATA_OP 0x03

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
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("TTFTP_ERROR: socket() failed");
		exit(1);
	}


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
		// Set up fd_set for select()
		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(sock, &readfds);
		// Set up timeout
		struct timeval timer;
		timer.tv_sec = (time_t)timeout;
		timer.tv_usec = 0;

		/* Select is Blocking until receive message from a client */
		int readyCheck = select(sockfd + 1, &readfds, NULL, NULL, &timeout);

		if (readyDescriptors == -1) {
			perror("TTFTP_ERROR: select() error");
			exit(1);
		}
		else if (readyDescriptors == 0) {
			//send ACK again. increase failure counter
		}

		//if Server socket is ready, get the packet:
		if ((recvMsgSize = recvfrom(sock, echoBuffer, ECHOMAX, 0,
			(struct sockaddr*)&echoClntAddr, &cliAddrLen)) < 0) { //TODO: maybe we need to use select here or in errors.cpp
			perror("TTFTP_ERROR: recvfrom() failed"); //TODO: handle error situations in errors.cpp
			exit(1);
		}
		//TODO: need to examine between first message WRQ--> open an inode
		//      and other messages(DATA)

		// check if it's WRQ packet:
		if (echoBuffer[0] == WRQ_OP) {

			//check if the file already exists in Server File System!!!!!!!!!!!! before get what the string is
			string filename;

			for (auto i = 2 * sizeof(short); *i != '\0'; ++i)){
				filename += echoBuffer[i]
			}
			ifstream file(filename);
			if (!file.good()) { //file doesn't exist
				WRQ_flag = 1;
			}
			else //send Error File already exist in the wright format:
			
		}
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