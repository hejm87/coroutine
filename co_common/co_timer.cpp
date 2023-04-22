#include "co_timer.h"

vector<AnyFunc> CoTimerList::get_enable_timer()
{
    vector<AnyFunc> funcs;

    auto now = now_ms();
    auto find_iter = _lst_timer.lower_bound(now);
    if (find_iter == _lst_timer.end()) {
        return funcs;
    }

    vector<shared_ptr<CoTimer>> timers;
    for (auto iter = _lst_timer.begin(); iter != find_iter; iter++) {
        timers.push_back(iter->second);    
        _lst_timer_ptr.erase(iter->second);
    }
    _lst_timer.erase(_lst_timer.begin(), find_iter);

    for (auto& item : timers) {
        if (item->_period_ms <= 0) {
            continue ;
        }
        auto iter = _lst_timer.insert(make_pair(now_ms() + period_ms, item));
        _lst_timer_ptr.insert(make_pair(item, iter));
    }
    return funcs;
}

CoTimerId CoTimerList::insert(AnyFunc func, int delay_ms, int period_ms)
{
    auto ptr = shared_ptr<CoTimer>(new CoTimer);
    ptr->_func = func;
    ptr->period_ms = period_ms;

    auto iter = _lst_timer.insert(make_pair(now_ms() + delay_ms, ptr));
    _lst_timer_ptr.insert(make_pair(ptr, iter));

    CoTimerId timer_id;
    timer_id._ptr = ptr;
    return timer_id;
}

bool CoTimerList::remove(const CoTimerId& timer_id)
{
    auto ret = false;
    if (auto ptr = timer._ptr.lock()) {
        auto iter = _lst_timer_ptr.find(ptr);
        if (iter != _lst_timer_ptr.end()) {
            _lst_timer.erase(iter);
            _lst_timer_ptr.erase(ptr);
            ret = true;
        }
    }
    return ret;
}

long CoTimerList::get_next_time()
{
    if (_lst_timer.size() == 0) {
        return -1;
    }
    return _lst_timer.begin()->first;
}