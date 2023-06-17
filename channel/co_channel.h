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
        printf("############## CoChannel construct\n");
        _closed = false;
        if (size > 0) {
            printf("############## CoChannel construct, with cache\n");
            _use_cache = true;
            _queue = new RingQueue<T>(size);
        } else if (size == 0) {
            printf("############## CoChannel construct, without cache\n");
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
        close();
    }

    void close() {
        if (_closed) {
            return ;
        }
        _closed = true;
        delete _queue;
        _queue = NULL;
    }

    void operator>>(T& obj) {
        {
            _mutex.lock();
            if (_closed) {
                throw CoException(CO_ERROR_CHANNEL_CLOSE);
            }
            _mutex.unlock();
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
            if (_closed) {
                throw CoException(CO_ERROR_CHANNEL_CLOSE);
            }
            _mutex.unlock();
        }
        if (_use_cache) {
            push_with_cache(obj);
        } else {
            push_without_cache(obj);
        }
    }

    void push_with_cache(const T& obj) {
        auto cur_co = Singleton<CoSchedule>::get_instance()->get_running_co();
        printf("[%s] channel.push_with_cache, cid:%d\n", date_ms().c_str(), cur_co->_id);
        _mutex.lock();
        if (_queue->size() > _queue->cur_size()) {
            printf("[%s] channel.push_with_cache, cid:%d, is ok\n", date_ms().c_str(), cur_co->_id);
            _queue->push(obj);
        } else {
            printf("[%s] channel.push_with_cache, cid:%d, no space\n", date_ms().c_str(), cur_co->_id);
            do {
                cur_co->_suspend_status = CO_SUSPEND_CHANNEL;
                _lst_send_waits.push_back(cur_co);
                Singleton<CoSchedule>::get_instance()->suspend([this, cur_co] {
                    printf("[%s] channel.push_with_cache, cid:%d, no space, suspend\n", date_ms().c_str(), cur_co->_id);
                    _mutex.unlock();
                    return true;
                });
                // ??? �?否会出现channel关闭后唤�?
                _mutex.lock();
                printf("[%s] channel.push_with_cache, cid:%d, no space, awake\n", date_ms().c_str(), cur_co->_id);
            } while (_queue->size() > _queue->cur_size());
            _queue->push(obj);
        }

        shared_ptr<Coroutine> co;
        if (_lst_recv_waits.front(co)) {
            _lst_recv_waits.pop_front();
            printf("[%s] channel.push_with_cache, resume, cid:%d, resume_cid:%d\n", date_ms().c_str(), cur_co->_id, co->_id);
            Singleton<CoSchedule>::get_instance()->resume(co);
        }
        _mutex.unlock();
    }

    void push_without_cache(const T& obj) {
        auto cur_co = Singleton<CoSchedule>::get_instance()->get_running_co();
        std::shared_ptr<Coroutine> co;
        _mutex.lock();
        printf("[%s] channel.push_without_cache, cid:%d\n", date_ms().c_str(), cur_co->_id);
        if (_lst_recv_waits.front(co)) {
            printf("[%s] channel.push_without_cache, resume, cid:%d, resume_cid:%d\n", date_ms().c_str(), cur_co->_id, co->_id);
            _lst_recv_waits.pop_front();
            co->_param.type = CO_PARAM_CHANNEL_RECV;
            co->_param.value = obj;
            _mutex.unlock();
            Singleton<CoSchedule>::get_instance()->resume(co);
        } else {
            printf("[%s] channel.push_without_cache, cid:%d, no waiter, suspend\n", date_ms().c_str(), cur_co->_id);
            cur_co->_suspend_status = CO_SUSPEND_CHANNEL;
            cur_co->_param.type = CO_PARAM_CHANNEL_SEND;
            cur_co->_param.value = obj;
            printf("[%s] channel.push_without_cache, cid:%d, no waiter, suspend1\n", date_ms().c_str(), cur_co->_id);
            _lst_send_waits.push_back(cur_co);
            printf("[%s] channel.push_without_cache, cid:%d, no waiter, suspend2\n", date_ms().c_str(), cur_co->_id);
            Singleton<CoSchedule>::get_instance()->suspend([this, cur_co] {
                printf("[%s] channel.push_without_cache, cid:%d, unlock1\n", date_ms().c_str(), cur_co->_id);
                _mutex.unlock();
                printf("[%s] channel.push_without_cache, cid:%d, unlock2\n", date_ms().c_str(), cur_co->_id);
                return true;
            });
            printf("[%s] channel.push_without_cache, cid:%d, awake\n", date_ms().c_str(), cur_co->_id);
            _mutex.lock();
            if (_closed) {
                _mutex.unlock();
                throw CoException(CO_ERROR_CHANNEL_CLOSE);
            }
            _mutex.unlock();
            cur_co->_param.type = CO_PARAM_NIL;
            cur_co->_param.value.Reset();
        }
    }

    void pop_with_cache(T& obj) {
        auto cur_co = Singleton<CoSchedule>::get_instance()->get_running_co();
        _mutex.lock();
        if (_queue->front(obj)) {
            printf("[%s] channel.pop_with_cache, cid:%d, get obj\n", date_ms().c_str(), cur_co->_id);
            _queue->pop();
        } else {
            printf("[%s] channel.pop_with_cache, cid:%d, get no obj, ready to suspend\n", date_ms().c_str(), cur_co->_id);
            do {
                cur_co->_suspend_status = CO_SUSPEND_CHANNEL;
                _lst_recv_waits.push_back(cur_co);
                Singleton<CoSchedule>::get_instance()->suspend([this, cur_co] {
                    printf("[%s] channel.pop_with_cache, cid:%d, is suspend\n", date_ms().c_str(), cur_co->_id);
                    _mutex.unlock();
                    return true;
                });
                _mutex.lock();
                printf("[%s] channel.pop_with_cache, cid:%d, awake\n", date_ms().c_str(), cur_co->_id);
            } while (_queue->size() == 0);
            printf("[%s] channel.pop_with_cache, cid:%d, awake, get obj\n", date_ms().c_str(), cur_co->_id);
        }
        shared_ptr<Coroutine> co;
        if (_lst_send_waits.front(co)) {
            printf("[%s] channel.pop_with_cache, resume, cid:%d, resume_cid:%d", date_ms().c_str(), cur_co->_id, co->_id);
            _lst_send_waits.pop_front();
            Singleton<CoSchedule>::get_instance()->resume(co);
        }
        _mutex.unlock();
    }

    bool pop_without_cache(T& obj) {
        shared_ptr<Coroutine> co;
        auto cur_co = Singleton<CoSchedule>::get_instance()->get_running_co();
        _mutex.lock();
        if (_lst_send_waits.front(co)) {
            printf("[%s] channel.pop_without_cache, cid:%d, get obj\n", date_ms().c_str(), cur_co->_id);
            _lst_send_waits.pop_front();
            if (co->_param.type != CO_PARAM_CHANNEL_SEND) {
                throw CoException(CO_ERROR_UNKNOW);
            }
            obj = co->_param.value.AnyCast<T>();
            Singleton<CoSchedule>::get_instance()->resume([this, cur_co, co] {
                _mutex.unlock();
                printf("[%s] channel.pop_without_cache, cid:%d, get obj & resume, resume_cid:%d", date_ms().c_str(), cur_co->_id, co->_id);
                return co;
            });
        } else {
            printf("[%s] channel.pop_without_cache, cid:%d, get no obj\n", date_ms().c_str(), cur_co->_id);
            cur_co->_param.type = CO_PARAM_CHANNEL_RECV;
            cur_co->_param.value.Reset();
            _lst_recv_waits.push_back(cur_co);
            Singleton<CoSchedule>::get_instance()->suspend([this, cur_co] {
                printf("[%s] channel.pop_without_cache, cid:%d, suspend\n", date_ms().c_str(), cur_co->_id);
                _mutex.unlock();
                return true;
            });
            if (_closed) {
                _mutex.unlock();
                throw CoException(CO_ERROR_CHANNEL_CLOSE);
            }
            _mutex.unlock();
            if (cur_co->_param.value.IsNull()) {
                throw CoException(CO_ERROR_UNKNOW);
            }
            printf("[%s] channel.pop_without_cache, cid:%d, awake, get obj\n", date_ms().c_str(), cur_co->_id);
            obj = cur_co->_param.value.AnyCast<T>();
            cur_co->_param.value.Reset();
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
