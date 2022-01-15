#include "Header.h"

#ifdef USERSPACE_BUILD

#include <sys/time.h>

struct timeval time_to_timeval(const Time_t *time)
{
    struct timeval tv;
    tv.tv_sec  = time->tv_sec;
    tv.tv_usec = time->tv_nsec / 1000;
    return tv;
}

Time_t timeval_to_time(const struct timeval *time)
{
    Time_t t;
    t.tv_sec  = time->tv_sec;
    t.tv_nsec = time->tv_usec * 1000;
    return t;
}

Time_t get_time_now(void)
{
    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    return timeval_to_time(&tv_now);
}

struct Header_t header_make(u32 id, u32 length)
{
    Header_t hdr = {.start_byte = ECU_HEADER_START_BYTE, .length = (length), .id = (id), .stamp = get_time_now()};
    return hdr;
}

#endif
