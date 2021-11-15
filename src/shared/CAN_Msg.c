#include "CAN_Msg.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

CRC can_msg_calc_checksum(CAN_Msg_t *msg) { return msg->checksum = calc_crc(msg, offsetof(CAN_Msg_t, checksum)); }

ecu_err_t can_msg_check(const CAN_Msg_t *const msg)
{
    if (msg->header.start_byte != ECU_HEADER_START_BYTE) { return -ERR_INVALID_START_BYTE; }

    if (msg->checksum != calc_crc(msg, offsetof(CAN_Msg_t, checksum))) { return -ERR_CRC_FAILED; }
    return 1;
}

/**
 * @brief Use at least 512 bytes for the message buffer.
 *
 * @param str
 * @param msg
 */
void can_msg_to_str(char *str, const CAN_Msg_t *msg)
{
    static const char fmt_str[] = "CAN_Msg:\n"
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
