#ifndef __CO_TIMER_LIST_H__
#define __CO_TIMER_LIST_H__

#include <map>
#include <unordered_map>
#include <vector>
#include <memory>

#include "../co_define.h"
#include "../common/any_func.h"

class CoTimer
{
public:
    AnyFunc _func;
    int     _period_ms;
};

class CoTimerId
{
private:
    std::weak_ptr<CoTimer>   _ptr;
};

class CoTimerList
{
public:
    bool is_empty() {
        return _lst_timer.size() > 0 ? true : false;
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