#ifndef __CO_CHANNEL_H__
#define __CO_CHANNEL_H__

#include "../mutex/co_mutex.h"
#include "../co_exception.h"

template <class T>
class CoChannel
{
public:
    CoChannel(int size = 0);
    ~CoChannel();

    void operator>>(T& obj) throw CoException {
        _mutex.lock();
        if (!_closed) {
            throw CoException(CO_ERROR_CHANNEL_CLOSE);
        }
        if (_use_cache) {
            push_with_cache(obj);
        } else {
            push_without_cache(obj);
        }
    }

    void operator<<(const T& obj) throw CoException {
        ;
    }

    void push_with_cache(const T& obj) {
        _mutex.lock();
        if (_queue.get_max_size() > _queue.get_size()) {
            _queue.push_back(obj);
        } else {
            do {
                auto cur_co = CoSchedule::get_instance()->get_cur_co();
                _lst_send_waits.push_back(cur_co);
                CoSchedule::get_instance()->yield([_mutex]() {
                    _mutex.unlock();
                });
                // ??? 是否会出现channel关闭后唤醒
                _mutex.lock();
            } while (_queue.get_max_size() > _queue.get_size());
            _queue.push_back(obj);
        }

        shared_ptr<Coroutine> co;
        if (_lst_recv_waits.pop_front(co)) {
            CoSchedule::get_instance()->resume(co);
        }
        _mutex.unlock();
    }

    void push_without_cache(const T& obj) throw CoException {
        _mutex.lock();
        if (_lst_recv_waits.size() > 0) {
            shared_ptr<Coroutine> co;
            _lst_recv_waits.pop_front(co);
            co->param = obj;
            _mutex.unlock();
            CoSchedule::get_instance()->resume(co);
        } else {
            auto cur_co = CoSchedule::get_instance()->get_cur_co();
            cur_co->_param.type = CO_PARAM_CHANNEL_SEND;
            cur_co->_param.value = obj;
            _lst_send_waits.push_back(cur_co);
            CoSchedule::get_instance()->yield([_mutex]() {
                _mutex.unlock();
            });
            if (_closed) {
                _mutex.unlock();
                throw CoException(CO_ERROR_CHANNEL_CLOSE);
            }
            _mutex.unlock();
            cur_co->_param.type = CO_PARAM_NIL;
            cur_co->_param.value.Reset();
        }
    }

    bool pop_with_cache(T& obj) {
        _mutex.lock();
        if (_queue.size() > 0) {
            _queue.pop_front(obj);
        } else {
            do {
                auto cur_co = CoSchedule::get_instance()->get_cur_co();
                _lst_recv_waits.push_back(cur_co);
                CoSchedule::get_instance()->yield([_mutex]() {
                    _mutex.unlock();
                });
                _mutex.lock();
            } while (_queue.size() == 0);
        }

        shared_ptr<Coroutine> co;
        if (_list_send_waits.pop_front(co)) {
            CoSchedule::get_instance()->resume(co);
        }
        _mutex.unlock();
    }

    bool pop_without_cache(T& obj) {
        _mutex.lock();
        if (_lst_send_waits.size() > 0) {
            shared_ptr<Coroutine> co;
            _lst_send_waits.pop_front(co);
            if (co._param.type != CO_PARAM_CHANNEL_SEND) {
                throw CoException(CO_ERROR_UNKNOW);
            }
            obj = co._param.value;
            _mutex.unlock();
        } else {
            auto cur_co = CoSchedule::get_instance()->get_cur_co();
            cur_co->_param.type = CO_PARAM_CHANNEL_RECV;
            cur_co->_param.value.Reset();
            _lst_recv_waits.push_back(cur_co);
            CoSchedule::get_instance()->yield([_mutex]() {
                _mutex.unlock();
            });
            if (_closed) {
                _mutex.unlock();
                throw CoException(CO_ERROR_CHANNEL_CLOSE);
            }
            _mutex.unlock();
            if (cur_co->_param.value.IsNull()) {
                throw CoException(CO_ERROR_UNKNOW);
            }
            obj = cur_co->_param.value;
            cur_co->_param.value.Reset();
        }
    }

private:
    bool    _closed;
    bool    _use_cache;

    CoList  _lst_send_waits;
    CoList  _lst_recv_waits;

    RingQueue<T>    _queue;

    CoMutex _mutex;
};

#endif