
#ifndef TYPES_H
#define TYPES_H

#ifdef USERSPACE_BUILD

#include <stdint.h>
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

#endif /* TYPES_H */