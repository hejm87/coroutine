#ifndef __CO_SCHEDULE_H__
#define __CO_SCHEDULE_H__

#include <mutex>
#include <atomic>
#include <exception>
#include <functional>
#include <vector>
#include "co_define.h"
#include "co_helper.h"
#include "co_exception.h"
#include "common/any.h"
#include "common/any_func.h"
#include "co_common/co_tools.h"

class CoTimer;
class CoTimerId;
class CoAwaiter;
class CoExecutor;
class Coroutine;

extern thread_local shared_ptr<CoExecutor> g_co_executor;

class CoSchedule
{
public:
    CoSchedule();

    ~CoSchedule();

    void create(const AnyFunc& func, bool priority = false);

    CoAwaiter create_with_promise(const AnyFunc& func, bool priority = false);

    void sleep(int sleep_ms);

    void yield(std::function<void()> do_after = nullptr);

    void resume(std::shared_ptr<Coroutine> co);

    // 定时器调度后使用协程执行定时器逻辑
    CoTimerId set_timer(int delay_ms, const AnyFunc& func);

    bool stop_timer(const CoTimerId& timer_id);

    std::shared_ptr<Coroutine> get_cur_co();

    std::vector<std::shared_ptr<Coroutine>> get_global_co(int size = 1);

    void set_logger(std::function<void(int, const char*)> logger) {
        _logger = logger;
    }

    template <class... Args>
    void logger(int level, const char* file, int line, const char* msg, Args... args)
    {
        char buf[8192];
        int size = sizeof(buf);
        int offset = snprintf(buf, size, "[FILE:%s,LINE:%d] ", file, line);
        snprintf(buf + offset, size - offset, msg, args...);
        _logger(level, buf);
    }

    void free(shared_ptr<Coroutine> co) {
        _lst_free.push_back(co);
    }

    bool is_set_end() {
        return _is_set_end;
    }

private:
    bool get_free_co(std::shared_ptr<Coroutine> &co) {
        lock_guard<mutex> lock(_mutex);
        if (!_lst_free.front(co)) {
            return false;
        }
        _lst_free.pop_front();
    }

private:
    CoList  _lst_free;      // 协程空闲队列
    CoList  _lst_ready;     // 协程就绪队列
    CoList  _lst_suspend;   // 协程等待队列

    CoTimer _timer;

    int     _stack_size;
    int     _max_co_size;

    int     _cur_co_size;
    int     _executor_count;

    std::atomic<bool>    _is_set_end;
    std::vector<std::shared_ptr<CoExecutor>>  _executors;

    std::mutex  _mutex;

    std::function<void(int, const char*)>  _logger;
};

#endif
