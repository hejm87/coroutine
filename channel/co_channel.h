#ifndef __CO_CHANNEL_H__
#define __CO_CHANNEL_H__

#include "../common/ring_queue.h"
#include "../mutex/co_mutex.h"
#include "../co_common/co_list.h"
#include "../co_schedule.h"
#include "../co_exception.h"

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

    ~CoChannel() {
        if (_queue) {
            delete _queue;
        }
    }

    void operator>>(T& obj) throw(CoException) {
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

    void operator<<(const T& obj) throw(CoException) {
        {
            _mutex.lock();
            if (!_closed) {
                throw CoException(CO_ERROR_CHANNEL_CLOSE);
            }
        }
        if (_use_cache) {
            _queue->pop_with_cache(obj);
        } else {
            _queue->pop_without_cache(obj);
        }
    }

    void push_with_cache(const T& obj) {
        _mutex.lock();
        if (_queue->get_max_size() > _queue->get_size()) {
            _queue->push_back(obj);
        } else {
            do {
                auto cur_co = CoSchedule::get_instance()->get_cur_co();
                _lst_send_waits.push_back(cur_co);
                CoSchedule::get_instance()->yield([this]() {
                    _mutex.unlock();
                });
                // ??? 是否会出现channel关闭后唤醒
                _mutex.lock();
            } while (_queue->get_max_size() > _queue->get_size());
            _queue->push_back(obj);
        }

        shared_ptr<Coroutine> co;
        if (_lst_recv_waits.front(co)) {
            _lst_recv_waits.pop_front();
            CoSchedule::get_instance()->resume(co);
        }
        _mutex.unlock();
    }

    void push_without_cache(const T& obj) throw(CoException) {
        std::shared_ptr<Coroutine> co;
        _mutex.lock();
        if (_lst_recv_waits.front(co)) {
            _lst_recv_waits.pop_front();
            co->_param = obj;
            _mutex.unlock();
            CoSchedule::get_instance()->resume(co);
        } else {
            co = CoSchedule::get_instance()->get_cur_co();
            co->_param.type = CO_PARAM_CHANNEL_SEND;
            co->_param.value = obj;
            _lst_send_waits.push_back(co);
            CoSchedule::get_instance()->yield([this]() {
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
            _queue->pop_front();
        } else {
            do {
                auto cur_co = CoSchedule::get_instance()->get_cur_co();
                _lst_recv_waits.push_back(cur_co);
                CoSchedule::get_instance()->yield([this]() {
                    _mutex.unlock();
                });
                _mutex.lock();
            } while (_queue->size() == 0);
        }
        shared_ptr<Coroutine> co;
        if (_lst_send_waits.front(co)) {
            _lst_send_waits.pop_front();
            CoSchedule::get_instance()->resume(co);
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
            obj = co->_param.value;
            _mutex.unlock();
        } else {
            co = CoSchedule::get_instance()->get_cur_co();
            co->_param.type = CO_PARAM_CHANNEL_RECV;
            co->_param.value.Reset();
            _lst_recv_waits.push_back(co);
            CoSchedule::get_instance()->yield([this]() {
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
            obj = co->_param.value;
            co->_param.value.Reset();
        }
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
