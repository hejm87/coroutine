#ifndef __CO_HELPER_H__
#define __CO_HELPER_H__

#include "co_schedule.h"

enum {
    CO_DEBUG = 0,
    CO_INFO,
    CO_WARN,
    CO_ERROR,
    CO_FATAL,
};

#define CO_LOG_DEBUG(fmt, ...)  CO_LOG(CO_DEBUG, fmt, ##__VA_ARGS__)
#define CO_LOG_INFO(fmt, ...)   CO_LOG(CO_INFO,  fmt, ##__VA_ARGS__)
#define CO_LOG_WARN(fmt, ...)   CO_LOG(CO_WARN,  fmt, ##__VA_ARGS__)
#define CO_LOG_ERROR(fmt, ...)  CO_LOG(CO_ERROR, fmt, ##__VA_ARGS__)
#define CO_LOG_FATAL(fmt, ...)  CO_LOG(CO_FATAL, fmt, ##__VA_ARGS__)

#define CO_LOG(level, fmt, ...) \
{ \
    CoSchedule::get_instance()->logger(level, __FILE__, __LINE__, fmt, ##__VA_ARGS__); \
}

#endif