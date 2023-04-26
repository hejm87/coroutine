#ifndef __COROUTINE_DEFINE_H__
#define __COROUTINE_DEFINE_H__

#include <memory>
#include <future>
#include "common/any.h"
#include "channel/co_channel.h"
#include "co_common/co_timer.h"

// Ĭ��Э��ջ��С
const int DEF_STACK_SIZE = 1024 * 4;

// Ĭ��ÿ����ȫ�ֶ��л�ȡ��ִ��Э����
const int DEF_GET_GLOBAL_CO_SIZE = 5;

// Э��ͬ���ȴ����
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
    bool    _external_thread;   // �ⲿ�̵߳��ã���Э���̣߳�
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
