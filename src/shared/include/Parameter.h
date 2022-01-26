#ifndef PARAMETER_H
#define PARAMETER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Header.h"

#define ECU_MSG_START_BYTE 0x9DU

// clang-format off

#ifdef __cplusplus
#define STRUCT_INIT {}
#define START_BYTE_INIT =ECU_MSG_START_BYTE
#else
#define STRUCT_INIT
#define START_BYTE_INIT
#endif

// clang-format on

// #define ALIGN __attribute__((__packed__, aligned(1U))) // Potentially unsafe/slower on some architectures, TODO: Test
#define ALIGN __attribute__((aligned(4)))

struct Parameter_t {
    enum Parameter_Type_t {
        UNDEFINED = 0,
        SHORT_INT,
        INT,
        LONG_INT,
        LONG_LONG_INT,
        UNSIGNED_SHORT_INT,
        UNSIGNED_INT,
        UNSIGNED_LONG_INT,
        UNSIGNED_LONG_LONG_INT,
        BOOL,
        SIGNED_CHAR,
        UNSIGNED_CHAR,
        CHAR,
        WCHAR,
        CHAR16,
        CHAR32,
        FLOAT,
        DOUBLE,
        LONG_DOUBLE,
        STRING
    }; ALIGN;

    Header_t header STRUCT_INIT;
    char name[64] STRUCT_INIT;
    uint8_t value[8] STRUCT_INIT; // Able to have 8 bytes / 64 bits (long long int) value at max
    enum Parameter_Type_t command STRUCT_INIT;
    CRC checksum STRUCT_INIT;
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
