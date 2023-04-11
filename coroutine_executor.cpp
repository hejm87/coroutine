#include "coroutine_executor.h"
#include "unistd.h"
#include <thread>

thread_local context_t  g_ctx_main;
thread_local shared_ptr<Coroutine> g_co_running;

CoroutineExecutor::CoroutineExecutor()
{
    ;
}

CoroutineExecutor::~CoroutineExecutor()
{
    ;
}

bool CoroutineExecutor::run()
{
    thread([this]() {
        init_context();
        while (CoroutineSchedule::get_instance()->is_running()) {
            auto coroutine = get_local_coroutine();
            if (!coroutine) {
                coroutine = get_global_coroutine();
            }
            if (!coroutine) {
                usleep(1);
                continue ;
            }
            CoroutineSchedule::get_instance()->schedule(coroutine);
        }
    });
}

void CoroutineExecutor::put(shared_ptr<Coroutine> coroutine)
{
    lock_guard<mutex> lock(_mutex);
    _list_ready.push_back(coroutine);
}

void CoroutineExecutor::schedule(shared_ptr<Coroutine> coroutine)
{
    g_co_running = coroutine;
    swap_context(&g_ctx_main, go_co_running->get_context());
    if (coroutine->_status == COROUTINE_FINISH) {

    } else if (coroutine->_status == COROUTINE_SUSPEND) {

    } else {
        throw runtime_error("CoroutineExecutor::schedule, invalid status");
    }
}

Coroutine* CoroutineExecutor::get_local_coroutine()
{
    lock_guard<mutex> lock(_mutex);
    int now = (int)time(0);
    Coroutine* coroutine = NULL;
    lock_guard<mutex> lock(_mutex);
    while (_list_wait.size() > 0) {
        auto wait_coroutine = _list_wait.front();                
        if (now >= wait_coroutine->_active_time) {
            _list_ready.push_front(wait_coroutine);
            _list_wait.pop_front();
        } else {
            break ;
        }
    }
    if (_list_ready.size() > 0) {
        coroutine = _list_ready.front();
        _list_ready.pop_front();
    }
    return coroutine;
}

Coroutine* CoroutineExecutor::get_global_coroutine(int size)
{
    return NULL;
}