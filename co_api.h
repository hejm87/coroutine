#ifndef __CO_API_H__
#define __CO_API_H__

#include <unistd.h>
#include "co_common.h"

class CoApi
{
public:
    template <class F, class... Args>
    static void create(F&& f, Args&&... args) throw CoException {
        auto func = AnyFunc(forward<F>(f), forward<Args>(args)...);
        CoSchedule::get_instance()->create(func);
    }

    template <class F, class... Args>
    static CoAwaiter create_with_promise(F&& f, Args&&... args) throw CoException {
        auto func = AnyFunc(forward<F>(f), forward<Args>(args)...);
        return CoSchedule::get_instance()->create_with_promise(func);
    }

    template <class F, class... Args>
    static CoTimer set_timer(int delay_ms, F&& f, Args&&... args) {
        auto func = AnyFunc(forward<F>(f), forward<Args>(args)...);
        return CoSchedule::get_instance()->set_timer(func, delay_ms, 0);
    }

    template <class F, class... Args>
    static CoTimer set_timer(int delay_ms, int period_ms, F&& f, Args&&... args) {
        auto func = AnyFunc(forward<F>(f), forward<Args>(args)...);
        return CoSchedule::get_instance()->set_timer(func, delay_ms, period_ms);
    }

    // ??? 是否需要考虑协程线程外使用的情况
    static void sleep(int sleep_ms) {
        if (check_in_co_thread()) {
            CoSchedule::get_instance()->sleep(sleep_ms);
        } else {
            usleep(sleep_ms * 1000);
        }
    }
};

#endif