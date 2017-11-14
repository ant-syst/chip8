#ifndef H_TIMESPEC
#define H_TIMESPEC

#include <sys/time.h>
#include <stdint.h>

#define NS_PER_SEC 1000000000L
#define FREQUENCY_TO_NS(HZ)((NS_PER_SEC)/(HZ))

static inline void timespec_diff(struct timespec *start, struct timespec *stop, struct timespec *result)
{
    if ((stop->tv_nsec - start->tv_nsec) < 0)
    {
        result->tv_sec = stop->tv_sec - start->tv_sec - 1;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
    }
    else
    {
        result->tv_sec = stop->tv_sec - start->tv_sec;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec;
    }
}

static inline void timespec_add_ns2(struct timespec *ts, uint64_t ns)
{
    ts->tv_sec += ns / NS_PER_SEC;
    ts->tv_nsec = ns % NS_PER_SEC;
}

#endif
