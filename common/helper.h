#ifndef __COMMON_HELPER_H__
#define __COMMON_HELPER_H__

#include <unistd.h>
#include <sys/time.h>
#include <sys/syscall.h>

#define now_ms() \
({ \
    struct timeval tv; \
    gettimeofday(&tv, NULL); \
    (tv.tv_sec * 1000 + tv.tv_usec / 1000); \
})

#define now_us() \
({ \
    struct timeval tv; \
    gettimeofday(&tv, NULL); \
	(tv.tv_sec * 1000000 + tv.tv_usec); \
})

inline pid_t gettid()
{
	return syscall(SYS_gettid);
}

// 进程单例
template <class T>
class Singleton
{
public:
    static T* get_instance() {
        static T _instance;
        return &_instance;
    }
};

// 线程单例
template <class T>
class ThreadSingleton
{
public:
    static T* get_instance() {
        static thread_local T _instance;
        return &_instance;
    }
};

#endif
