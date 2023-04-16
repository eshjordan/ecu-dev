#ifndef PARAMETER_H
#define PARAMETER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Header.h"

#define PARAMETER_START_BYTE 0xEDU

// clang-format off

#ifdef __cplusplus
#define _PARAMETER_HEADER_INIT ={PARAMETER_START_BYTE, 0U, 0U}
#else
#define PARAMETER_START_BYTE_INIT
#endif

// clang-format on

// #define ALIGN __attribute__((__packed__, aligned(1U))) // Potentially unsafe/slower on some architectures, TODO: Test
#define ALIGN __attribute__((aligned(4)))

struct Parameter_t {
    enum Parameter_Type_t : uint8_t {
        PARAMETER_NOT_SET,
        PARAMETER_BOOL,
        PARAMETER_INTEGER,
        PARAMETER_DOUBLE,
        PARAMETER_STRING,
        PARAMETER_BYTE_ARRAY,
    } ALIGN;

    Header_t header _PARAMETER_HEADER_INIT;
    char name[64] _STRUCT_INIT;
    uint8_t value[8] _STRUCT_INIT; // Able to have 8 bytes / 64 bits (long long int) value at max
    enum Parameter_Type_t command _STRUCT_INIT;
    CRC checksum _STRUCT_INIT;
} ALIGN;


#ifdef __cplusplus
using Parameter_Type_t = Parameter_t::Parameter_Type_t;
#else
typedef enum Parameter_Type_t Parameter_Type_t;
#endif

typedef struct Parameter_t Parameter_t;

#ifdef USERSPACE_BUILD

/* Area for function definitions. */

#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif // PARAMETER_H
