#ifndef __CO_API_H__
#define __CO_API_H__

#include <unistd.h>
#include "co_define.h"
#include "co_schedule.h"
#include "co_common/co_common.h"
#include "co_common/co_timer.h"

class CoApi
{
public:
    template <class F, class... Args>
    static void create(F&& f, Args&&... args) {
        auto func = std::bind(forward<F>(f), forward<Args>(args)...);
		Singleton<CoSchedule>::get_instance()->create(false, func);
    }

    template <class F, class... Args>
    static auto create_with_promise(F&& f, Args&&... args)
        -> CoAwaiter<typename std::enable_if<std::is_void<typename std::result_of<F(Args...)>::type>::value>::type>
    {
        CoAwaiter<void> awaiter;
        auto co_func = std::bind(forward<F>(f), forward<Args>(args)...);
        auto func = [co_func, &awaiter]() {
            co_func();
            awaiter.set_value();
        };
        Singleton<CoSchedule>::get_instance()->create(false, func);
        return awaiter;
    }

    template <class F, class... Args>
    static auto create_with_promise(F&& f, Args&&... args)
        -> CoAwaiter<typename std::enable_if<!std::is_void<typename std::result_of<F(Args...)>::type>::value>::type>
    {
        using utype = typename std::result_of<F(Args...)>::type;
        CoAwaiter<utype> awaiter;
        auto co_func = std::bind(forward<F>(f), forward<Args>(args)...);
        // xxxx 这里肯定有问题
        auto func = [co_func, &awaiter]() {
            awaiter.set_value(co_func());
        };
        Singleton<CoSchedule>::get_instance()->create(false, func);
        return awaiter;
    }

    template <class F, class... Args>
    static CoTimerId set_timer(int delay_ms, F&& f, Args&&... args) {
        auto func = AnyFunc(forward<F>(f), forward<Args>(args)...);
		return Singleton<CoSchedule>::get_instance()->set_timer(delay_ms, func);
    }

    static void sleep(int sleep_ms) {
        if (!is_in_co_thread()) {
            usleep(sleep_ms);
        } else {
			Singleton<CoSchedule>::get_instance()->sleep(sleep_ms);
        }
    }

    static int getcid() {
        if (!is_in_co_thread()) {
            throw CoException(CO_ERROR_NOT_IN_CO_THREAD);
        }
        auto co = Singleton<CoSchedule>::get_instance()->get_running_co();
        if (!co) {
            CO_LOG_DEBUG("############ tid:%d, getcid, co is invalid", gettid());
        }
        return co->_id;
    }

    static void get_info(CoInfo& info) {
        info.stack_size = get_stack_size();
        info.executor_count = get_executor_count();
        info.coroutine_count = get_coroutine_count();
    }
};

#endif
