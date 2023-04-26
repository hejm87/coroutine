#ifndef __COROUTINE_DEFINE_H__
#define __COROUTINE_DEFINE_H__

#include <memory>
#include <future>
#include "common/any.h"
#include "channel/co_channel.h"
#include "co_common/co_timer.h"

// 默认协程栈大小
const int DEF_STACK_SIZE = 1024 * 4;

// 默认每次向全局队列获取可执行协程数
const int DEF_GET_GLOBAL_CO_SIZE = 5;

// 协程同步等待组件
class CoAwaiter
{
public:
    template <typename T>
    T await() {
        Any obj;
        if (_external_thread) {
            obj = _wait_future.wait();
        } else {
            _wait_chan >> obj;
        }
        return obj.AnyCast<T>();
    }

private:
    bool    _external_thread;   // 外部线程调用（非协程线程）
    std::future<Any>    _wait_future;
    CoChannel<Any>      _wait_chan;
};

class CoTimerId
{
friend class CoSchedule;
friend class CoTimerList;
private:
    std::weak_ptr<CoTimer>   _ptr;
};

#endif
