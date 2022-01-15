#include "Interproc_Msg.h"

#ifdef USERSPACE_BUILD

#include <stddef.h>

struct Interproc_Msg_t interproc_msg_make(u32 id, const char name[64], const void *data,
                                          Interproc_Command_t command)
{
    Interproc_Msg_t dst = {.header = header_make(id, sizeof(Interproc_Msg_t)), .command = command};

    if (name) { strncpy(dst.name, name, 64); }

    if (data) { memcpy(dst.data, data, 8); }

    interproc_msg_calc_checksum(&dst);

    return dst;
}

CRC interproc_msg_calc_checksum(Interproc_Msg_t *msg)
{
    return msg->checksum = calc_crc(msg, offsetof(Interproc_Msg_t, checksum));
}

ecu_err_t interproc_msg_check(const Interproc_Msg_t *const msg)
{
    if (msg->header.start_byte != ECU_HEADER_START_BYTE) { return -ERR_INVALID_START_BYTE; }

    if (msg->checksum != calc_crc(msg, offsetof(Interproc_Msg_t, checksum))) { return -ERR_CRC_FAILED; }
    return 1;
}

#endif
