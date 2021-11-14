#include "ESP32Msg.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

CRC calc_esp_checksum(ESP32Msg_t *const msg) { return msg->checksum = calc_crc(msg, offsetof(ESP32Msg_t, checksum)); }

int check_esp(const ESP32Msg_t *const msg)
{
    if (msg->header.start_byte != ECU_MSG_START_BYTE) { return -ERR_INVALID_START_BYTE; }

    if (msg->checksum != calc_crc(msg, offsetof(ESP32Msg_t, checksum))) { return -ERR_CRC_FAILED; }
    return 1;
}

void esp_err_to_str(char *str, const int err_code)
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

void ESP32Msg_to_str(char *str, const ESP32Msg_t *const msg)
{
    static const char fmt_str[] = "ESP32Msg:\n"
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
                                  "    adc:\n"
                                  "        adc[0]: %u\n"
                                  "        adc[1]: %u\n"
                                  "        adc[2]: %u\n"
                                  "        adc[3]: %u\n"
                                  "        adc[4]: %u\n"
                                  "        adc[5]: %u\n"
                                  "        adc[6]: %u\n"
                                  "        adc[7]: %u\n"
                                  "        adc[8]: %u\n"
                                  "    hall_effect: %u\n"
                                  "    din1: %hu\n"
                                  "    din2: %hu\n"
                                  "    din3: %hu\n"
                                  "    CANMsg:\n"
                                  "        flags: %u\n"
                                  "        identifier: %u\n"
                                  "        data_length_code: %hu\n"
                                  "        data:\n"
                                  "            data[0]: %hu\n"
                                  "            data[1]: %hu\n"
                                  "            data[2]: %hu\n"
                                  "            data[3]: %hu\n"
                                  "            data[4]: %hu\n"
                                  "            data[5]: %hu\n"
                                  "            data[6]: %hu\n"
                                  "            data[7]: %hu\n"
                                  "        checksum: %u\n"
                                  "    checksum: %u\n"
                                  "\n";

    sprintf(str, fmt_str, msg->header.start_byte, msg->header.length, msg->header.id, msg->header.stamp.tv_sec,
            msg->header.stamp.tv_nsec, msg->adc[0], msg->adc[1], msg->adc[2], msg->adc[3], msg->adc[4], msg->adc[5],
            msg->adc[6], msg->adc[7], msg->adc[8], msg->hall_effect, msg->din1, msg->din2, msg->din3, msg->CANMsg.flags,
            msg->CANMsg.identifier, msg->CANMsg.data_length_code, msg->CANMsg.data[0], msg->CANMsg.data[1],
            msg->CANMsg.data[2], msg->CANMsg.data[3], msg->CANMsg.data[4], msg->CANMsg.data[5], msg->CANMsg.data[6],
            msg->CANMsg.data[7], msg->CANMsg.checksum, msg->checksum);
}
