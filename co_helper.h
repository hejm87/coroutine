#ifndef __CO_HELPER_H__
#define __CO_HELPER_H__

#include <sys/time.h>
#include <string>
#include "co_schedule.h"
#include "common/helper.h"

enum {
    CO_LEVEL_DEBUG = 0,
    CO_LEVEL_INFO,
    CO_LEVEL_WARN,
    CO_LEVEL_ERROR,
    CO_LEVEL_FATAL,
};

inline std::string date_ms(long time_ms = 0) {
    char date[32];
    time_t sec;
    long msec;
    if (time_ms > 0) {
        sec = time_ms / 1000;
        msec = time_ms % 1000;
    } else {
        struct timeval tvTime;
        gettimeofday(&tvTime, NULL);
        sec = tvTime.tv_sec;
        msec = tvTime.tv_usec / 1000;
    }

    struct tm tmTime;
    localtime_r(&sec, &tmTime);
    snprintf(
        date, 
        sizeof(date), 
        "%04d-%02d-%02d_%02d:%02d:%02d.%ld", 
        tmTime.tm_year + 1900,
        tmTime.tm_mon + 1,
        tmTime.tm_mday,
        tmTime.tm_hour,
        tmTime.tm_min,
        tmTime.tm_sec,
        msec
    );
    return date;
} 

#define CO_LOG_DEBUG(fmt, ...)  CO_LOG(CO_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define CO_LOG_INFO(fmt, ...)   CO_LOG(CO_LEVEL_INFO,  fmt, ##__VA_ARGS__)
#define CO_LOG_WARN(fmt, ...)   CO_LOG(CO_LEVEL_WARN,  fmt, ##__VA_ARGS__)
#define CO_LOG_ERROR(fmt, ...)  CO_LOG(CO_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define CO_LOG_FATAL(fmt, ...)  CO_LOG(CO_LEVEL_FATAL, fmt, ##__VA_ARGS__)

#define CO_LOG(level, fmt, ...) \
{ \
    printf("====> %s\n", date_ms().c_str()); \
    printf(fmt, ##__VA_ARGS__); \
    printf("\n"); \
}

//    Singleton<CoSchedule>::get_instance()->logger(level, "[FILE:%s,LINE:%d]" fmt, __FILE__, __LINE__, ##__VA_ARGS__);
#endif