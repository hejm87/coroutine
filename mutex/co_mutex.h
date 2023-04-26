#ifndef __CO_MUTEX_H__
#define __CO_MUTEX_H__

#include <atomic>
#include "../co_common/co_list.h"

class CoMutex
{
public:
    CoMutex() {
        _value = 0;
    }

    void lock() {
        auto is_wake_up = false;
        do {
            if (_value.compare_exchange_strong(0, 1)) {
                return ;
            }
            auto co = CoSchendule::get_instance()->get_cur_co();
            if (is_wake_up) {
                _block_list.push_back(co);
            } else {
                _block_list.push_front(co);
            } 
            CoSchedule::get_instance()->yield();
        } while (1);
    }

    void unlock() {
        _value = 0; 
        if (_block)

        if (_block_list.size() > 0) {
            auto co = _block_list.front();
            _block_list.pop_front();
            CoSchedule::get_instance()->resume(co);
        }
        _value = 0;
    }

private:
    std::atomic<int>    _value;
    CoList              _block_list;
};

#endif