#include <unistd.h>
#include <thread>

#include "common/helper.h"
#include "coroutine_executor.h"
#include "coroutine_schedule.h"

using namespace std;

CoExecutor::CoExecutor()
{
    _is_running = false;
}

CoExecutor::~CoExecutor()
{
    if (_is_running) {
        stop(true);
    }
}

bool CoExecutor::run()
{
    _thread_handle = thread([this]() {
        init_context();
        while (!CoSchedule::get_instance()->is_set_end()) {
            if (!on_execute()) {
                sleep(1);
            }
        }
        _is_running = false;
    });
}

bool CoExecutor::stop()
{
    if (!_is_running) {
        return false;
    }
    _thread_handle.join();
    return true;
}

void CoExecutor::put(shared_ptr<Coroutine> co)
{
    lock_guard<mutex> lock(_mutex);
    _list_ready.push_back(co);
}

void CoExecutor::sleep(int sleep_ms)
{
    if (sleep_ms <= 0) {
        return 0;
    }

    lock_guard<mutex> lock(_mutex);
    auto co = _running_co;
    auto delay = now_ms() + sleep_ms;
    _lst_timer[delay] = [this, co]() {
        lock_guard<mutex> lock(_mutex);
        _lst_wait.remove(co);
        _lst_ready.push_front(co);
    };
}

void CoExecutor::yield(function<void()> do_after)
{
    _running_co->_status = CO_STATUS_READY;
    if (do_after) {
        do_after();
    }
    swap_context(_running_co->get_context(), &g_ctx_main);
}

void CoExecutor::resume(shared_ptr<Coroutine> co)
{
    lock_guard<mutex> lock(_mutex);
    if (!_lst_wait.remove(co)) {
        throw CoException(CO_ERROR_COROUTINE_EXCEPTION);
    }
    _lst_ready.push_front(co);
}

CoTimerId CoExecutor::set_timer(const AnyFunc& func, int delay_ms, int period_ms)
{
    lock_guard<mutex> lock(_mutex);

    auto timer_ptr = shared_ptr<CoTimer>(new CoTimer);
    timer_ptr->timer_func = func;
    timer_ptr->period_ms  = period_ms;

    auto result = _lst_timer.insert(now_ms() + delay_ms, timer_ptr);
    if (!result.first) {
        throw CoException(CO_ERROR_SET_TIMER);
    }

    CoTimerId timer_id;
    timer_id._ptr = timer_ptr;
    return timer_id;
}

bool CoExecutor::stop_timer(const CoTimer& timer)
{
    auto ret = false;
    if (auto ptr = timer._ptr.lock()) {
        lock_guard<mutex> lock(_mutex);
        auto iter = _lst_timer_ptr.find(ptr);
        if (iter != _lst_timer_ptr) {
            _lst_timer.erase(iter);
            _lst_timer.ptr.erase(ptr);
            ret = true;
        }
    }
    return ret;
}

bool CoExecutor::on_execute()
{
    shared_ptr<Coroutine> co;
    if (!get_ready_co(co)) {
        return false;
    }

    _running_co = co;
    swap_context(&g_ctx_main, co->get_context());
    if (co->_status != CO_STATUS_SUSPEND && co->_status != CO_STATUS_FINISH) {
        throw CoException(CO_ERROR_COROUTINE_EXCEPTION);
    }

    if (co->_status == CO_STATUS_FINISH) {
        co->_status = CO_STATUS_IDLE;
        CoSchedule::get_instance()->free(co);
    }   
    return true;
}

bool CoExecutor::get_ready_co(shared_ptr<Coroutine>& co)
{
    {
        lock_guard<mutex> lock(_mutex);
        if (_lst_ready.pop_front(co)) {
            return true;
        }
    }

    vector<shared_ptr<Coroutine>> cos;
    if (!CoSchedule::get_instance()->get_global_co(cos)) {
        return false;
    }

    lock_guard<mutex> lock(_mutex);
    for (auto& item : cos) {
        if (item->_priority) {
            _lst_ready.push_front(item);
        } else {
            _lst_ready.push_back(item);
        }
    }
    return _lst_ready.pop_front(co);
}