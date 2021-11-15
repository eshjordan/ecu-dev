#ifndef ECU_ERROR_H
#define ECU_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define ERR_CRC_FAILED 1
#define ERR_INVALID_LENGTH 2
#define ERR_INVALID_START_BYTE 3

typedef int16_t ecu_err_t;

void ecu_err_to_str(char *str, ecu_err_t err_code);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
