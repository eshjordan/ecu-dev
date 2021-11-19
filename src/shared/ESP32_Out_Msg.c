#include "ESP32_Out_Msg.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

CRC esp32_out_msg_calc_checksum(ESP32_Out_Msg_t *const msg)
{
    return msg->checksum = calc_crc(msg, offsetof(ESP32_Out_Msg_t, checksum));
}

ecu_err_t esp32_out_msg_check(const ESP32_Out_Msg_t *const msg)
{
    if (msg->header.start_byte != ECU_HEADER_START_BYTE) { return -ERR_INVALID_START_BYTE; }

    if (msg->checksum != calc_crc(msg, offsetof(ESP32_Out_Msg_t, checksum))) { return -ERR_CRC_FAILED; }
    return 1;
}

void esp32_out_msg_to_str(char *str, const ESP32_Out_Msg_t *const msg)
{
    static const char fmt_str[] = "ESP_Out_Msg:\n"
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
                                  "    dac:\n"
                                  "        dac[0]: %lf\n"
                                  "        dac[1]: %lf\n"
                                  "    pwm:\n"
                                  "        pwm[0]:\n"
                                  "            frequency: %lf\n"
                                  "            duty: %lf\n"
                                  "        pwm[1]:\n"
                                  "            frequency: %lf\n"
                                  "            duty: %lf\n"
                                  "    dout:\n"
                                  "        dout[0]: %u\n"
                                  "    can_msg:\n"
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
            msg->header.stamp.tv_nsec, msg->dac[0], msg->dac[1], msg->pwm[0].frequency, msg->pwm[0].duty,
            msg->pwm[1].frequency, msg->pwm[1].duty, msg->dout[0], msg->can_msg.flags, msg->can_msg.identifier,
            msg->can_msg.data_length_code, msg->can_msg.data[0], msg->can_msg.data[1], msg->can_msg.data[2],
            msg->can_msg.data[3], msg->can_msg.data[4], msg->can_msg.data[5], msg->can_msg.data[6],
            msg->can_msg.data[7], msg->can_msg.checksum, msg->checksum);
}
