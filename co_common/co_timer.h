#ifndef __CO_TIMER_H__
#define __CO_TIMER_H__

#include <map>
#include <unordered_map>
#include <vector>
#include <memory>

#include "../common/any_func.h"

class CoTimer
{
public:
    AnyFunc _func;
    int     _period_ms;
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
        return _lst_timer.size() > 0 ? true : false;
    }

    int size() {
        return (int)_lst_timer.size();
    }

    std::vector<AnyFunc> get_enable_timer();

    CoTimerId insert(AnyFunc func, int delay_ms, int period_ms);
    bool remove(const CoTimerId& timer_id);
    long get_next_time();

private:
    typedef std::multimap<long, std::shared_ptr<CoTimer>> lst_timer;

    lst_timer   _lst_timer;
    std::unordered_map<std::shared_ptr<CoTimer>, lst_timer::iterator>   _lst_timer_ptr;
};

#endif