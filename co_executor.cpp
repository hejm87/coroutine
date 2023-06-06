#include <unistd.h>
#include <thread>

#include "common/helper.h"
#include "coroutine.h"
#include "co_executor.h"
#include "co_schedule.h"

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
    _thread = thread([this]() {
        g_co_executor = shared_from_this();
        CO_LOG_DEBUG("########### tid:%d CoExecutor running, ptr:%p", gettid(), g_co_executor.get());
        g_ctx_handle->init_context();
        while (!_is_set_end) {
            if (!on_execute()) {
                sleep(1);
            }
        }
        _is_running = false;
    });
    // xxxx 需要添加等待初始化设置完成
    _thread.detach();
}

void CoExecutor::stop(bool wait)
{
    set_end();
    if (wait) {
        wait_util_stop();
    }
}

bool CoExecutor::wait_util_stop()
{
    if (!_is_running) {
        return false;
    }
    _thread.join();
    return true;
}
/*
void CoExecutor::put(shared_ptr<Coroutine> co)
{
    lock_guard<mutex> lock(_mutex);
    _lst_ready.push_back(co);
}

void CoExecutor::sleep(int sleep_ms)
{
    if (sleep_ms <= 0) {
        return ;
    }
    lock_guard<mutex> lock(_mutex);
    auto co = _running_co;
    auto delay = now_ms() + sleep_ms;
    _lst_timer.insert([this, co]() {
        lock_guard<mutex> lock(_mutex);
        _lst_wait.remove(co);
        _lst_ready.push_front(co);
    }, delay, 0);
}

void CoExecutor::yield(function<void()> do_after)
{
    _running_co->_status = CO_STATUS_READY;
    if (do_after) {
        do_after();
    }
    g_ctx_handle->swap_context(_running_co->get_context(), g_ctx_main);
}

void CoExecutor::resume(shared_ptr<Coroutine> co)
{
    lock_guard<mutex> lock(_mutex);
    if (!_lst_wait.remove(co)) {
        throw CoException(CO_ERROR_COROUTINE_EXCEPTION);
    }
    _lst_ready.push_front(co);
}
*/
shared_ptr<Coroutine> CoExecutor::get_running_co()
{
    return _running_co;
}

void CoExecutor::set_end()
{
    _is_set_end = true;
}

bool CoExecutor::on_execute()
{
    shared_ptr<Coroutine> co;
    if (!get_ready_co(co)) {
        return false;
    }
    CO_LOG_DEBUG("######## tid:%d, cid:%d, run", gettid(), co->_id);
    co->_status = CO_STATUS_RUNNING;
    _running_co = co;
    g_ctx_handle->swap_context(g_ctx_main, co->get_context());
    if (co->_status != CO_STATUS_SUSPEND && co->_status != CO_STATUS_FINISH) {
        CO_LOG_DEBUG("######### EXCEPTION|tid:%d, cid:%d, status:%d", gettid(), co->_id, co->_status);
        throw CoException(CO_ERROR_COROUTINE_EXCEPTION);
    }

    CO_LOG_DEBUG("######## tid:%d, cid:%d, release", gettid(), co->_id);
    if (co->_status == CO_STATUS_FINISH) {
        co->_status = CO_STATUS_IDLE;
        Singleton<CoSchedule>::get_instance()->free(co);
    }   
    return true;
}

bool CoExecutor::get_ready_co(shared_ptr<Coroutine>& co)
{
    {
        lock_guard<mutex> lock(_mutex);
        if (!_lst_ready.is_empty()) {
            _lst_ready.front(co);
            _lst_ready.pop_front();
            return true;
        }
    }

    auto cos = Singleton<CoSchedule>::get_instance()->get_global_co();
    if (cos.size() == 0) {
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
    _lst_ready.front(co);
    _lst_ready.pop_front();
   // co->_co_executor = shared_from_this();
    return true;
}