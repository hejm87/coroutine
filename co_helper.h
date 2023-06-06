#ifndef __CO_HELPER_H__
#define __CO_HELPER_H__

#include "co_schedule.h"
#include "common/helper.h"

enum {
    CO_LEVEL_DEBUG = 0,
    CO_LEVEL_INFO,
    CO_LEVEL_WARN,
    CO_LEVEL_ERROR,
    CO_LEVEL_FATAL,
};

#define CO_LOG_DEBUG(fmt, ...)  CO_LOG(CO_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define CO_LOG_INFO(fmt, ...)   CO_LOG(CO_LEVEL_INFO,  fmt, ##__VA_ARGS__)
#define CO_LOG_WARN(fmt, ...)   CO_LOG(CO_LEVEL_WARN,  fmt, ##__VA_ARGS__)
#define CO_LOG_ERROR(fmt, ...)  CO_LOG(CO_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define CO_LOG_FATAL(fmt, ...)  CO_LOG(CO_LEVEL_FATAL, fmt, ##__VA_ARGS__)

#define CO_LOG(level, fmt, ...) \
{ \
    printf(fmt, ##__VA_ARGS__); \
    printf("\n"); \
}

//    Singleton<CoSchedule>::get_instance()->logger(level, "[FILE:%s,LINE:%d]" fmt, __FILE__, __LINE__, ##__VA_ARGS__);
#endif