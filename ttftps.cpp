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



    // struct sockaddr_in currClntAddr;
    // int fail_cnt = 1;
    // unsigned short curr_data_block = 0x0;
    // unsigned short ACK_block_num = 0x0;
    // string filename;

    // /* START RUNNING THE SERVER */
    // for (;;) { /* Run forever */
    //     /* Set the size of the in-out parameter */
    //     cliAddrLen = sizeof(echoClntAddr);
    //     // Set up fd_set for select()
    //     fd_set readfds;
    //     FD_ZERO(&readfds);
    //     FD_SET(sock, &readfds);
    //     // Set up timeout
    //     struct timeval timer;
    //     timer.tv_sec = (time_t)timeout;
    //     timer.tv_usec = 0;

    //     fail_cnt = 1;
        
    //     /* Select is Blocking until receive message from a client */
    //     while (true) {

    //         //Error "Abandoning file transmission"
    //         if (fail_cnt > timeout) {
    //             //send Error "Abandoning file transmission" in the wright format: TODO
    //             fail_cnt = 1; //start the counting again
    //         }

    //         int readyCheck = select(sockfd + 1, &readfds, NULL, NULL, &timeout);

    //         if (readyCheck == -1) {
    //             perror("TTFTP_ERROR: select() error");
    //             exit(1);
    //         }
    //         else if (readyCheck == 0) { //timval seconts have passed
    //             //send ACK again. TODO
    //             fail_cnt++;
    //         }
    //         // Check if sockfd is ready for reading
    //         if (FD_ISSET(sockfd, &readfds)) break;

    //     }
        

    //     //if Server socket is ready, get the packet:
    //     if ((recvMsgSize = recvfrom(sock, echoBuffer, ECHOMAX, 0,
    //         (struct sockaddr*)&echoClntAddr, &cliAddrLen)) < 0) { 
    //         perror("TTFTP_ERROR: recvfrom() failed"); 
    //         exit(1);
    //     }
        
        
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

        
        
    //     //error : Client's first packet is not WRQ 
    //     if ((echoBuffer[0] != WRQ_OP) && !WRQ_flag) {
    //         //send Error "Uknown user" in the wright format: TODO
    //     }

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


    // /* NOT REACHED */
    // }
}

///////********* END OF TEMPLATE FOR UDP SERVER **************///////////
///////////////////////////////////////////////////////////////////////////////////////////