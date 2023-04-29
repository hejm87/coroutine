#ifndef __CO_DEFINE_H__
#define __CO_DEFINE_H__

#include <future>
#include "common/any.h"
#include "channel/co_channel.h"
#include "co_common.h"

// 等待协程完成
class CoAwaiter
{
public:
    template <typename T>
    T await() {
        Any obj;
        if (is_in_co_thread() == true) {
            obj = _wait_future.wait();
        } else {
            _wait_chan >> obj;
        }
        return obj.AnyCast<T>();
    }

private:
    std::future<Any>    _wait_future;
    CoChannel<Any>      _wait_chan;
};

#endif
