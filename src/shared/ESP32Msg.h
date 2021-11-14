#ifndef ESP32MSG_H
#define ESP32MSG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "CANMsg.h"
#include "CRC.h"
#include "Header.h"
#include <stdint.h>
#include <stdio.h>

// clang-format off

#ifdef __cplusplus
#define STRUCT_INIT
#else
#define STRUCT_INIT
#endif

// clang-format on

#define ALIGN __attribute__((aligned(4)))

struct ESP32Msg_t {
    Header_t header STRUCT_INIT;
    uint32_t adc[9] STRUCT_INIT;
    uint32_t hall_effect STRUCT_INIT;
    uint8_t din1 STRUCT_INIT;
    uint8_t din2 STRUCT_INIT;
    uint8_t din3 STRUCT_INIT;
    CANMsg_t CANMsg STRUCT_INIT;
    CRC checksum STRUCT_INIT;
} ALIGN;

typedef struct ESP32Msg_t ESP32Msg_t;

CRC calc_esp_checksum(ESP32Msg_t *msg);

int check_esp(const ESP32Msg_t *msg);

void esp_err_to_str(char *str, int err_code);

/**
 * @brief Use at least 1024 bytes for the message buffer.
 *
 * @param str
 * @param msg
 */
void ESP32Msg_to_str(char *str, const ESP32Msg_t *msg);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ESP32MSG_H
