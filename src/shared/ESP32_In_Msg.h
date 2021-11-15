#ifndef ESP32_IN_MSG_H
#define ESP32_IN_MSG_H

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

struct ESP32_In_Msg_t {
    Header_t header STRUCT_INIT;
    uint32_t adc[9] STRUCT_INIT;
    uint32_t hall_effect STRUCT_INIT;
    uint8_t din1 STRUCT_INIT;
    uint8_t din2 STRUCT_INIT;
    uint8_t din3 STRUCT_INIT;
    CAN_Msg_t can_msg STRUCT_INIT;
    CRC checksum STRUCT_INIT;
} ALIGN;

typedef struct ESP32_In_Msg_t ESP32_In_Msg_t;

CRC esp32_in_msg_calc_checksum(ESP32_In_Msg_t *msg);

ecu_err_t esp32_in_msg_check(const ESP32_In_Msg_t *msg);

/**
 * @brief Use at least 1024 bytes for the message buffer.
 *
 * @param str
 * @param msg
 */
void esp32_in_msg_to_str(char *str, const ESP32_In_Msg_t *msg);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ESP32_IN_MSG_H
