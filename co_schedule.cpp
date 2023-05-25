#include "co_executor.h"
#include "common/helper.h"
#include "co_common/co_awaiter.h"

using namespace std;

thread_local shared_ptr<CoExecutor> g_co_executor;

CoSchedule::CoSchedule()
{
    _stack_size = get_stack_size();

    _executor_count = get_executor_count();
    for (int i = 0; i < _executor_count; i++) {
        auto ptr = shared_ptr<CoExecutor>(new CoExecutor);
        _executors.push_back(ptr);
        ptr->run();
    }
    _timer.init(DEF_TIMER_THREAD_COUNT);
}

CoSchedule::~CoSchedule()
{
    _is_set_end = true;
    for (auto& item : _executors) {
        item->stop();
    }
}

void CoSchedule::create(const AnyFunc& func, bool priority)
{
    lock_guard<mutex> lock(_mutex);
    shared_ptr<Coroutine> co;
    if (!_lst_free.front(co)) {
        throw CoException(CO_ERROR_NO_RESOURCE);
    }
    _lst_free.pop_front();

    co->_func = func;
    co->_priority = priority;
    if (priority) {
        _lst_ready.push_front(co);
    } else {
        _lst_ready.push_back(co);
    }
}

CoAwaiter CoSchedule::create_with_promise(const AnyFunc& func, bool priority)
{
    lock_guard<mutex> lock(_mutex);
    shared_ptr<Coroutine> co;
    CoAwaiter awaiter;
    if (!_lst_free.front(co)) {
        throw CoException(CO_ERROR_NO_RESOURCE);
    }
    _lst_free.pop_front();
    if (is_in_co_thread()) {     // Э���߳�
        co->_func = AnyFunc([func, &awaiter] () {
            awaiter._wait_chan << func();
        });
        awaiter._is_call_on_co_thread = false;
    } else {    // ��Э���߳�
        promise<Any> p;
        co->_func = AnyFunc([func, &p] () {
            p.set_value(func());
        });
        awaiter._is_call_on_co_thread = true;
        awaiter._wait_future = p.get_future();
    }
    co->_priority = priority;
    if (priority) {
        _lst_ready.push_front(co);
    } else {
        _lst_ready.push_back(co);
    }
    return awaiter;
}

void CoSchedule::sleep(int sleep_ms)
{
    if (!g_co_executor) {
        throw CoException(CO_ERROR_NOT_IN_CO_THREAD);
    }
    auto co = g_co_executor->get_running_co();
    co->_suspend_type = CO_SUSPEND_SLEEP;
}

void CoSchedule::yield(function<void()> do_after)
{
    if (!g_co_executor) {
        throw CoException(CO_ERROR_NOT_IN_CO_THREAD);
    }
    g_co_executor->yield();
    if (do_after) {
        do_after();
    }
}

void CoSchedule::resume(shared_ptr<Coroutine> co)
{
    if (auto co_executor = co->_co_executor.lock()) {
        co_executor->resume(co);
    } else {
        throw CoException(CO_ERROR_INVALID_CO_EXECUTOR);
    }
}

CoTimerId CoSchedule::set_timer(int delay_ms, const AnyFunc& func)
{
    return _timer.set(delay_ms, [this, &func]() {
        create(func, true);
    });
}

bool CoSchedule::stop_timer(const CoTimerId& timer_id)
{
    _timer.cancel(timer_id);
}

vector<shared_ptr<Coroutine>> CoSchedule::get_global_co(int size)
{
    vector<shared_ptr<Coroutine>> cos;
    lock_guard<mutex> lock(_mutex);
    shared_ptr<Coroutine> co;
    if (_lst_ready.front(co)) {
        _lst_ready.pop_front();
        cos.push_back(co);
    }
}

shared_ptr<Coroutine> CoSchedule::get_cur_co()
{
    g_co_executor->get_running_co();
}
