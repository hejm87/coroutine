#ifndef __CO_SCHEDULE_H__
#define __CO_SCHEDULE_H__

#include <mutex>
#include <atomic>
#include <exception>
#include "co_define.h"
#include "co_common.h"
#include "common/any.h"

extern thread_local context_t*      g_ctx_main = NULL;
extern thread_local CoExecutor*     g_co_executor = NULL;

class CoSchedule
{
friend class CoChannel;
public:
    CoSchedule* get_instance() {
        static CoSchedule instance;
        return &instance;
    }

    void create(const AnyFunc& func, bool priority = false) throw CoException;

    CoAwaiter create_with_promise(const AnyFunc& func, bool priority = false);

    void sleep(int sleep_ms);

    void yield(function<void()> do_after = nullptr);

    void resume(shared_ptr<Coroutine> co);

    /*
    函数功能：设置定时器
    参数：
    func:   定时器回调函数
    delay:  延迟执行时间（单位：毫秒）
    period: 周期性执行周日（单位：毫秒），默认参数为0，只执行一次 
    */
    CoTimerId set_timer(const AnyFunc& func, int delay_ms, int period_ms = 0);

    bool stop_timer(const CoTimerId& timer_id);

    bool is_set_end() {
        return _is_set_end;
    }

    bool set_stack_size(int size) {
        _stack_size = size;
    }

    int get_stack_size() {
        return _stack_size;
    }

private:
    CoSchedule();
    ~CoSchedule();

    void run_timer();

    bool get_free_co(shared_ptr<Coroutine> &co) {
        lock_guard<mutex> lock(_mutex);
        return _lst_free.pop_front(co);
    }

    void free(shared_ptr<Coroutine> co) {
        _lst_free.push_back(co);
    }

private:
    CoList  _lst_free;
    CoList  _lst_ready;

    int     _stack_size;
    int     _max_co_size;

    int     _cur_co_size;
    int     _executor_count;

    CoTimerList     _lst_timer;

    std::atomic<bool>    _is_set_end;
    std::vector<std::shared_ptr<CoExecutor>>  _executors;

    std::condition_variable _cv;

    std::mutex   _mutex;
};

#endif