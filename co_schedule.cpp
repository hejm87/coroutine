#include "co_schedule.h"

using namespace std;

CoSchedule::CoSchedule()
{
    _is_running = false;
    _stack_size = get_stack_size();

    _executor_count = get_executor_count();
    for (int i = 0; i < _executor_count; i++) {
        auto ptr = shared_ptr<CoExecutor>(new CoExecutor);
        _executors.push_back(ptr);
        ptr->run();
    }

    _timer_thread = thread([this](){
        run_timer();
    });
}

CoSchedule::~CoSchedule()
{
    _is_set_end = true;
    for (auto& item : _executors) {
        item->wait();
    }
    _timer_thread.join();
}

void CoSchedule::create(const AnyFunc& func, bool priority) throw CoException
{
    lock_guard<mutex> lock(_mutex);
    shared_ptr<Coroutine> co;
    if (!_lst_free.pop_front(co)) {
        throw CoException(CO_ERROR_NO_RESOURCE);
    }
    co->_func = func;
    co->_priority = priority;
    if (priority) {
        _lst_ready.push_front(co);
    } else {
        _lst_ready.push_back(co);
    }
}

CoAwaiter CoSchedule::create_with_promise(const AnyFunc& func, bool priority) throw CoException
{
    lock_guard<mutex> lock(_mutex);
    shared_ptr<Coroutine> co;
    CoAwaiter awaiter;
    if (!_lst_free.pop_front(co)) {
        throw CoException(CO_ERROR_NO_RESOURCE);
    }
    if (check_in_co_thread()) {     // Э���߳�
        CoChannel<Any> chan();
        co->_func = AnyFunc([func, chan] () {
            chan << func();
        });
        awaiter._external_thread = false;
        awaiter._wait_chan = chann;
    } else {    // ��Э���߳�
        promise<Any> p;
        co->_func = AnyFunc([func, p] () {
            p.set_value(func());
        });
        awaiter._external_thread = true;
        awaiter._wait_future = p.get_future;
    }
    co->_priority = priority;
    if (priority) {
        _lst_ready.push_front(co);
    } else {
        _lst_ready.push_back(co);
    }
    return awaiter;
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

CoTimerId CoSchedule::set_timer(const AnyFunc& func, int delay_ms, int period_ms)
{
    CoTimerId timer_id;
    {
        lock_guard<mutex> lock(_mutex);
        auto notify = false;
        auto happen_ms = now_ms() + delay_ms;
        if (_lst_timer.size() > 0 && _lst_timer.get_next_time() > happen_ms) {
            notify = true;
        }
        timer_id = _lst_timer.insert(func, delay_ms, period_ms);
    }
    _cv.notify();
    return timer_id;
}

bool CoSchedule::stop_timer(const CoTimerId& timer_id)
{
    lock_guard<mutex> lock(_mutex);
    return _lst_timer.remove(timer_id);
}

vector<shared_ptr<Coroutine>> CoSchedule::get_global_co(int size)
{
    vector<shared_ptr<Coroutine>> cos;
    lock_guard<mutex> lock(_mutex);
    shared_ptr<Coroutine> co;
    if (_lst_ready.pop_front(co)) {
        cos.push_back(co);
    }
}

void CoSchedule::run_timer()
{
    while (!_is_set_end) {

        int wait_ms = -1;
        {
            lock_guard<mutex> lock(_mutex);
            if (_lst_timer.size() > 0) {
                auto delta_ms = _lst_timer.get_next_time() - now_ms();
                wait_ms = delta_ms > 0 ? delta_ms : 0;
            }
        }

        unique_lock<mutex> lock(_mutex);
        if (wait_ms == -1) {
            _cv.wait(lock);
        } else if (wait_ms > 0) {
            _cv.wait_for(lock, chrono::milliseconds(wait_ms));
        }

        vector<AnyFunc> funcs;
        {
            lock_guard<mutex> lock(_mutex);
            funcs = _lst_timer.get_enable_timer();
        }
        for (auto& item : funcs) {
            create(item, true);
        }
    }
}