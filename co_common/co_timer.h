#ifndef __CO_TIMER_H__
#define __CO_TIMER_H__

#include <map>
#include <unordered_map>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <thread>

#include "../common/any_func.h"

class CoTimer
{
public:
    AnyFunc _func;
};

class CoTimerId
{
friend class CoTimerList;
private:
    std::weak_ptr<CoTimer>   _ptr;
};

class CoTimerList
{
public:
    bool is_empty() {
        return _lst_timer.size() == 0 ? true : false;
    }

    int size() {
        return (int)_lst_timer.size();
    }

    std::vector<AnyFunc> get_enable_timer();

    CoTimerId insert(AnyFunc func, int delay_ms);
    bool remove(const CoTimerId& timer_id);
    long get_next_time();

private:
    typedef std::multimap<long, std::shared_ptr<CoTimer>> LIST_TIMER;

    LIST_TIMER  _lst_timer;
    std::unordered_map<std::shared_ptr<CoTimer>, LIST_TIMER::iterator>  _lst_timer_iter;
};

// ###################################################
struct CoTimerRunner
{
    AnyFunc func;
};

class CoTimerId
{
friend class CoTimer;
private:
    std::weak_ptr<CoTimerRunner>    _ptr;
};

class CoTimer
{
public:
    CoTimer* get_instance() {
        static CoTimer s_instance;
        return &s_instance;
    }

    CoTimerId insert(AnyFunc func, int delay_ms);

    bool remove(const CoTimerId& id);

private:
    CoTimer();
    ~CoTimer();

    void run();

private:
    typedef std::multimap<long, std::shared_ptr<CoTimerRunner>> LIST_TIMER;

    LIST_TIMER  _list;
    std::map<std::shared_ptr<CoTimerRunner>, LIST_TIMER::iterator>  _list_timer_iter;

    std::mutex  _mutex;
    std::thread _thread;

    std::atomic<bool>   _terminate;
};

#endif