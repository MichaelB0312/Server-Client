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
    this->max_wait_timeout = timeout;
    this->max_resends = max_num_of_resends;
    this->current_resends = 0;
    this->has_ongoing_client = false;

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
    // Read a packet from the connection packet
    bool did_get_packet = this->get_next_packet();

    // Check if packet received
    if (!did_get_packet) {
        // no packet only happens if timeout occurred with ongoing client
        this->handle_timeout();
        return;
    }

    // Reject packets from non-ongoing clients
    if ((this->has_ongoing_client) && 
        (0 != memcmp(&this->current_client_address, 
                     &this->ongoing_client_address, sizeof(this->current_client_address)))){
        this->send_unexpected_packet();
        return;
    }

    // Get opcode of packet
    unsigned short packet_opcode = ntohs(((struct general_packet*)(this->packet_buffer))->opcode);
    cout << "DEBUG: got opcode " << packet_opcode << endl; // TODO: Remove me

    // Handle packet based on server state
    switch (packet_opcode)
    {
    case WRQ_OP:
        this->handle_write_packet();
        break;
    case DATA_OP:
        this->handle_data_packet();
        break;
    default:
        // We assume that the client always sends valid WRQ or DATA packets, so we should never reach this.
        // If we do reach this, just ignore the current packet quietly and return to wait for the next packet.
        return;
    }
}

bool connection::get_next_packet()
{
    struct timeval timer_with_client;
    timer_with_client.tv_sec = (time_t)this->max_wait_timeout;
    timer_with_client.tv_usec = 0;
    struct timeval * select_timer;
    // choose if we need to select forever or only for 3 seconds, based on server state
    if (this->has_ongoing_client) {
        select_timer = &timer_with_client;
    } else {
        select_timer = NULL;
    }

    // select on sever socket
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(this->socket_fd, &readfds);
    int select_number_of_fds = this->socket_fd + 1;
    
    int number_of_ready_fds = select(select_number_of_fds, &readfds, NULL, NULL, select_timer);
    if (-1 == number_of_ready_fds) {
        perror("TTFTP_ERROR: select() error");
        exit(1);
    }
    if ((0 == number_of_ready_fds) || (!FD_ISSET(this->socket_fd, &readfds))) {
        // socket not ready, return without reading
        return false;
    }

    // read packet from socket
    socklen_t cliAddrLen = sizeof(this->current_client_address);
    this->current_packet_length = recvfrom(this->socket_fd, this->packet_buffer,
        MAX_PACKET_SIZE, 0, (struct sockaddr*)&this->current_client_address, &cliAddrLen);
    if (-1 == this->current_packet_length) { 
        perror("TTFTP_ERROR: recvfrom() failed"); 
        exit(1);
    }

    cout << "DEBUG: got a packet from " << ntohs(this->current_client_address.sin_port) << endl; // TODO: Remove me
    return true;
}

void connection::handle_timeout()
{
    // TODO: Fill this!
    cout << "DEBUG: got timeout" << endl; // TODO: Remove me
}

void connection::handle_write_packet()
{
    cout << "DEBUG: got write request" << endl; // TODO: Remove me

    // check if we already have an ongoing connection
    if (this->has_ongoing_client) {
        // Should't get write request from client if we already have a connection.
        // Kill the connection
        this->cancel_current_connection();
        return;
    }

    // TODO: Handle client starting to write (open file and handle ). Change following code to do what's needed.
          
    //     // check if it's WRQ packet:
    //     if (echoBuffer[0] == WRQ_OP && !WRQ_flag) { //first WRQ packet

    //         currClntAddr.sin_addr.s_addr = echoClntAddr.sin_addr.s_addr;
    //         currClntAddr.sin_port = ntohs(echoClntAddr.sin_port);
            

    //         for (auto i = 2 * sizeof(short); *i != '\0'; ++i)){
    //             filename += echoBuffer[i]
    //         }
    //         ifstream file(filename);
    //         if (!file.good()) { //file doesn't exist, new file has arrived!
    //             WRQ_flag = 1;
    //             ofstream file(fileName, std::ios::app);); //add file to current cd
    //         }
    //         else //send Error "File already exist" in the wright format: TODO
            
    //     }

    //     //send Error "Unexpected packet" in the wright format: TODO
    //     if (!SessionEnd_flag && ((echoBuffer[0] == WRQ_OP) ||
    //                             (currClntAddr.sin_port != ntohs(echoClntAddr.sin_port)
    //                             || (currClntAddr.sin_addr.s_addr != echoClntAddr.sin_addr.s_addr)) {

    //         //send Error "Unexpected packet" in the wright format: TODO
    //     }

}

void connection::cancel_current_connection()
{
    // notify client we are killing the connection
    this->send_unexpected_packet();
    // drop the connection
    this->has_ongoing_client = false;
    // reset resend retries
    this->current_resends = 0;

    // TODO: Add removing the file we started to write for this client
}

void connection::handle_data_packet()
{
    // TODO: Fill this!
    cout << "DEBUG: got data request" << endl; // TODO: Remove me

    // TODO: change following code to do what's needed

//     /* Response to DATA packet */
//     if (echoBuffer[0] == DATA_OP) {
//         // parse DATA packet block number
//         rec_block_num = (static_cast<unsigned char>(echoBuffer[2]) << 8) | static_cast<unsigned char>(echoBuffer[3]);
//         //error: "Bad block number"
//         if (rec_block_num != (curr_data_block + 1)) {
//             //send Error "Bad block number" in the wright format: TODO
//         }
//         else //valid data block, send ACK
//         {
//             unsigned int ACK_Samp_Content = 0x0;
//             ACK_Samp_Content += ACK_OP;
//             ACK_Samp_Content = ACK_Samp_Content << (sizeof(unsigned short) * 8);
//             ACK_Samp_Content += ACK_block_num;
            
//             if (sendto(sock, &ACK_Samp_Content, sizeof(unsigned int), 0, (struct sockaddr*)&currClntAddr,
//                 sizeof(currClntAddr)) {
                
//                 perror("TTFTP_ERROR: sendto() failed");
//                 exit(1);
//             }
//             curr_data_block++;
//             ACK_block_num++;

//             // TODO: write data content to filename. I dont know what is the issue with modulo 512???
//             if (recvMsgSize < (ECHOMAX - HEADER_SIZE)) { //end of session
//                 SessionEnd_flag = 1;
//                 memset(&currClntAddr, 0, sizeof(currClntAddr)); //prepare to a new client
//                 WRQ_flag = 0;
//                 ACK_block_num = 0x0;
//                 curr_data_block = 0x0;
//             }

//             //insert data packet content:
//             char* content = echoBuffer + 4; //do not include header
//             file << content;

//         }
//     }
}


void connection::send_unexpected_packet()
{
    // TODO: Fill this!
    cout << "DEBUG: sending unexpected packet error packet" << endl; // TODO: Remove me

    // build the unexpected packet error packet

    // send packet to current client
}
