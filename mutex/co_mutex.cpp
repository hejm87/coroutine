#include <algorithm>
#include "co_mutex.h"
#include "../coroutine.h"
#include "../co_schedule.h"
#include "../common/helper.h"

using namespace std;

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

CoMutex::~CoMutex()
{
    printf("############## ~CoMutex\n");
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
        auto co = Singleton<CoSchedule>::get_instance()->get_running_co();
        if (_value.compare_exchange_strong(expect, 1)) {
            _lock_co = co;
            return ;
        }
        Singleton<CoSchedule>::get_instance()->suspend([this, &co, is_wake_up]() -> bool {
            // TODO 别的协程在这个时间点解锁的，这个协程就唤醒不了
            if (_value.load() == 0) {
                return false;
            }
            co->_suspend_status = CO_SUSPEND_LOCK;
            is_wake_up ? _block_list.push_front(co) : _block_list.push_back(co);
            return true;
        });
        is_wake_up = true;
    } while (1);    
}

//void CoMutex::unlock() {
//    if (_value.load() == 0) {
//        return ;
//    }
//    auto cur_co = Singleton<CoSchedule>::get_instance()->get_running_co();
//    if (_value.load() == 1 && _lock_co != cur_co) {
//        throw CoException(CO_ERROR_UNLOCK_EXCEPTION);
//    }
//    shared_ptr<Coroutine> resume_co;
//    if (!_block_list.is_empty()) {
//        _block_list.front(resume_co);
//        _block_list.pop_front();
//    }
//    _lock_co = nullptr;
//    _value.store(0);
//    if (resume_co) {
//        Singleton<CoSchedule>::get_instance()->resume(resume_co);
//    }
//}

void CoMutex::unlock() {
    if (_value.load() == 0) {
        return ;
    }
    auto cur_co = Singleton<CoSchedule>::get_instance()->get_running_co();
    if (_value.load() == 1 && _lock_co != cur_co) {
        throw CoException(CO_ERROR_UNLOCK_EXCEPTION);
    }

    Singleton<CoSchedule>::get_instance()->resume([this] {
        shared_ptr<Coroutine> co;
        if (_block_list.front(co)) {
            _block_list.pop_front();
        }
        _lock_co = nullptr;
        _value.store(0);
        return co;
    });
}