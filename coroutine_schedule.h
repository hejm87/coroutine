#ifndef __COROUTINE_SCHEDULE_H__
#define __COROUTINE_SCHEDULE_H__

#include <mutex>
#include <exception>
#include "coroutine_common.h"

using namespace std;

class CoroutineSchedule
{
friend class CoroutineExecutor;
public:
    CoroutineSchedule* get_instance() {
        static CoroutineSchedule instance;
        return &instance;
    }

    void create(function<Any()> func) {
        xxxx ;
    }

    bool await(function<Any()> func, int timeout_ms) {

    }

    void yield() {
        xxxx
    }

    bool set_stack_size(int size) {
        _stack_size = size;
    }

    int get_stack_size() {
        return _stack_size;
    }

    bool is_running() {
        return _is_running;
    }

private:
    CoroutineSchedule();
    ~CoroutineSchedule();

    coroutine_id create(shared_ptr<AnyFunction>& func);

private:
    vector<shared_ptr<Coroutine>>   _coroutines;
    shared_ptr<Coroutine>           _list_free;

    bool    _is_running;
    int     _stack_size;

    mutex   _mutex;
};

#endif