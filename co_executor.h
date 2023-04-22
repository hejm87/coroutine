#ifndef __COROUTINE_EXECUTOR_H__
#define __COROUTINE_EXECUTOR_H__

#include <mutex>
#include <atomic>
#include <memory>
#include <exception>
#include <list>
#include <multimap>

#include "coroutine.h"
#include "co_define.h"
#include "co_executor.h"

class CoSchedule;

class CoExecutor
{
public:
    CoExecutor(); 
    ~CoExecutor();

    bool run();
    bool wait();

	void put(std::shared_ptr<Coroutine> coroutine);

    void sleep(int sleep_ms);
    void yield(std::function<void()> do_after = nullptr);
    void resume(std::shared_ptr<Coroutine> coroutine) throw CoException;

private:
    bool on_timer();
    bool on_execute();

    bool get_ready_co(std::shared_ptr<Coroutine>& co);
    bool get_global_co(std::vector<std::shared_ptr<Coroutine>>& cos, int size = DEF_GET_GLOBAL_CO_SIZE);

private:
    CoList      _lst_wait;      // 等待队列
    CoList      _lst_ready;     // 就绪队列
    CoTimerList _lst_timer;     // 定时器队列

    std::shared_ptr<Coroutine>   _running_co;

    std::thread         _thread_handle;
    std::atomic<bool>   _is_running;
    std::atomic<bool>   _is_set_end;

    mutex   _mutex;
};

#endif