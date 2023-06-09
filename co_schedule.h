#ifndef __CO_SCHEDULE_H__
#define __CO_SCHEDULE_H__

#include <mutex>
#include <atomic>
#include <functional>
#include <exception>
#include <vector>
#include <map>
#include <condition_variable>
#include "co_define.h"
#include "co_helper.h"
#include "co_exception.h"
#include "co_common/co_tools.h"

class CoTimer;
class CoTimerId;
class CoExecutor;
class Coroutine;

extern thread_local std::shared_ptr<CoExecutor> g_co_executor;
//extern thread_local CoExecutor* g_co_executor;

class CoSchedule
{
public:
    CoSchedule();

    ~CoSchedule();

    void create(bool priority, const std::function<void()>& func);

    void sleep(int sleep_ms);

    void yield(std::function<void()> doing = nullptr);

    void suspend(std::function<bool()> do_and_check);

    void resume(std::shared_ptr<Coroutine> co);

    void resume(std::function<std::shared_ptr<Coroutine>()> do_and_resume);

    void release();

    CoTimerId set_timer(int delay_ms, const std::function<void()>& func);

    bool stop_timer(const CoTimerId& timer_id);

    std::shared_ptr<Coroutine> get_running_co();

    std::vector<std::shared_ptr<Coroutine>> get_global_co(int size = 1);

    void set_logger(std::function<void(int, const char*)> logger) {
        _logger = logger;
    }

    template <class... Args>
    void logger(int level, const char* msg, const char* file, int line, Args... args)
    {
        static std::map<int, const char*> _level_str = {
            {CO_LEVEL_DEBUG, "debug"},
            {CO_LEVEL_INFO,  "info"},
            {CO_LEVEL_WARN,  "warn"},
            {CO_LEVEL_ERROR, "error"},
            {CO_LEVEL_FATAL, "fatal"},
        };

        auto iter = _level_str.find(level);
        if (iter == _level_str.end()) {
            return ;
        }

        char buf[8192];
        snprintf(buf, sizeof(buf), msg, file, line, iter->second, args...);
        if (_logger) {
            _logger(level, buf);
        }
    }

    std::shared_ptr<Coroutine> get_coroutine(int index) {
        std::shared_ptr<Coroutine> ptr;
        if (index >= 0 && index < (int)_coroutines.size()) {
            ptr = _coroutines[index];
        }
        return ptr;
    }

    bool is_set_end() {
        return _is_set_end;
    }

private:
    bool get_free_co(std::shared_ptr<Coroutine> &co) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (!_lst_free.front(co)) {
            return false;
        }
        _lst_free.pop_front();
        return true;
    }

private:
    CoList  _lst_free;
    CoList  _lst_ready;
    CoList  _lst_suspend;

    std::multimap<long, std::shared_ptr<Coroutine>>  _lst_sleep;

    CoTimer*    _timer;

    int     _stack_size;
    int     _executor_count;

    std::atomic<bool>    _is_set_end;
    std::vector<std::shared_ptr<CoExecutor>>  _executors;

    std::vector<std::shared_ptr<Coroutine>>   _coroutines;

    std::mutex  _mutex;
    std::condition_variable _cv;

    std::function<void(int, const char*)>  _logger;

    int     _log_level;
};

#endif
