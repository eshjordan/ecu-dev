#include "Message.h"
#include <stdio.h>
#include <string.h>

CRC calc_msg_checksum(Message_t *msg) { return msg->checksum = calc_crc(msg, sizeof(Message_t) - sizeof(CRC)); }

int check_msg(const Message_t *const msg)
{
    if (msg->header.start_byte != ECU_MSG_START_BYTE) { return -ERR_INVALID_START_BYTE; }

    if (msg->checksum != calc_crc(msg, sizeof(Message_t) - sizeof(CRC))) { return -ERR_CRC_FAILED; }
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

void make_message(Message_t *dst, uint32_t id, const char name[64], const void *data, Command_t command)
{
    make_header(&dst->header, id, get_time_now(), sizeof(Message_t));

    if (name) { strncpy(dst->name, name, 64); }

    if (data) { memcpy(dst->data, data, 8); }

    dst->command = command;

    calc_msg_checksum(dst);
}

void msg_to_str(char *str, const Message_t *const msg)
{

#if UINTPTR_MAX == 0xffffffff
    /* 32-bit */
    static const char fmt_str[] = "Message:\n"
                                  "    Header:\n"
                                  "        length: %u\n"
                                  "        id: %u\n"
                                  "        stamp:\n"
                                  "            tv_sec: %lld\n"
                                  "            tv_nsec: %lld\n"
                                  "        start_byte: %hhu\n"
                                  "    Name: %s\n"
                                  "    Data: %s\n"
                                  "    Command: %u\n"
                                  "    CRC: %u\n"
                                  "\n";

#elif UINTPTR_MAX == 0xffffffffffffffff
    /* 64-bit */
    static const char fmt_str[] = "Message:\n"
                                  "    Header:\n"
                                  "        length: %u\n"
                                  "        id: %u\n"
                                  "        stamp:\n"
                                  "            tv_sec: %ld\n"
                                  "            tv_nsec: %ld\n"
                                  "        start_byte: %hhu\n"
                                  "    Name: %s\n"
                                  "    Data: %s\n"
                                  "    Command: %u\n"
                                  "    CRC: %u\n"
                                  "\n";

#endif

    sprintf(str, fmt_str, msg->header.length, msg->header.id, msg->header.stamp.tv_sec, msg->header.stamp.tv_nsec,
            msg->header.start_byte, msg->name, msg->data, msg->command, msg->checksum);
}
