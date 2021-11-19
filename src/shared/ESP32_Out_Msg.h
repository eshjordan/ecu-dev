#ifndef ESP32_OUT_MSG_H
#define ESP32_OUT_MSG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "CAN_Msg.h"
#include "Header.h"

// clang-format off

#ifdef __cplusplus
#define STRUCT_INIT {}
#else
#define STRUCT_INIT
#endif

// clang-format on

#define ALIGN __attribute__((aligned(4)))

struct PWM_t {
    uint32_t frequency STRUCT_INIT;
    uint16_t duty STRUCT_INIT;
    uint8_t duty_resolution STRUCT_INIT;
} ALIGN;

typedef struct PWM_t PWM_t;

struct ESP32_Out_Msg_t {
    Header_t header STRUCT_INIT;
    double dac[2] STRUCT_INIT;
    PWM_t pwm[2] STRUCT_INIT;
    uint8_t dout[1] STRUCT_INIT;
    CAN_Msg_t can_msg STRUCT_INIT;
    CRC checksum STRUCT_INIT;
} ALIGN;

typedef struct ESP32_Out_Msg_t ESP32_Out_Msg_t;

CRC esp32_out_msg_calc_checksum(ESP32_Out_Msg_t *msg);

ecu_err_t esp32_out_msg_check(const ESP32_Out_Msg_t *msg);

/**
 * @brief Use at least 1024 bytes for the message buffer.
 *
 * @param str
 * @param msg
 */
void esp32_out_msg_to_str(char *str, const ESP32_Out_Msg_t *msg);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ESP32_OUT_MSG_H
