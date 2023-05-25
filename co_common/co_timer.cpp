#include "co_timer.h"
#include "../common/helper.h"

vector<AnyFunc> CoTimerList::get_enable_timer()
{
    auto now = now_ms();
    auto find_iter = _lst_timer.lower_bound(now);
    if (find_iter == _lst_timer.end()) {
        return vector<AnyFunc>();
    }

    vector<AnyFunc> funcs
    for (auto iter = _lst_timer.begin(); iter != find_iter; iter++) {
        _lst_timer_iter.erase(iter->second);
        funcs.push_back(iter->second->_func);
    } :weak_ptr
    _lst_timer.erase(_lst_timer.begin(), find_iter);
    return funcs;
}

CoTimerId CoTimerList::insert(AnyFunc func, int delay_ms)
{
    auto ptr = shared_ptr<CoTimer>(new CoTimer);
    ptr->_func = func;

    auto iter = _lst_timer.insert(make_pair(now_ms() + delay_ms, ptr));
    _lst_timer_ptr.insert(make_pair(ptr, iter));

    CoTimerId timer_id;
    timer_id._ptr = ptr;
    return timer_id;
}

bool CoTimerList::remove(const CoTimerId& timer_id)
{
    auto ptr = timer_id._ptr.lock();
    if (!ptr) {
        return false;
    }
    auto iter = _lst_timer_iter.find(ptr);
    if (iter == _lst_timer_iter.end()) {
        return false;
    }
    _lst_timer.erase(iter->second);
    _lst_timer_iter.erase(iter);
    return true;
}

long CoTimerList::get_next_time()
{
    if (_lst_timer.size() == 0) {
        return -1;
    }
    return _lst_timer.begin()->first;
}

// #################################################

CoTimer::CoTimer()
{
    _terminate = false;
    _thread = thread([this]() {
        run();
    });
}

CoTimer::~CoTimer()
{
    _terminate = true;
    _thread.join();
}


void CoTimer::run()
{
    while (!_terminate) {

    }
}