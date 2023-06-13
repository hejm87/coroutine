#ifndef __CO_COMMON_H__
#define __CO_COMMON_H__

#include <future>
#include "../channel/co_channel.h"
#include "../context/co_context.h"

class CoSchedule;

class CoEnv
{
public:
    static bool is_in_co_thread() {
        return g_ctx_main ? true : false;
    }

    static int get_executor_count() {
        int count;
        char* p = getenv("CO_EXECUTOR_COUNT");
        return p ? atoi(p) : sysconf(_SC_NPROCESSORS_ONLN);
    }

    static int get_stack_size() {
        char* p = getenv("CO_STACK_SIZE");
        return p ? atoi(p) : DEF_STACK_SIZE;
    }
};

template <typename T>
class CoAwaiter
{
friend class CoSchedule;
friend class CoApi;
public:
    void wait() {
        if (_wait_finish) {
            return ;
        }
        if (_wait_chan) {
            _wait_chan >> _result;
        } else {
            _wait_future.wait();
            _result = _wait_future.get();
        }
        _wait_finish = true;
    }

    T get() {
        wait();
        return _result;
    }

private:
    CoAwaiter(const CoAwaiter& obj) = delete;

    CoAwaiter() {
        _wait_in_co_thread = is_in_co_thread();
        if (_wait_in_co_thread) {
            _wait_future = _wait_promise.get_future();
        }
    }

    CoAwaiter(CoAwaiter&& obj) {
        swap_obj(std::forward<CoAwaiter>(obj));
    }

    CoAwaiter& operator=(CoAwaiter&& obj) {
        swap_obj(std::forward<CoAwaiter>(obj));
        return *this;
    }

    void set_value(const T& value) {
        if (_wait_finish) {
            return ;
        }
        if (_wait_in_co_thread) {
            _wait_chan << value;
        } else {
            _wait_promise.set_value(value);
        }
    }

    void swap_obj(CoAwaiter&& obj) {
        std::swap(_wait_promise, obj._wait_promise);
        std::swap(_wait_future, obj._wait_future);
        std::swap(_wait_chan, obj._wait_chan);
        std::swap(_result, obj._result);
        std::swap(_wait_in_co_thread, obj._wait_in_co_thread);
        std::swap(_wait_finish, obj._wait_finish);
    }

private:
    std::promise<T> _wait_promise;
    std::future<T>  _wait_future;
    CoChannel<T>    _wait_chan;
    T               _result;
    bool            _wait_in_co_thread;
    bool            _wait_finish;
};

template <>
class CoAwaiter<void>
{
friend class CoSchedule;
friend class CoApi;
public:
    void await() {
        if (_wait_finish) {
            return ;
        }
        if (_wait_in_co_thread) {
            int result;
            _wait_chan >> result;
        } else {
            _wait_future.wait();
        }
        _wait_finish = true;
    }

    void get() {;}

private:
    CoAwaiter(const CoAwaiter& obj) = delete;

    CoAwaiter() {
        _wait_in_co_thread = is_in_co_thread();
        if (_wait_in_co_thread) {
            _wait_future = _wait_promise.get_future();
        }
    }

    CoAwaiter(CoAwaiter&& obj) {
        swap_obj(std::forward<CoAwaiter>(obj));
    }

    CoAwaiter& operator=(CoAwaiter&& obj) {
        swap_obj(std::forward<CoAwaiter>(obj));
        return *this;
    }

    void set_value() {
        if (_wait_finish) {
            return ;
        }
        if (_wait_in_co_thread) {
            _wait_chan << 1; 
        } else {
            _wait_promise.set_value(1);
        }
    }

    void swap_obj(CoAwaiter&& obj) {
        std::swap(_wait_promise, obj._wait_promise);
        std::swap(_wait_future, obj._wait_future);
        std::swap(_wait_chan, obj._wait_chan);
        std::swap(_wait_in_co_thread, obj._wait_in_co_thread);
        std::swap(_wait_finish, obj._wait_finish);
    }

private:
    std::promise<int>   _wait_promise;
    std::future<int>    _wait_future;
    CoChannel<int>      _wait_chan;
    bool                _wait_in_co_thread;
    bool                _wait_finish;
};

#endif