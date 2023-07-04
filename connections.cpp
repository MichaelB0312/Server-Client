#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <limits.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <time.h>
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
    struct timeval * select_timer;
    struct timeval select_timeout_from_last_valid_packet;


    // choose if we need to select forever or only for remainder of timeout, based on server state
    if (this->has_ongoing_client) {
        // calculate time to select for
        // get current time
        struct timespec current_time;
        if (-1 == clock_gettime(CLOCK_MONOTONIC, &current_time)) {
            perror("TTFTP_ERROR: clock_gettime() failed"); 
            exit(1);
        }
        // subtract current time from last packet time plus timeout
        select_timeout_from_last_valid_packet.tv_sec = (time_t)this->max_wait_timeout + this->last_valid_packet_time.tv_sec - current_time.tv_sec;
        select_timeout_from_last_valid_packet.tv_usec = this->last_valid_packet_time.tv_nsec - current_time.tv_nsec;
        if (0 > select_timeout_from_last_valid_packet.tv_usec) {
            // convert one second to usec to deal with negative usec value
            select_timeout_from_last_valid_packet.tv_usec += 1000000;
            select_timeout_from_last_valid_packet.tv_sec -= 1;
        }
        if (select_timeout_from_last_valid_packet.tv_sec < 0) {
            // timeout already passed, and we didn't call select yet
            // (meaning the timeout passed between our last call to select and now)
            return false;
        }
        select_timer = &select_timeout_from_last_valid_packet;
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

    // fill this time as the last valid data packet
    // (as if this packet is invalid, we will reset the connection anyway, as this is the correct client)
    if (-1 == clock_gettime(CLOCK_MONOTONIC, &this->last_valid_packet_time)) {
        perror("TTFTP_ERROR: clock_gettime() failed"); 
        exit(1);
    }

    // check if we already have an ongoing connection
    if (this->has_ongoing_client) {
        // Should't get write request from client if we already have a connection.
        // Kill the connection
        this->cancel_current_connection();
        return;
    }

    // save current client as ongoing client
    memcpy(&this->ongoing_client_address, &this->current_client_address, sizeof(this->current_client_address));
    this->has_ongoing_client = true;
    this->current_resends = 0;

    // get filename
    strncpy(this->filename, ((struct WRQ_packet*)(&this->packet_buffer))->strings, MAX_PACKET_SIZE-1);
    this->filename[MAX_PACKET_SIZE-1] = '\0';
    cout << "DEBUG: requested file to create: " << this->filename << endl; // TODO: Remove me

    // check if file already exists
    ifstream intput_file (this->filename);
    if (intput_file.good()) {
        // file exists. We return error and close connection.
        this->close_connection_file_exists();
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

void connection::close_connection_file_exists()
{
    // drop the connection
    this->has_ongoing_client = false;

    // notify client file exists
    ERROR_packet file_exists_packet = {htons(ERROR_OP), htons(ERROR_CODE_FILE_EXISTS), ERROR_MESSAGE_FILE_EXISTS};
    if (-1 == sendto(this->socket_fd, &file_exists_packet, sizeof(ERROR_MESSAGE_FILE_EXISTS) + ERROR_PACKET_HEADER_SIZE, 0, (struct sockaddr*)&this->current_client_address, sizeof(this->current_client_address))) {
        perror("TTFTP_ERROR: sendto() failed");
        exit(1);
    }
}

void connection::cancel_current_connection()
{
    // drop the connection
    this->has_ongoing_client = false;
    
    // notify client we are killing the connection
    this->send_unexpected_packet();

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
