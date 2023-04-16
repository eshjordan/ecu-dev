#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

struct Time_t {
    int64_t tv_sec;  /* Seconds.  */
    int64_t tv_nsec; /* Nanoseconds.  */
} ALIGN;

typedef struct Time_t Time_t;

#ifdef USERSPACE_BUILD

struct timeval time_to_timeval(const Time_t *time);

Time_t timeval_to_time(const struct timeval *time);

Time_t get_time_now(void);

#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif // TIMESTAMP_H
