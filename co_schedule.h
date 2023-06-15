#ifndef __CO_SCHEDULE_H__
#define __CO_SCHEDULE_H__

#include <mutex>
#include <atomic>
#include <functional>
#include <exception>
#include <vector>
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

    void suspend(std::function<void()> doing = nullptr);

    void release();

    void resume(std::shared_ptr<Coroutine> co);

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
        char buf[8192];
        snprintf(buf, sizeof(buf), msg, file, line, args...);
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

    void append_ready_list(const std::vector<std::shared_ptr<Coroutine>>& cos) {
        if (cos.size() > 0) {
            std::lock_guard<std::mutex> lock(_mutex);
            for (auto& item : cos) {
                _lst_ready.push_front(item);
            }
        }
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
    int     _max_co_size;

    int     _cur_co_size;
    int     _executor_count;

    std::atomic<bool>    _is_set_end;
    std::vector<std::shared_ptr<CoExecutor>>  _executors;

 //   std::vector<CoExecutor*>    _executors;
    std::vector<std::shared_ptr<Coroutine>>   _coroutines;

    std::mutex  _mutex;
    std::condition_variable _cv;

    std::function<void(int, const char*)>  _logger;

    int     _log_level;
};

#endif
