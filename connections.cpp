#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <limits.h>
#include <iostream>
#include <string.h>
#include <iostream>
#include "connections.h"

#define WRQ_OP 0x02
#define ACK_OP 0x04
#define DATA_OP 0x03

void connection::init_connection(unsigned short port_num, unsigned short timeout, unsigned short max_num_of_resends)
{
    struct sockaddr_in server_address; /* Local address */
	int bind_rc = 0;

	// Set connection variables
	this->timeout = timeout;
	this->max_resends = max_num_of_resends;
	this->current_resends = 0;

    /* Create socket for sending/receiving datagrams */
    this->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (this->socket_fd < 0) {
        perror("TTFTP_ERROR: socket() failed");
        exit(1);
    }

    /* Construct local address structure */
    /* Zero out structure */
    memset(&server_address, 0, sizeof(server_address));
    /* Internet address family */
    server_address.sin_family = AF_INET;
    /* Any incoming interface */
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    /* Local port */
    server_address.sin_port = htons(port_num);
    /* Bind to the local address */
	bind_rc = bind(this->socket_fd, (struct sockaddr*)&server_address, sizeof(server_address));
    if (0 != bind_rc) {
        perror("TTFTP_ERROR: bind() failed");
        exit(1);
    }
}

void connection::handle_packet()
{

}
