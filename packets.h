#ifndef _PACKETS_H
#define _PACKETS_H

#include <string>

/* max packet size our server supports */
#define MAX_PACKET_SIZE 516
#define ERROR_PACKET_HEADER_SIZE 4
#define ACK_PACKET_SIZE 4
#define DATA_PACKET_HEADER_SIZE 4

using namespace std;

enum packet_opcode {
    WRQ_OP = 0x02,
    DATA_OP = 0x03,
    ACK_OP = 0x04,
    ERROR_OP = 0x05,
};

enum error_packet_codes {
    ERROR_CODE_FILE_EXISTS = 6,
    ERROR_CODE_UNKNOWN_USER = 7,
    ERROR_CODE_UNEXPECTED_PACKET = 4,
    ERROR_CODE_BAD_BLOCK = 0,
    ERROR_CODE_MAX_RESEND = 0,
};

#define ERROR_MESSAGE_FILE_EXISTS "File already exists"
#define ERROR_MESSAGE_UNEXPECTED_PACKET "Unexpected packet"
#define ERROR_MESSAGE_MAX_RESEND "Abandoning file transmission"
#define ERROR_MESSAGE_BAD_BLOCK "Bad block number"
#define ERROR_MESSAGE_UNKNOWN_USER "Unknown user"

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

struct ACK_packet {
    unsigned short opcode;
    unsigned short block_number;
} __attribute__((packed));

struct ERROR_packet {
    unsigned short opcode;
    unsigned short error_code;
    char error_string[MAX_PACKET_SIZE - sizeof(opcode) - sizeof(error_code)];
} __attribute__((packed));


#endif // _PACKETS_H