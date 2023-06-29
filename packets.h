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
    ERROR_OP = 0x05,
};

struct general_packet {
    unsigned short opcode;
    char undefined_data[MAX_PACKET_SIZE - sizeof(opcode)];
} __attribute__((packed));

struct WRQ_packet {
    unsigned short opcode;
    char strings[MAX_PACKET_SIZE - sizeof(opcode)];
} __attribute__((packed));

struct DATA_packet {
    unsigned short opcode;
    unsigned short block_number;
    char file_data[MAX_PACKET_SIZE - sizeof(opcode) - sizeof(block_number)];
} __attribute__((packed));

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