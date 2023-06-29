#ifndef _PACKETS_H
#define _PACKETS_H

#include <string>

/* max packet size our server supports */
#define MAX_PACKET_SIZE 516

using namespace std;

enum packet_opcode {
    WRQ_OP = 0x02,
    DATA_OP = 0x03,
    ACK_OP = 0x04,
};

#define PACKET_HEADER_SIZE 4

// typedef struct {
// 	unsigned short opcode;
// 	string filename;


// } WRQpacket

// /*! msg_type: Recognify packet type and raise an appropiate flag
//   \param[in] 
//   \param[in]
// */
// int msg_type()


// /* Recognize DATA packet*/





#endif // _PACKETS_H