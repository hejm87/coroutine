#ifndef __CO_API_H__
#define __CO_API_H__

#include <unistd.h>
#include "co_define.h"
#include "co_schedule.h"

class CoApi
{
public:
    template <class F, class... Args>
    static void create(F&& f, Args&&... args) {
        auto func = AnyFunc(forward<F>(f), forward<Args>(args)...);
        CoSchedule::get_instance()->create(func);
    }

    template <class F, class... Args>
    static CoAwaiter create_with_promise(F&& f, Args&&... args) {
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

    static void sleep(int sleep_ms) {
        if (!is_in_co_thread()) {
            throw CoException(CO_ERROR_NOT_IN_CO_THREAD);
        }
        CoSchedule::get_instance()->sleep(sleep_ms);
    }
};

#endif
