#include "ECU_Msg.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

struct ECU_Msg_t ecu_msg_make(uint32_t id, const char name[64], const void *data, ECU_Command_t command)
{
    ECU_Msg_t dst = {.header = header_make(id, sizeof(ECU_Msg_t)), .command = command};

    if (name) { strncpy(dst.name, name, 64); }

    if (data) { memcpy(dst.data, data, 8); }

    ecu_msg_calc_checksum(&dst);

    return dst;
}

CRC ecu_msg_calc_checksum(ECU_Msg_t *msg) { return msg->checksum = calc_crc(msg, offsetof(ECU_Msg_t, checksum)); }

ecu_err_t ecu_msg_check(const ECU_Msg_t *const msg)
{
    if (msg->header.start_byte != ECU_MSG_START_BYTE) { return -ERR_INVALID_START_BYTE; }

    if (msg->checksum != calc_crc(msg, offsetof(ECU_Msg_t, checksum))) { return -ERR_CRC_FAILED; }
    return 1;
}

void ecu_msg_to_str(char *str, const ECU_Msg_t *const msg)
{
    static const char fmt_str[] = "ECU_Msg:\n"
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
