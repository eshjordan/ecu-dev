#ifndef MESSAGE_H
#define MESSAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "CRC.h"
#include <stdio.h>
#include <time.h>

#define START_BYTE 0x9DU

struct Header_t {
    const unsigned char start_byte = START_BYTE;
    struct timespec stamp {};
    unsigned int id{};
};

struct Message_t {
    struct Header_t header {};
    char name[64]         = {0};
    unsigned char data[8] = {0}; // Able to have 8 bytes / 64 bits (long long int) value at max
    enum {
        UNKNOWN,         /* Used for error checking. */
        PING,            /* Check client-server connection status. */
        VALUE,           /* Send a raw data value. */
        FIRMWARE_UPDATE, /* Upgrade the ECU's Firmware, the SDK. */
        PROGRAM_UPDATE,  /* Update the ECU's programmed software, user-defined. */
        PARAM_GET,       /* Retreive a parameter's value. */
        PARAM_SET        /* Set a parameter's value. */
    } command = UNKNOWN;

    CRC checksum{};
};

typedef struct Message_t Message_t;

inline void calc_checksum(Message_t *msg) { msg->checksum = calc_crc(msg, sizeof(Message_t) - sizeof(msg->checksum)); }

inline int looks_like_message(const unsigned char buffer[], const long int bytes_received)
{
    if (bytes_received != sizeof(Message_t)) { return 0; }
    if (buffer[0] != START_BYTE) { return 0; }
    if (*(CRC *)&buffer[bytes_received - sizeof(CRC)] != calc_crc(buffer, bytes_received - sizeof(CRC)))
    {
        puts("CRC Match Error for received message!");
        return 0;
    }
    return 1;
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // MESSAGE_H
