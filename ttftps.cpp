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
#include <string.h>

#include "packets.h"
#include "connections.h"

using namespace std;

enum prog_args {
    PROG_ARGS_PROGNAME = 0,
    PROG_ARGS_PORT,
    PROG_ARGS_TIMEOUT,
    PROG_ARGS_MAX_RESENDS,
    PROG_ARGS_LEN
};


int main(int argc, char* argv[]) {

    // Parse arguments
    if (argc < PROG_ARGS_LEN) {
        cerr << "TTFTP_ERROR: illegal arguments" << endl;
        exit(1);
    }

    unsigned short port_num = atoi(argv[PROG_ARGS_PORT]);
    unsigned short timeout = atoi(argv[PROG_ARGS_TIMEOUT]);
    unsigned short max_num_of_resends = atoi(argv[PROG_ARGS_MAX_RESENDS]);

    // Validate arguments are valid
    if ((port_num > USHRT_MAX || port_num <= 0) ||
        (timeout > USHRT_MAX || timeout <= 0) ||
        (max_num_of_resends > USHRT_MAX || max_num_of_resends <= 0)) {
        cerr << "TTFTP_ERROR: illegal arguments" << endl;
        exit(1);
    }

    // Open listening socket
    connection server_connection;
    server_connection.init_connection(port_num, timeout, max_num_of_resends);

    // Serve clients forever
    for (;;) {
        // Read and process next packet
        server_connection.handle_packet();
    }
    return EXIT_SUCCESS; // This is never reached.
}
