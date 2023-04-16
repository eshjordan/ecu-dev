
#ifndef TYPES_H
#define TYPES_H

#ifdef USERSPACE_BUILD

#include <stdint.h>
#include <stddef.h>
typedef uint8_t u8;

typedef int8_t s8;

typedef uint16_t u16;

typedef int16_t s16;

typedef uint32_t u32;

typedef int32_t s32;

__extension__ typedef unsigned long long u64;

__extension__ typedef __signed__ long long s64;

#else

#include <linux/types.h>

#endif /* USERSPACE_BUILD */

// #define ALIGN __attribute__((__packed__, aligned(1U))) // Potentially unsafe/slower on some architectures, TODO: Test
#define ALIGN __attribute__((aligned(4)))

// clang-format off

#ifdef __cplusplus
#define _STRUCT_INIT {}
#else
#define _STRUCT_INIT
#endif

// clang-format on

#endif /* TYPES_H */