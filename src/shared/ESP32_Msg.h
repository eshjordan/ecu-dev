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

#define ESP32_UNKNOWN 0x0U
#define ESP32_ACK 0x1U
#define ESP32_NACK 0x2U
#define ESP32_IN_ADC 0x3U
#define ESP32_IN_HALL 0x4U
#define ESP32_IN_DIN 0x5U
#define ESP32_IN_CAN 0x6U
#define ESP32_OUT_DAC 0x7U
#define ESP32_OUT_PWM 0x8U
#define ESP32_OUT_DOUT 0x9U
#define ESP32_OUT_CAN 0xAU

typedef struct ESP32_Request_t {
    uint8_t seed;
    uint8_t type;
    uint8_t channel;
    CRC checksum;
} ALIGN ESP32_Request_t;

typedef struct ESP32_In_ADC_t {
    uint8_t seed;
    uint32_t adc;
    CRC checksum;
} ALIGN ESP32_In_ADC_t;

typedef struct ESP32_In_Hall_t {
    uint8_t seed;
    uint32_t hall;
    CRC checksum;
} ALIGN ESP32_In_Hall_t;

typedef struct ESP32_In_DIN_t {
    uint8_t seed;
    uint8_t din;
    CRC checksum;
} ALIGN ESP32_In_DIN_t;

typedef struct ESP32_Out_DAC_t {
    uint8_t seed;
    uint8_t dac; // 0-255, 0 = 0V, 255 = 3.3V
    CRC checksum;
} ALIGN ESP32_Out_DAC_t;

typedef struct ESP32_Out_PWM_t {
    uint8_t seed;
    uint8_t duty_resolution;
    uint16_t duty;
    uint32_t frequency;
    CRC checksum;
} ALIGN ESP32_Out_PWM_t;

typedef struct ESP32_Out_DOUT_t {
    uint8_t seed;
    uint8_t dout;
    CRC checksum;
} ALIGN ESP32_Out_DOUT_t;

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ESP32_IN_MSG_H
