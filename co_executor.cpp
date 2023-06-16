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
    printf("############## goto ~CoExecutor, tid:%d\n", gettid());
    // TODO 下面代码增加terminate called without an active exception风险
    if (_is_running) {
        stop(true);
    }
}

bool CoExecutor::run()
{
    auto ptr = shared_from_this();
    _thread = move(thread([ptr]() {
        printf("[%s]tid:%d, CoExecutor thread is start\n", date_ms().c_str(), gettid());
        ptr->_is_running = true;
        g_co_executor = ptr;
        g_ctx_handle->init_context();
        while (!ptr->_is_set_end) {
            if (!ptr->on_execute()) {
                usleep(10);
            }
        }
        ptr->_is_running = false;
        printf("[%s]tid:%d CoExecutor thread is exit\n", date_ms().c_str(), gettid());
    }));
    // xxxx 需要添加等待初始化设置完成
   // _thread.detach();
    return true;
}

void CoExecutor::stop(bool wait)
{
    set_end();
    if (wait) {
        wait_util_stop();
        _is_running = false;
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

void CoExecutor::sleep(int sleep_ms)
{
    if (sleep_ms <= 0) {
        return ;
    }
    auto co = g_co_executor->get_running_co();
    co->_status = CO_STATUS_SUSPEND;
    co->_suspend_status = CO_SUSPEND_SLEEP;
    _lst_sleep.insert(make_pair(now_ms() + sleep_ms, co));
    g_ctx_handle->swap_context(co->get_context(), g_ctx_main);
}

/*
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

//bool CoExecutor::on_awake()
//{
//    std::vector<std::shared_ptr<Coroutine>> awake_cos;
//    {
//        lock_guard<mutex> lock(_mutex);
//        auto find_iter = _lst_sleep.upper_bound(now_ms());
//        if (find_iter == _lst_sleep.end()) {
//            return false;
//        }
//        for (auto iter = _lst_sleep.begin(); iter < find_iter; iter++) {
//            awake_cos.push_back(iter->second);
//        }
//    }
//    Singleton<CoSchedule>::get_instance()->append_ready_list(awake_cos);
//    return !awake_cos.empty() ? true : false;
//}

bool CoExecutor::on_execute()
{
    shared_ptr<Coroutine> co;
    if (!get_ready_co(co)) {
        return false;
    }
    printf("[%s]tid:%d, cid:%d, run\n", date_ms().c_str(), gettid(), co->_id);
    co->_status = CO_STATUS_RUNNING;
    _running_co = co;
    printf(
        "[%s]tid:%d, cid:%d, status:%d, swap_context before\n", 
        date_ms().c_str(),
        gettid(), 
        co->_id, 
        co->_status
    );
    g_ctx_handle->swap_context(g_ctx_main, co->get_context());
    return true;
}

bool CoExecutor::get_ready_co(shared_ptr<Coroutine>& co)
{
    auto now = now_ms();
    // 获取休眠结束的协�?
    if (_lst_sleep.size() > 0 && _lst_sleep.begin()->first <= now) {
        auto iter = _lst_sleep.begin();
        for (; iter != _lst_sleep.end(); iter++) {
            if (iter->first > now) {
                break ;
            }
            _lst_ready.push_front(iter->second);
        }
        _lst_sleep.erase(_lst_sleep.begin(), iter);
    }

    if (_lst_ready.front(co)) {
        _lst_ready.pop_front();
        return true;
    }

    auto cos = Singleton<CoSchedule>::get_instance()->get_global_co();
    if (!cos.size()) {
        return false;
    }

    for (auto& item : cos) {
        if (item->_priority) {
            _lst_ready.push_front(item);
        } else {
            _lst_ready.push_back(item);
        }
    }
    _lst_ready.front(co);
    _lst_ready.pop_front();
    return true;
}

//bool CoExecutor::get_ready_co(shared_ptr<Coroutine>& co)
//{
//    {
//        lock_guard<mutex> lock(_mutex);
//        if (!_lst_ready.is_empty()) {
//            _lst_ready.front(co);
//            _lst_ready.pop_front();
//            return true;
//        }
//    }
//
//    auto cos = Singleton<CoSchedule>::get_instance()->get_global_co();
//    if (cos.size() == 0) {
//        return false;
//    }
//
//    lock_guard<mutex> lock(_mutex);
//    for (auto& item : cos) {
//        if (item->_priority) {
//            _lst_ready.push_front(item);
//        } else {
//            _lst_ready.push_back(item);
//        }
//    }
//    _lst_ready.front(co);
//    _lst_ready.pop_front();
//   // co->_co_executor = shared_from_this();
//    return true;
//}
