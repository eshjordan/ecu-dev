#ifndef MESSAGE_H
#define MESSAGE_H

#include <cstddef>
#include <cstdint>
#include <cstring>
#ifdef __cplusplus
extern "C" {
#endif

#include "CRC.h"
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define START_BYTE 0x9DU
#define ALIGNMENT_SIZE 4U

struct Time_t {
    int64_t tv_sec;  /* Seconds.  */
    int64_t tv_nsec; /* Nanoseconds.  */
} __attribute__((aligned(ALIGNMENT_SIZE)));

typedef struct Time_t Time_t;

struct Header_t {
    const uint8_t start_byte = START_BYTE;
    uint32_t id{};
    Time_t stamp{};
} __attribute__((aligned(ALIGNMENT_SIZE)));

typedef struct Header_t Header_t;

struct Message_t {
    enum Command_t {
        UNKNOWN,         /* Used for error checking. */
        PING,            /* Check client-server connection status. */
        SYNC,            /* Synchronize client-server time. */
        VALUE,           /* Send a raw data value. */
        FIRMWARE_UPDATE, /* Upgrade the ECU's Firmware, the SDK. */
        PROGRAM_UPDATE,  /* Update the ECU's programmed software, user-defined. */
        PARAM_GET,       /* Retreive a parameter's value. */
        PARAM_SET        /* Set a parameter's value. */
    };

    Header_t header{};
    char name[64]     = {0};
    uint8_t data[8]   = {0}; // Able to have 8 bytes / 64 bits (long long int) value at max
    Command_t command = UNKNOWN;
    CRC checksum{};
} __attribute__((aligned(ALIGNMENT_SIZE)));

typedef struct Message_t Message_t;

inline void calc_checksum(Message_t *msg) { msg->checksum = calc_crc(msg, sizeof(Message_t) - sizeof(msg->checksum)); }

inline int looks_like_message(const void *buffer, const uint64_t bytes_received)
{
    if (bytes_received != sizeof(Message_t))
    {
        puts("Received bytes did not match Message_t size!");
        return 0;
    }

    Message_t msg = *(Message_t *)buffer;
    if (msg.header.start_byte != START_BYTE)
    {
        puts("Message START_BYTE did not match!!");
        return 0;
    }

    if (msg.checksum != calc_crc(buffer, bytes_received - sizeof(CRC)))
    {
        puts("CRC Match Error!");
        return 0;
    }
    return 1;
}

inline timespec time_to_timespec(const Time_t *time) { return {(long)time->tv_sec, (long)time->tv_nsec}; }

inline Time_t timespec_to_time(const timespec *time) { return {(int64_t)time->tv_sec, (int64_t)time->tv_nsec}; }

inline Time_t get_time_now(void)
{
    timespec now{};
    timespec_get(&now, TIME_UTC);
    return timespec_to_time(&now);
}

inline void make_header(Header_t *dst, uint32_t id, Time_t stamp)
{
    dst->id    = id;
    dst->stamp = stamp;
}

inline void make_message(Message_t *dst, uint32_t id, Time_t stamp, const char name[64], const void *__restrict data,
                         Message_t::Command_t command)
{
    dst->header.id    = id;
    dst->header.stamp = stamp;
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
    sprintf(str, "Message:\n\tName: %s\n\tID: %u\n\tStamp: %lld.%09lld\n\tCommand: %u\n\tCRC: %u\n", msg->name,
            msg->header.id, msg->header.stamp.tv_sec, msg->header.stamp.tv_nsec, msg->command, msg->checksum);
}
#else
inline void msg_to_str(char *str, Message_t *msg)
{
    sprintf(str, "Message:\n\tName: %s\n\tID: %u\n\tStamp: %ld.%09ld\n\tCommand: %u\n\tCRC: %u\n", msg->name,
            msg->header.id, msg->header.stamp.tv_sec, msg->header.stamp.tv_nsec, msg->command, msg->checksum);
}
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif // MESSAGE_H
