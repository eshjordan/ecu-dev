#ifndef MESSAGE_H
#define MESSAGE_H

#include "CRC.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define START_BYTE 0x9DU

// #define ALIGN __attribute__((__packed__, aligned(1U))) // Potentially unsafe/slower on some architectures
#define ALIGN

#define ERR_CRC_FAILED 1
#define ERR_INVALID_LENGTH 2
#define ERR_INVALID_START_BYTE 3

struct Time_t {
    int64_t tv_sec;  /* Seconds.  */
    int64_t tv_nsec; /* Nanoseconds.  */
} ALIGN;

typedef struct Time_t Time_t;

struct Header_t {
    uint32_t length{};
    uint32_t id{};
    Time_t stamp{};
    const uint8_t start_byte = START_BYTE;
} ALIGN;

typedef struct Header_t Header_t;

struct Message_t {
    enum Command_t {
        UNKNOWN         = (uint8_t)(0U),  /* Used for error checking. */
        ECHO            = (uint8_t)(1U),  /* Echo a copy of this message exactly. */
        PING            = (uint8_t)(2U),  /* Get server to send a connection acknowledgement. */
        ACK             = (uint8_t)(3U),  /* Acknowledge a message was received correctly. */
        RESTART         = (uint8_t)(4U),  /* Restart the server. */
        STATUS          = (uint8_t)(5U),  /* Request Server status. */
        SYNC            = (uint8_t)(6U),  /* Synchronize client-server time. */
        FIRMWARE_UPDATE = (uint8_t)(7U),  /* Upgrade the ECU's Firmware, the SDK. */
        PROGRAM_UPDATE  = (uint8_t)(8U),  /* Update the ECU's programmed software, user-defined. */
        VALUE           = (uint8_t)(9U),  /* Send a raw data value. */
        PARAM_GET       = (uint8_t)(10U), /* Retreive a parameter's value. */
        PARAM_SET       = (uint8_t)(11U)  /* Set a parameter's value. */
    } ALIGN;

    Header_t header{};
    char name[64]     = {0};
    uint8_t data[8]   = {0}; // Able to have 8 bytes / 64 bits (long long int) value at max
    Command_t command = UNKNOWN;
    CRC checksum{};
} ALIGN;

typedef struct Message_t Message_t;

inline void calc_checksum(Message_t *msg) { msg->checksum = calc_crc(msg, sizeof(Message_t) - sizeof(msg->checksum)); }

inline int check_message(const void *buffer)
{
    Message_t msg = *(Message_t *)buffer;
    if (msg.header.start_byte != START_BYTE) { return -ERR_INVALID_START_BYTE; }

    if (msg.checksum != calc_crc(buffer, sizeof(Message_t) - sizeof(CRC))) { return -ERR_CRC_FAILED; }
    return 1;
}

inline void print_msg_err(const int err_code)
{
    switch (err_code)
    {
    case ERR_CRC_FAILED: {
        puts("CRC Failed!");
        break;
    }
    case ERR_INVALID_LENGTH: {
        puts("Invalid Length!");
        break;
    }
    case ERR_INVALID_START_BYTE: {
        puts("Invalid Start Byte!");
        break;
    }
    default: {
        puts("Unknown Error!");
        break;
    }
    }
}

inline timespec time_to_timespec(const Time_t *time) { return {(long)time->tv_sec, (long)time->tv_nsec}; }

inline Time_t timespec_to_time(const timespec *time) { return {(int64_t)time->tv_sec, (int64_t)time->tv_nsec}; }

inline Time_t get_time_now(void)
{
    timespec now{};
    timespec_get(&now, TIME_UTC);
    return timespec_to_time(&now);
}

inline void make_header(Header_t *dst, uint32_t id, Time_t stamp, uint32_t length)
{
    dst->id     = id;
    dst->stamp  = stamp;
    dst->length = length;
}

inline void make_message(Message_t *dst, uint32_t id, const char name[64], const void *__restrict data,
                         Message_t::Command_t command)
{
    make_header(&dst->header, id, get_time_now(), sizeof(Message_t));

    strncpy(dst->name, name, 64);

    if (data)
    {
        memcpy(dst->data, data, 8);
    } else
    {
        memset(dst->data, 0, 8);
    }

    dst->command = command;

    calc_checksum(dst);
}

#ifdef RASPI
inline void msg_to_str(char *str, Message_t *msg)
{
    sprintf(str, "Message:\n\tName: %s\n\tID: %u\n\tStamp: %lld.%09lld\n\tLength: %u\n\tCommand: %u\n\tCRC: %u\n",
            msg->name, msg->header.id, msg->header.stamp.tv_sec, msg->header.stamp.tv_nsec, msg->header.length,
            msg->command, msg->checksum);
}
#else
inline void msg_to_str(char *str, Message_t *msg)
{
    sprintf(str, "Message:\n\tName: %s\n\tID: %u\n\tStamp: %ld.%09ld\n\tLength: %u\n\tCommand: %u\n\tCRC: %u\n",
            msg->name, msg->header.id, msg->header.stamp.tv_sec, msg->header.stamp.tv_nsec, msg->header.length,
            msg->command, msg->checksum);
}
#endif

#endif // MESSAGE_H
