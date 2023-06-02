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
            printf("goto normal sleep\n");
            usleep(sleep_ms);
        } else {
            printf("goto coroutine sleep\n");
			Singleton<CoSchedule>::get_instance()->sleep(sleep_ms);
        }
    }
};

#endif
