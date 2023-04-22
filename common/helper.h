#ifndef __COMMON_HELPER_H__
#define __COMMON_HELPER_H__

#include <sys/time.h>

#define now_ms() \
({ \
    struct timeval tv; \
    gettimeofday(&tv, NULL); \
    (tv.tv_sec * 1000 + tv.tv_usec / 1000); \
})

#endif