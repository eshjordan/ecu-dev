#ifndef CRC_H
#define CRC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

/*
 * The width of the CRC calculation and result.
 * Modify the typedef for a 16 or 32-bit CRC standard.
 */
#define CRC_WIDTH 32

#if CRC_WIDTH != 32
typedef u16 CRC;
#else
typedef u32 CRC;
#endif

#ifdef USERSPACE_BUILD

void init_crc(void);

CRC calc_crc(const void *data, u16 length);

#else

#include <linux/crc32.h>

inline CRC calc_crc(const void *data, u16 length)
{
	const CRC INITIAL_REMAINDER = 0xFFFFFFFFU;
	const CRC FINAL_XOR_VALUE = 0xFFFFFFFFU;
	return crc32(INITIAL_REMAINDER, data, length) ^ FINAL_XOR_VALUE;
}

#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CRC_H
