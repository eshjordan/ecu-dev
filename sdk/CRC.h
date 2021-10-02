#ifndef CRC_H
#define CRC_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The width of the CRC calculation and result.
 * Modify the typedef for a 16 or 32-bit CRC standard.
 */
#define CRC_WIDTH 32

#if CRC_WIDTH != 32
typedef unsigned short int CRC;
#else
typedef unsigned int CRC;
#endif

void init_crc(void);

CRC calc_crc(const void *data, unsigned short int length);

static CRC reflect(CRC data, unsigned char num_bits);

#define REFLECT_DATA(X) ((unsigned char)reflect((X), 8))
#define REFLECT_REMAINDER(X) ((CRC)reflect((X), WIDTH))

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CRC_H
