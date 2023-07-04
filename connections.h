#ifndef _CONNECTIONS_H
#define _CONNECTIONS_H

#include "packets.h"

typedef struct connection {
    int socket_fd;
    unsigned short max_wait_timeout;
    unsigned short max_resends;
    unsigned short current_resends;
    struct sockaddr_in current_client_address;
    bool has_ongoing_client;
    struct sockaddr_in ongoing_client_address;
    char filename[MAX_PACKET_SIZE];
    char packet_buffer[MAX_PACKET_SIZE];
    int current_packet_length;
    struct timespec last_valid_packet_time;
    ofstream current_file;
    unsigned short current_block;

    // external functions
    void init_connection(unsigned short port_num, unsigned short timeout, unsigned short max_num_of_resends);
    bool get_next_packet();

    // internal functions
    void handle_packet();
    void handle_timeout();
    void send_unexpected_packet();
    void handle_write_packet();
    void handle_data_packet();
    void cancel_current_connection();
    void close_connection_file_exists();
}connection;


#endif // _CONNECTIONS_H