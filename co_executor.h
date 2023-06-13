#ifndef __COROUTINE_EXECUTOR_H__
#define __COROUTINE_EXECUTOR_H__

#include <mutex>
#include <atomic>
#include <memory>
#include <exception>
#include <list>
#include <map>
#include <thread>
#include <mutex>

#include "co_define.h"

class Coroutine;

class CoExecutor //: public std::enable_shared_from_this<CoExecutor>
{
public:
    CoExecutor(); 
    ~CoExecutor();

    bool run();
    void stop(bool wait = true);
    bool wait_util_stop();

    void sleep(int sleep_ms);
//    void yield(std::function<void()> do_after = nullptr);
//    void resume(std::shared_ptr<Coroutine> co);

    std::shared_ptr<Coroutine> get_running_co();

    void set_end();

private:
    bool on_awake();
    bool on_execute();

    bool get_ready_co(std::shared_ptr<Coroutine>& co);

private:
    CoList      _lst_ready;
    std::multimap<long, std::shared_ptr<Coroutine>>  _lst_sleep;

    std::shared_ptr<Coroutine>   _running_co;

    std::atomic<bool>   _is_running;
    std::atomic<bool>   _is_set_end;

    std::thread  _thread;
    std::mutex   _mutex;
};

#endif