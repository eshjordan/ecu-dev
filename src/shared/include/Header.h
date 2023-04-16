#ifndef HEADER_H
#define HEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "CRC.h"
#include "ECU_Error.h"

struct Header_t {
    uint8_t start_byte _STRUCT_INIT;
    uint8_t _pad1 _STRUCT_INIT;
    uint16_t length _STRUCT_INIT;
} ALIGN;

typedef struct Header_t Header_t;

Header_t header_make(uint8_t start_byte, uint16_t length);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // HEADER_H
