#include "CANMsg.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

CRC calc_can_checksum(CANMsg_t *msg)
{
    return msg->checksum = calc_crc(msg, offsetof(CANMsg_t, checksum));
}

int check_can(const CANMsg_t *const msg)
{
    if (msg->header.start_byte != ECU_MSG_START_BYTE) { return -ERR_INVALID_START_BYTE; }

    if (msg->checksum != calc_crc(msg, offsetof(CANMsg_t, checksum))) { return -ERR_CRC_FAILED; }
    return 1;
}

void can_err_to_str(char *str, const int err_code)
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

/**
 * @brief Use at least 512 bytes for the message buffer.
 *
 * @param str
 * @param msg
 */
void CANMsg_to_str(char *str, const CANMsg_t *msg)
{
    static const char fmt_str[] = "CANMsg:\n"
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
                                  "    flags: %u\n"
                                  "    identifier: %u\n"
                                  "    data_length_code: %hu\n"
                                  "    data:\n"
                                  "        data[0]: %hu\n"
                                  "        data[1]: %hu\n"
                                  "        data[2]: %hu\n"
                                  "        data[3]: %hu\n"
                                  "        data[4]: %hu\n"
                                  "        data[5]: %hu\n"
                                  "        data[6]: %hu\n"
                                  "        data[7]: %hu\n"
                                  "    checksum: %u\n"
                                  "\n";

    sprintf(str, fmt_str, msg->header.start_byte, msg->header.length, msg->header.id, msg->header.stamp.tv_sec,
            msg->header.stamp.tv_nsec, msg->flags, msg->identifier, msg->data_length_code, msg->data[0], msg->data[1],
            msg->data[2], msg->data[3], msg->data[4], msg->data[5], msg->data[6], msg->data[7], msg->checksum);
}
