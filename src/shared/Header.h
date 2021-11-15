#ifndef HEADER_H
#define HEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "CRC.h"
#include "ECU_Error.h"

#define ECU_HEADER_START_BYTE 0x9DU

// clang-format off

#ifdef __cplusplus
#define STRUCT_INIT {}
#define START_BYTE_INIT =ECU_HEADER_START_BYTE
#else
#define STRUCT_INIT
#define START_BYTE_INIT
#endif

// clang-format on

// #define ALIGN __attribute__((__packed__, aligned(1U))) // Potentially unsafe/slower on some architectures, TODO: Test
#define ALIGN __attribute__((aligned(4)))

struct Time_t {
    int64_t tv_sec;  /* Seconds.  */
    int64_t tv_nsec; /* Nanoseconds.  */
} ALIGN;

typedef struct Time_t Time_t;

struct Header_t {
    uint8_t start_byte START_BYTE_INIT;
    uint32_t length STRUCT_INIT;
    uint32_t id STRUCT_INIT;
    Time_t stamp STRUCT_INIT;
} ALIGN;

typedef struct Header_t Header_t;

struct timeval time_to_timeval(const Time_t *time);

Time_t timeval_to_time(const struct timeval *time);

Time_t get_time_now(void);

struct Header_t header_make(uint32_t id, uint32_t length);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // HEADER_H
