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
    s64 tv_sec;  /* Seconds.  */
    s64 tv_nsec; /* Nanoseconds.  */
} ALIGN;

typedef struct Time_t Time_t;

struct Header_t {
    u8 start_byte START_BYTE_INIT;
    u32 length STRUCT_INIT;
    u32 id STRUCT_INIT;
    Time_t stamp STRUCT_INIT;
} ALIGN;

typedef struct Header_t Header_t;

#ifdef USERSPACE_BUILD

struct timeval time_to_timeval(const Time_t *time);

Time_t timeval_to_time(const struct timeval *time);

Time_t get_time_now(void);

struct Header_t header_make(u32 id, u32 length);

#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif // HEADER_H
