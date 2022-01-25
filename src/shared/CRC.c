#include "CRC.h"
// #include <assert.h>

#if CRC_WIDTH != 32

#define POLYNOMIAL 0x8005U
#define INITIAL_REMAINDER 0x0000U
#define FINAL_XOR_VALUE 0x0000U
#define CHECK_VALUE 0xBB3DU // for "123456789"

#else

#define POLYNOMIAL 0x04C11DB7U
#define INITIAL_REMAINDER 0xFFFFFFFFU
#define FINAL_XOR_VALUE 0xFFFFFFFFU
#define CHECK_VALUE 0xCBF43926U // for "123456789"

#endif

static CRC reflect(CRC data, u8 num_bits);

#define REFLECT_DATA(X) ((u8)reflect((X), 8))
#define REFLECT_REMAINDER(X) ((CRC)reflect((X), WIDTH))

#define WIDTH (8U * sizeof(CRC))
#define TOPBIT 1U << (WIDTH - 1U)

typedef u8 byte_t;

static CRC crcTable[256] = {0};
static u8 inited    = 0;

void init_crc(void)
{
    u16 dividend;
    u8 bit;

    for (dividend = 0; dividend < 256; dividend++) /* iterate over all possible input byte values 0 - 255 */
    {
        /*
         * Start with the dividend followed by zeros.
         */
        CRC remainder = (CRC)(dividend << (WIDTH - 8U)); /* move dividend byte into MSB of 32Bit CRC */

        /*
         * Perform modulo-2 division, a bit at a time.
         */
        for (bit = 0; bit < 8; bit++)
        {
            /*
             * Try to divide the current data bit.
             */
            if (remainder & TOPBIT)
            {
                remainder <<= 1U;
                remainder ^= POLYNOMIAL;
            } else
            {
                remainder <<= 1U;
            }
        }

        /*
         * Store the result into the table.
         */
        crcTable[dividend] = remainder;
    }

    inited = 1;
    // assert(calc_crc("123456789", 9) == CHECK_VALUE);
}

CRC calc_crc(const void *const data, const u16 length)
{
    u16 i;

    if (!inited) { init_crc(); }

    byte_t *bytes = (byte_t *)data;

    CRC remainder = INITIAL_REMAINDER; /* CRC is set to specified initial value */
    for (i = 0; i < length; i++)
    {
        byte_t b = bytes[i];
        /* reflect input byte if specified, otherwise input byte is taken as it is */
        byte_t curByte = REFLECT_DATA(b);

        /* XOR-in next input byte into MSB of crc and get this MSB, that's our new intermediate dividend */
        byte_t pos = (byte_t)((remainder ^ (curByte << (WIDTH - 8U))) >> (WIDTH - 8U));
        /* Shift out the MSB used for division per lookuptable and XOR with the remainder */
        remainder = (CRC)((remainder << 8U) ^ (CRC)(crcTable[pos]));
    }
    /* reflect result crc if specified, otherwise calculated crc value is taken as it is */
    remainder = REFLECT_REMAINDER(remainder);
    /* Xor the crc value with specified final XOR value before returning */
    return (CRC)(remainder ^ FINAL_XOR_VALUE);
}

static CRC reflect(CRC data, const u8 num_bits)
{
    u8 bit;
    CRC reflection = 0;

    /*
     * Reflect the data about the center bit.
     */
    for (bit = 0; bit < num_bits; ++bit)
    {
        /*
         * If the LSB bit is set, set the reflection of it.
         */
        if (data & 0x01) { reflection |= (1 << ((num_bits - 1) - bit)); }

        data = (data >> 1);
    }

    return (reflection);

} /* reflect() */


//MODULE_LICENSE("GPL v2");

