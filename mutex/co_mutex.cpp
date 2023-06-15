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
        Singleton<CoSchedule>::get_instance()->suspend([this, &co, is_wake_up] {
            co->_suspend_status = CO_SUSPEND_LOCK;
            is_wake_up ? _block_list.push_front(co) : _block_list.push_back(co);
        });
       // if (is_wake_up) {
       //     printf(
       //         "############### CoMutex.lock, try, cur_co_id:%d, is_wake_up push_front, ptr:%p\n", 
       //         co->_id, 
       //         co.get()
       //     );
       //     _block_list.push_front(co);
       // } else {
       //     printf(
       //         "############### CoMutex.lock, try, cur_co_id:%d, is_not_wake_up push_back, ptr:%p\n", 
       //         co->_id, 
       //         co.get()
       //     );
       //     _block_list.push_back(co);
       // }
       // Singleton<CoSchedule>::get_instance()->suspend();
        is_wake_up = true;
    } while (1);    
}

void CoMutex::unlock() {
    if (_value.load() == 0) {
        return ;
    }
    auto cur_co = Singleton<CoSchedule>::get_instance()->get_running_co();
   // printf("################# [%s]CoMutex.unlock, try, cur_co_id:%d\n", date_ms().c_str(), cur_co->_id);
    if (_value.load() == 1 && _lock_co != cur_co) {
        throw CoException(CO_ERROR_UNLOCK_EXCEPTION);
    }
    shared_ptr<Coroutine> resume_co;
    if (!_block_list.is_empty()) {
       // printf("###############");
        _block_list.front(resume_co);
        _block_list.pop_front();
    }
    _lock_co = nullptr;
    _value.store(0);
   // printf("################# [%s]CoMutex.unlock, ok, cur_co_id:%d\n", date_ms().c_str(), cur_co->_id);
    if (resume_co) {
       // printf(
       //     "################# [%s]CoMutex.unlock, resume, cur_co_id:%d, resume_co_id:%d\n", 
       //     date_ms().c_str(), 
       //     cur_co->_id, 
       //     resume_co->_id
       // );
        Singleton<CoSchedule>::get_instance()->resume(resume_co);
    }
}