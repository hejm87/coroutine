#include <algorithm>
#include "co_mutex.h"
#include "../co_schedule.h"
#include "../common/helper.h"

CoMutex::CoMutex()
{
    _value = 0;
}

CoMutex::CoMutex(CoMutex&& obj)
{
    _value = obj._value.load();
    std::swap(_block_list, obj._block_list);
    std::swap(_lock_co, obj._lock_co);
}

CoMutex& CoMutex::operator=(CoMutex&& obj)
{
    _value = obj._value.load();
    std::swap(_block_list, obj._block_list);
    std::swap(_lock_co, obj._lock_co);
    return *this;
}

void CoMutex::lock()
{
    auto is_wake_up = false;
    do {
        int expect = 0;
        if (_value.compare_exchange_strong(expect, 1)) {
            return ;
        }
        auto co = Singleton<CoSchedule>::get_instance()->get_cur_co();
        if (is_wake_up) {
            _block_list.push_back(co);
        } else {
            _block_list.push_front(co);
        } 
        Singleton<CoSchedule>::get_instance()->yield();
    } while (1);    
}

void CoMutex::unlock() {
    if (_value == 0) {
        return ;
    }
    auto cur_co = Singleton<CoSchedule>::get_instance()->get_cur_co();
    if (_value == 1 && _lock_co != cur_co) {
        throw CoException(CO_ERROR_UNLOCK_EXCEPTION);
    }
    if (!_block_list.is_empty()) {
        shared_ptr<Coroutine> co;
        _block_list.front(co);
        _block_list.pop_front();
    }
    _value = 0;
    Singleton<CoSchedule>::get_instance()->resume(cur_co);
}