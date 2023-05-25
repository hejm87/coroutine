#ifndef __COMMON_HELPER_H__
#define __COMMON_HELPER_H__

#include <sys/time.h>

#define now_ms() \
({ \
    struct timeval tv; \
    gettimeofday(&tv, NULL); \
    (tv.tv_sec * 1000 + tv.tv_usec / 1000); \
})

// 进程单例
template <class T>
class SingleInstance
{
public:
    static T* get_instance() {
        static T _instance;
        return &_instance;
    }
};

// 线程单例
template <class T>
class ThreadSingleInstance
{
public:
    static T* get_instance() {
        static thread_local T _instance;
        return &_instance;
    }
};

#endif
