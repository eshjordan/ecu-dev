#ifndef MESSAGE_H
#define MESSAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "CRC.h"

struct Message_t {
    const unsigned char start_byte = 0x9D;
    unsigned char id{};
    unsigned char name[64] = {0};
    unsigned char data[8]  = {0}; // Able to have 8 bytes / 64 bits (long long int) value at max
    enum { UNKNOWN, PING, VALUE, PARAM_GET, PARAM_SET } command = UNKNOWN;

    CRC checksum{};
};

typedef struct Message_t Message_t;

inline void calc_checksum(Message_t *msg) { msg->checksum = calc_crc(msg, sizeof(Message_t) - sizeof(msg->checksum)); }

#ifdef __cplusplus
} // extern "C"
#endif

#endif // MESSAGE_H
