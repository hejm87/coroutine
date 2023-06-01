#include <assert.h>
#include "coroutine.h"
#include "co_executor.h"
#include "common/helper.h"
#include "co_common/co_timer.h"

using namespace std;

thread_local shared_ptr<CoExecutor> g_co_executor;

CoSchedule::CoSchedule()
{
    _timer = new CoTimer;

    _stack_size = get_stack_size();
    _executor_count = get_executor_count();

    for (int i = 0; i < _executor_count; i++) {
        auto ptr = shared_ptr<CoExecutor>(new CoExecutor);
        _executors.push_back(ptr);
        ptr->run();
    }

    for (int i = 0; i < get_coroutine_count(); i++) {
        auto ptr = shared_ptr<Coroutine>(new Coroutine(i));
        _lst_free.push_back(ptr);
    }
}

CoSchedule::~CoSchedule()
{
    _is_set_end = true;
    for (auto& item : _executors) {
        item->stop();
    }
    delete _timer;
}

void CoSchedule::create(bool priority, const function<void()>& func)
{
    lock_guard<mutex> lock(_mutex);
    shared_ptr<Coroutine> co;
    printf("########## CoSchedule::create, _lst_free.size:%d\n", _lst_free.size());
    if (!_lst_free.front(co)) {
        throw CoException(CO_ERROR_NO_RESOURCE);
    }
    printf("########## CoSchedule::create, has free coroutine\n");
    _lst_free.pop_front();

    co->_func = func;
    co->_priority = priority;
    if (priority) {
        _lst_ready.push_front(co);
    } else {
        _lst_ready.push_back(co);
    }
}

void CoSchedule::sleep(int sleep_ms)
{
    if (!g_co_executor) {
        throw CoException(CO_ERROR_NOT_IN_CO_THREAD);
    }
    auto co = g_co_executor->get_running_co();
    co->_status = CO_STATUS_SUSPEND;
    co->_suspend_status = CO_SUSPEND_SLEEP;

    auto time = now_ms() + sleep_ms;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _lst_suspend.push_back(co);
        _timer->set(sleep_ms, [this, &co]() {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_lst_suspend.is_exist(co)) {
                _lst_suspend.remove(co);
            } else {
                CO_LOG_ERROR("co_id:%d not in suspend list", co->_id);
                assert(false);
            }
            _lst_ready.push_front(co);
        });
    }
    g_ctx_handle->swap_context(co->get_context(), g_ctx_main);
}

void CoSchedule::yield(function<void()> do_after)
{
    auto co = g_co_executor->get_running_co();
    co->_status = CO_STATUS_READY;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _lst_ready.push_back(co);
    }
    if (do_after) {
        do_after();
    }
    g_ctx_handle->swap_context(co->get_context(), g_ctx_main);
}

void CoSchedule::resume(shared_ptr<Coroutine> co)
{
    std::lock_guard<std::mutex> lock(_mutex);     
    assert(_lst_suspend.is_exist(co));
    _lst_suspend.remove(co);
    _lst_ready.push_front(co);
}

CoTimerId CoSchedule::set_timer(int delay_ms, const std::function<void()>& func)
{
    return _timer->set(delay_ms, [this, &func]() {
        create(true, func);
    });
}

bool CoSchedule::stop_timer(const CoTimerId& timer_id)
{
    _timer->cancel(timer_id);
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
