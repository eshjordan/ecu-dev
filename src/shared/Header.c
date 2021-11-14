#include "Header.h"
#include <stdio.h>
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

struct Header_t make_header(uint32_t id, uint32_t length)
{
    Header_t hdr = {.start_byte = ECU_MSG_START_BYTE, .length = (length), .id = (id), .stamp = get_time_now()};
    return hdr;
}
