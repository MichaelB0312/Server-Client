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

    // fill this time as the last valid packet time and reset resend counter
    // (as if this packet is invalid, we will reset the connection anyway, as this is the correct client)
    this->current_resends = 0;
    if (-1 == clock_gettime(CLOCK_MONOTONIC, &this->last_valid_packet_time)) {
        perror("TTFTP_ERROR: clock_gettime() failed"); 
        exit(1);
    }

    // Get opcode of packet
    unsigned short packet_opcode = ntohs(((struct general_packet*)(this->packet_buffer))->opcode);

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
        select_timeout_from_last_valid_packet.tv_usec = (this->last_valid_packet_time.tv_nsec - current_time.tv_nsec) / 1000;
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

    return true;
}


void connection::handle_timeout()
{

    // Check if max resends reached
    if (this->current_resends >= this->max_resends) {
        // drop the connection
        this->has_ongoing_client = false;
        
        // notify client we are killing the connection
        ERROR_packet max_resends_packet = {htons(ERROR_OP), htons(ERROR_CODE_MAX_RESEND), ERROR_MESSAGE_MAX_RESEND};
        if (-1 == sendto(this->socket_fd, &max_resends_packet, sizeof(ERROR_MESSAGE_MAX_RESEND) + ERROR_PACKET_HEADER_SIZE, 0, (struct sockaddr*)&this->ongoing_client_address, sizeof(this->ongoing_client_address))) {
            perror("TTFTP_ERROR: sendto() failed");
            exit(1);
        }

        // close and remove current file
        this->current_file.close();
        if (!this->current_file.good()) {
            perror("TTFTP_ERROR: close() failed");
            exit(1);
        }
        if (0 != remove(this->filename)) {
            perror("TTFTP_ERROR: remove() failed");
            exit(1);
        }
    } else {
        // add to resend count
        this->current_resends += 1;

        // reset timeout
        if (-1 == clock_gettime(CLOCK_MONOTONIC, &this->last_valid_packet_time)) {
            perror("TTFTP_ERROR: clock_gettime() failed"); 
            exit(1);
        }

        // resend ack packet
        ACK_packet ack_packet = {htons(ACK_OP), htons(this->current_block)};
        if (-1 == sendto(this->socket_fd, &ack_packet, ACK_PACKET_SIZE, 0, (struct sockaddr*)&this->ongoing_client_address, sizeof(this->ongoing_client_address))) {
            perror("TTFTP_ERROR: sendto() failed");
            exit(1);
        }
    }
}

void connection::handle_write_packet()
{

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
    this->current_block = 0;

    // get filename
    strncpy(this->filename, ((struct WRQ_packet*)(this->packet_buffer))->strings, MAX_PACKET_SIZE-1);
    this->filename[MAX_PACKET_SIZE-1] = '\0';

    // check if file already exists
    ifstream intput_file (this->filename);
    if (intput_file.good()) {
        // file exists. We return error and close connection.
        intput_file.close();
        this->close_connection_file_exists();
        return;
    }

    // open file for writing
    this->current_file.open(this->filename, ofstream::out | ofstream::binary | ofstream::ate);
    if (!this->current_file.good()) {
        // error opening the file
        perror("TTFTP_ERROR: open() failed");
        exit(1);
    }

    // send ack to client
    ACK_packet ack_packet = {htons(ACK_OP), htons(this->current_block)};
    if (-1 == sendto(this->socket_fd, &ack_packet, ACK_PACKET_SIZE, 0, (struct sockaddr*)&this->current_client_address, sizeof(this->current_client_address))) {
        perror("TTFTP_ERROR: sendto() failed");
        exit(1);
    }
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

    // close and remove current file
    this->current_file.close();
    if (!this->current_file.good()) {
        perror("TTFTP_ERROR: close() failed");
        exit(1);
    }
    if (0 != remove(this->filename)) {
        perror("TTFTP_ERROR: remove() failed");
        exit(1);
    }
}

void connection::handle_data_packet()
{

    // check we already started writing
    if (!this->has_ongoing_client) {
        // send unexpected packet to current client
        ERROR_packet unknown_user_packet = {htons(ERROR_OP), htons(ERROR_CODE_UNKNOWN_USER), ERROR_MESSAGE_UNKNOWN_USER};
        if (-1 == sendto(this->socket_fd, &unknown_user_packet, sizeof(ERROR_MESSAGE_UNKNOWN_USER) + ERROR_PACKET_HEADER_SIZE, 0, (struct sockaddr*)&this->current_client_address, sizeof(this->current_client_address))) {
            perror("TTFTP_ERROR: sendto() failed");
            exit(1);
        }
        return;
    }

    // Check block number
    unsigned short next_block = ntohs(((struct DATA_packet*)(this->packet_buffer))->block_number);
    if (1 + this->current_block != next_block) {
        // drop the connection
        this->has_ongoing_client = false;
        
        // notify client we are killing the connection
        ERROR_packet bad_block_packet = {htons(ERROR_OP), htons(ERROR_CODE_BAD_BLOCK), ERROR_MESSAGE_BAD_BLOCK};
        if (-1 == sendto(this->socket_fd, &bad_block_packet, sizeof(ERROR_MESSAGE_BAD_BLOCK) + ERROR_PACKET_HEADER_SIZE, 0, (struct sockaddr*)&this->ongoing_client_address, sizeof(this->ongoing_client_address))) {
            perror("TTFTP_ERROR: sendto() failed");
            exit(1);
        }

        // close and remove current file
        this->current_file.close();
        if (!this->current_file.good()) {
            perror("TTFTP_ERROR: close() failed");
            exit(1);
        }
        if (0 != remove(this->filename)) {
            perror("TTFTP_ERROR: remove() failed");
            exit(1);
        }

        // return to stop processing this data block
        return;
    }

    // move to next data block
    this->current_block = next_block;

    // send ack to client
    ACK_packet ack_packet = {htons(ACK_OP), htons(this->current_block)};
    if (-1 == sendto(this->socket_fd, &ack_packet, ACK_PACKET_SIZE, 0, (struct sockaddr*)&this->current_client_address, sizeof(this->current_client_address))) {
        perror("TTFTP_ERROR: sendto() failed");
        exit(1);
    }

    // write data to file
    unsigned short data_length = this->current_packet_length - DATA_PACKET_HEADER_SIZE;
    if (0 != data_length) {
        this->current_file.write(((struct DATA_packet*)(this->packet_buffer))->file_data, data_length);
        if (!this->current_file.good()) {
            perror("TTFTP_ERROR: write() failed");
            exit(1);
        }
        this->current_file.flush();
    }

    // close file and connection if this is the last data part
    if (MAX_PACKET_SIZE - DATA_PACKET_HEADER_SIZE != data_length) {
        // Close file
        this->current_file.close();
        if (!this->current_file.good()) {
            perror("TTFTP_ERROR: close() failed");
            exit(1);
        }

        // close connection
        this->has_ongoing_client = false;
    }
}

void connection::send_unexpected_packet()
{

    // send unexpected packet to current client
    ERROR_packet file_exists_packet = {htons(ERROR_OP), htons(ERROR_CODE_UNEXPECTED_PACKET), ERROR_MESSAGE_UNEXPECTED_PACKET};
    if (-1 == sendto(this->socket_fd, &file_exists_packet, sizeof(ERROR_MESSAGE_UNEXPECTED_PACKET) + ERROR_PACKET_HEADER_SIZE, 0, (struct sockaddr*)&this->current_client_address, sizeof(this->current_client_address))) {
        perror("TTFTP_ERROR: sendto() failed");
        exit(1);
    }
}
