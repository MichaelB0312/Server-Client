#ifndef _CONNECTIONS_H
#define _CONNECTIONS_H

typedef struct connection {
    int socket_fd;
    unsigned short timeout;
    unsigned short max_resends;
    unsigned short current_resends;
    struct sockaddr_in current_client_address;

    void init_connection(unsigned short port_num, unsigned short timeout, unsigned short max_num_of_resends);
    void handle_packet();
}connection;


#endif // _CONNECTIONS_H