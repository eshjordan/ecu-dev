#include "Message.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

CRC calc_msg_checksum(Message_t *msg)
{
    return msg->checksum = calc_crc(msg, offsetof(Message_t, checksum));
}

int check_msg(const Message_t *const msg)
{
    if (msg->header.start_byte != ECU_MSG_START_BYTE) { return -ERR_INVALID_START_BYTE; }

    if (msg->checksum != calc_crc(msg, offsetof(Message_t, checksum))) { return -ERR_CRC_FAILED; }
    return 1;
}

void msg_err_to_str(char *str, const int err_code)
{
    switch (err_code)
    {
    case -ERR_CRC_FAILED: {
        sprintf(str, "CRC Failed!");
        return;
    }
    case -ERR_INVALID_LENGTH: {
        sprintf(str, "Invalid Length!");
        return;
    }
    case -ERR_INVALID_START_BYTE: {
        sprintf(str, "Invalid Start Byte!");
        return;
    }
    default: {
        sprintf(str, "Unknown Error!");
        return;
    }
    }
}

struct Message_t make_message(uint32_t id, const char name[64], const void *data, Command_t command)
{
    Message_t dst = {.header = make_header(id, sizeof(Message_t)), .command = command};

    if (name) { strncpy(dst.name, name, 64); }

    if (data) { memcpy(dst.data, data, 8); }

    calc_msg_checksum(&dst);

    return dst;
}

void msg_to_str(char *str, const Message_t *const msg)
{
    static const char fmt_str[] = "Message:\n"
                                  "    Header:\n"
                                  "        start_byte: %hhu\n"
                                  "        length: %u\n"
                                  "        id: %u\n"
                                  "        stamp:\n"
#if UINTPTR_MAX == 0xffffffff
                                  /* 32-bit */
                                  "            tv_sec: %lld\n"
                                  "            tv_nsec: %lld\n"
#elif UINTPTR_MAX == 0xffffffffffffffff
                                  /* 64-bit */
                                  "            tv_sec: %ld\n"
                                  "            tv_nsec: %ld\n"
#endif
                                  "    Name: %s\n"
                                  "    Data: %s\n"
                                  "    Command: %u\n"
                                  "    CRC: %u\n"
                                  "\n";

    sprintf(str, fmt_str, msg->header.start_byte, msg->header.length, msg->header.id, msg->header.stamp.tv_sec,
            msg->header.stamp.tv_nsec, msg->name, msg->data, msg->command, msg->checksum);
}
