#ifndef __CO_CHANNEL_H__
#define __CO_CHANNEL_H__

#include <algorithm>
#include "../coroutine.h"
#include "../co_define.h"
#include "../co_schedule.h"
#include "../co_exception.h"
#include "../common/helper.h"
#include "../common/ring_queue.h"
#include "../mutex/co_mutex.h"

template <class T>
class CoChannel
{
public:
    CoChannel(int size = 0) {
        _closed = false;
        if (size > 0) {
            _use_cache = true;
            _queue = new RingQueue<T>(size);
        } else if (size == 0) {
            _use_cache = false;
            _queue = NULL;
        } else {
            throw CoException(CO_ERROR_PARAM_INVALID);
        }
    }

    CoChannel(CoChannel&& obj) {
        std::swap(_closed, obj._closed);
        std::swap(_use_cache, obj._use_cache);
        std::swap(_lst_send_waits, obj._lst_send_waits);
        std::swap(_lst_recv_waits, obj._lst_recv_waits);
        std::swap(_queue, obj._queue);
        std::swap(_mutex, obj._mutex);
    }

    CoChannel& operator=(CoChannel&& obj) {
        std::swap(_closed, obj._closed);
        std::swap(_use_cache, obj._use_cache);
        std::swap(_lst_send_waits, obj._lst_send_waits);
        std::swap(_lst_recv_waits, obj._lst_recv_waits);
        std::swap(_queue, obj._queue);
        std::swap(_mutex, obj._mutex);
        return *this;
    }

    ~CoChannel() {
        if (_queue) {
            delete _queue;
        }
    }

    void operator>>(T& obj) {
        {
            _mutex.lock();
            if (!_closed) {
                throw CoException(CO_ERROR_CHANNEL_CLOSE);
            }
        }
        if (_use_cache) {
            pop_with_cache(obj);
        } else {
            pop_without_cache(obj);
        }
    }

    void operator<<(const T& obj) {
        {
            _mutex.lock();
            if (!_closed) {
                throw CoException(CO_ERROR_CHANNEL_CLOSE);
            }
        }
        if (_use_cache) {
            push_with_cache(obj);
        } else {
            push_without_cache(obj);
        }
    }

    void push_with_cache(const T& obj) {
        _mutex.lock();
        if (_queue->size() > _queue->cur_size()) {
            _queue->push(obj);
        } else {
            do {
                auto cur_co = Singleton<CoSchedule>::get_instance()->get_running_co();
                cur_co->_suspend_status = CO_SUSPEND_CHANNEL;
                _lst_send_waits.push_back(cur_co);
                Singleton<CoSchedule>::get_instance()->yield([this]() {
                    _mutex.unlock();
                });
                // ??? �?否会出现channel关闭后唤�?
                _mutex.lock();
            } while (_queue->size() > _queue->cur_size());
            _queue->push(obj);
        }

        shared_ptr<Coroutine> co;
        if (_lst_recv_waits.front(co)) {
            _lst_recv_waits.pop_front();
            Singleton<CoSchedule>::get_instance()->resume(co);
        }
        _mutex.unlock();
    }

    void push_without_cache(const T& obj) {
        std::shared_ptr<Coroutine> co;
        _mutex.lock();
        if (_lst_recv_waits.front(co)) {
            _lst_recv_waits.pop_front();
            co->_param.type = CO_PARAM_CHANNEL_RECV;
            co->_param.value = obj;
            _mutex.unlock();
            Singleton<CoSchedule>::get_instance()->resume(co);
        } else {
            Singleton<CoSchedule>::get_instance()->resume(co);
            co->_suspend_status = CO_SUSPEND_CHANNEL;
            co->_param.type = CO_PARAM_CHANNEL_SEND;
            co->_param.value = obj;
            _lst_send_waits.push_back(co);
            Singleton<CoSchedule>::get_instance()->yield([this]() {
                _mutex.unlock();
            });
            if (_closed) {
                _mutex.unlock();
                throw CoException(CO_ERROR_CHANNEL_CLOSE);
            }
            _mutex.unlock();
            co->_param.type = CO_PARAM_NIL;
            co->_param.value.Reset();
        }
    }

    void pop_with_cache(T& obj) {
        _mutex.lock();
        if (_queue->front(obj)) {
            _queue->pop();
        } else {
            do {
                auto cur_co = Singleton<CoSchedule>::get_instance()->get_running_co();
                cur_co->_suspend_status = CO_SUSPEND_CHANNEL;
                _lst_recv_waits.push_back(cur_co);
                Singleton<CoSchedule>::get_instance()->yield([this]() {
                    _mutex.unlock();
                });
                _mutex.lock();
            } while (_queue->size() == 0);
        }
        shared_ptr<Coroutine> co;
        if (_lst_send_waits.front(co)) {
            _lst_send_waits.pop_front();
            Singleton<CoSchedule>::get_instance()->resume(co);
        }
        _mutex.unlock();
    }

    bool pop_without_cache(T& obj) {
        shared_ptr<Coroutine> co;
        _mutex.lock();
        if (_lst_send_waits.front(co) > 0) {
            _lst_send_waits.pop_front();
            if (co->_param.type != CO_PARAM_CHANNEL_SEND) {
                throw CoException(CO_ERROR_UNKNOW);
            }
            obj = co->_param.value.AnyCast<T>();
            _mutex.unlock();
        } else {
            co = Singleton<CoSchedule>::get_instance()->get_running_co();
            co->_param.type = CO_PARAM_CHANNEL_RECV;
            co->_param.value.Reset();
            _lst_recv_waits.push_back(co);
            Singleton<CoSchedule>::get_instance()->yield([this]() {
                _mutex.unlock();
            });
            if (_closed) {
                _mutex.unlock();
                throw CoException(CO_ERROR_CHANNEL_CLOSE);
            }
            _mutex.unlock();
            if (co->_param.value.IsNull()) {
                throw CoException(CO_ERROR_UNKNOW);
            }
            obj = co->_param.value.AnyCast<T>();
            co->_param.value.Reset();
        }
        return true;
    }

private:
    bool    _closed;
    bool    _use_cache;

    CoList  _lst_send_waits;
    CoList  _lst_recv_waits;

    RingQueue<T>    *_queue;

    CoMutex _mutex;
};

#endif
