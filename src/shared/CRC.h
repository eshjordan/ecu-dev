#ifndef CRC_H
#define CRC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*
 * The width of the CRC calculation and result.
 * Modify the typedef for a 16 or 32-bit CRC standard.
 */
#define CRC_WIDTH 32

#if CRC_WIDTH != 32
typedef uint16_t CRC;
#else
typedef uint32_t CRC;
#endif

void init_crc(void);

CRC calc_crc(const void *data, uint16_t length);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CRC_H
