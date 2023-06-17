#include <assert.h>
#include <chrono>
#include "coroutine.h"
#include "co_executor.h"
#include "common/helper.h"
#include "co_common/co_timer.h"

using namespace std;

thread_local shared_ptr<CoExecutor> g_co_executor;
//thread_local CoExecutor* g_co_executor;

CoSchedule::CoSchedule()
{
    _log_level = CO_LEVEL_INFO;
    _timer = new CoTimer(get_timer_thread_count());

    _stack_size = get_stack_size();
    _executor_count = get_executor_count();

    printf("init|_lst_free addr:%p\n", &_lst_free);
    printf("init|_lst_ready addr:%p\n", &_lst_ready);
    printf("init|_lst_suspend addr:%p\n", &_lst_suspend);

    printf("executor_count:%d", _executor_count);
    printf("timer_thread_count:%d", get_timer_thread_count());
    printf("coroutine_count:%d", get_coroutine_count());

    for (int i = 0; i < _executor_count; i++) {
  //  for (int i = 0; i < 1; i++) {
        auto ptr = shared_ptr<CoExecutor>(new CoExecutor);
       // auto ptr = new CoExecutor;
        _executors.push_back(ptr);
        ptr->run();
    }

    for (int i = 0; i < get_coroutine_count(); i++) {
   // for (int i = 0; i < 10; i++) {
        auto co = new Coroutine(i);
        printf("co[%d] : %p\n", i, co);
        auto ptr = shared_ptr<Coroutine>(co);
        if (!ptr->init()) {
            throw CoException(CO_ERROR_INIT_ENV_FAIL);
        }
        _lst_free.push_back(ptr);
        _coroutines.push_back(ptr);
    }
}

CoSchedule::~CoSchedule()
{
    printf("########## ~CoSchedule\n");
    _is_set_end = true;
    for (auto item : _executors) {
        item->stop();
    }
    delete _timer;
}

void CoSchedule::create(bool priority, const function<void()>& func)
{
    printf("############ CoSchedule::create\n");
    CO_LOG_INFO("create coroutine, priority:%s", priority ? "true" : "false");
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

void CoSchedule::sleep(int sleep_ms)
{
    if (sleep_ms <= 0) {
        return ;
    }
    if (!g_co_executor) {
        throw CoException(CO_ERROR_NOT_IN_CO_THREAD);
    }
    g_co_executor->sleep(sleep_ms);
}

//void CoSchedule::sleep(int sleep_ms)
//{
//    printf("############ sleep %dms\n", sleep_ms);
//    if (sleep_ms <= 0) {
//        return ;
//    }
//    if (!g_co_executor) {
//        throw CoException(CO_ERROR_NOT_IN_CO_THREAD);
//    }
//    auto co = g_co_executor->get_running_co();
//    co->_status = CO_STATUS_SUSPEND;
//    co->_suspend_status = CO_SUSPEND_SLEEP;
//
//   // {
//   //     std::lock_guard<std::mutex> lock(_mutex);
//   //     _lst_suspend.push_back(co);
//   //     _timer->set(sleep_ms, [this, co] {
//   //         printf("[%s][co_schedule, sleep]tid:%d, cid:%d sleep finish\n", date_ms().c_str(), gettid(), co->_id);
//   //         {
//   //             std::lock_guard<std::mutex> lock(_mutex);
//   //             printf("[%s][co_schedule, sleep]cid:%d sleep.timer co_schedule.lock\n", date_ms().c_str(), co->_id);
//   //             assert(_lst_suspend.is_exist(co));
//   //             _lst_suspend.remove(co);
//   //             printf("[%s][co_schedule, sleep]cid:%d sleep.timer _lst_ready push_front, ptr:%p\n", date_ms().c_str(), co->_id, co.get());
//   //             _lst_ready.push_front(co);
//   //             printf("[%s][co_schedule, sleep]cid:%d sleep.timer co_schedule.unlock\n", date_ms().c_str(), co->_id);
//   //         }
//   //         printf("[%s][co_schedule, sleep]tid:%d, cid:%d co_schedule.unlock\n", date_ms().c_str(), gettid(), co->_id);
//   //     });
//   // }
//
//    {
//        lock_guard<mutex> lock(_mutex);
//       // _lst_suspend.push_back(co);
//        _lst_sleep.insert(make_pair(now_ms() + sleep_ms, co));
//    }
//
//    // TODO û����swap_context�����ͱ������ˣ�˯�ߵ�Э���е��������ߵ�������ô�ƣ�����
//    g_ctx_handle->swap_context(co->get_context(), g_ctx_main);
//}

void CoSchedule::yield(function<void()> doing)
{
    auto co = g_co_executor->get_running_co();
    co->_status = CO_STATUS_READY;
    {
        lock_guard<mutex> lock(_mutex);
        _lst_ready.push_back(co);
        if (doing) {
            doing();
        }
    }
   // ֮ǰ����д�����?
   // if (doing) {
   //     doing();
   // }
    g_ctx_handle->swap_context(co->get_context(), g_ctx_main);
}

void CoSchedule::suspend(function<bool()> do_and_check)
{
    auto co = g_co_executor->get_running_co();
    co->_status = CO_STATUS_SUSPEND;
    {
       // printf("[%s][co_schedule, suspend]tid:%d, cid:%d, suspend, try lock, mutex:%p\n", date_ms().c_str(), gettid(), co->_id, &_mutex);
        lock_guard<mutex> lock(_mutex);
        if (!do_and_check()) {
           // printf("[%s][co_schedule, suspend]tid:%d, cid:%d, suspend, unlock1, mutex:%p\n", date_ms().c_str(), gettid(), co->_id, &_mutex);
            return ;
        }
       // printf("[%s][co_schedule, suspend]tid:%d, cid:%d, suspend, lock, mutex:%p\n", date_ms().c_str(), gettid(), co->_id, &_mutex);
        _lst_suspend.push_back(co);
       // printf("[%s][co_schedule, suspend]tid:%d, cid:%d, suspend, unlock2, mutex:%p\n", date_ms().c_str(), gettid(), co->_id, &_mutex);
    }
    g_ctx_handle->swap_context(co->get_context(), g_ctx_main);
}

void CoSchedule::resume(shared_ptr<Coroutine> co)
{
    auto cur_co = g_co_executor->get_running_co();
   // printf("[%s][co_schedule, resume]tid:%d, cid:%d, try lock\n", date_ms().c_str(), gettid(), co->_id);
    lock_guard<mutex> lock(_mutex);     
   // printf("[%s][co_schedule, resume]tid:%d, cid:%d, lock, resume_cid:%d\n", date_ms().c_str(), gettid(), co->_id, co->_id);
    assert(_lst_suspend.is_exist(co));
    co->_status = CO_STATUS_READY;
    _lst_suspend.remove(co);
    _lst_ready.push_front(co);
   // printf("[%s][co_schedule, resume]tid:%d, cid:%d, unlock\n", date_ms().c_str(), gettid(), co->_id);
}

void CoSchedule::resume(std::function<std::shared_ptr<Coroutine>()> do_and_resume)
{
    auto cur_co = g_co_executor->get_running_co();
    printf("[%s][co_schedule, resume_lamdba]tid:%d, cid:%d, try lock\n", date_ms().c_str(), gettid(), cur_co->_id);
    lock_guard<mutex> lock(_mutex);
    printf("[%s][co_schedule, resume_lamdba]tid:%d, cid:%d, lock\n", date_ms().c_str(), gettid(), cur_co->_id);
    auto resume_co = do_and_resume();
    if (resume_co) {
        printf(
            "[%s][co_schedule, resume_lamdba]tid:%d, cid:%d, resume_cid:%d\n", 
            date_ms().c_str(), 
            gettid(), 
            cur_co->_id, 
            resume_co->_id
        );
        assert(_lst_suspend.is_exist(resume_co));
        resume_co->_status = CO_STATUS_READY;
        _lst_suspend.remove(resume_co);
        _lst_ready.push_front(resume_co);
    }
   // printf("[%s][co_schedule, resume_lamdba]tid:%d, cid:%d, unlock\n", date_ms().c_str(), gettid(), cur_co->_id);
}

void CoSchedule::release()
{
    auto co = g_co_executor->get_running_co();
    printf(
        "------------- [%s]release, tid:%d, cid:%d, is %s\n", 
        date_ms().c_str(),
        gettid(),
        co->_id,
        co ? "valid" : "invalid"
    );
    co->_status = CO_STATUS_IDLE;
    {
        printf("[%s][co_schedule, release]tid:%d, cid:%d, free, try lock\n", date_ms().c_str(), gettid(), co->_id);
        lock_guard<mutex> lock(_mutex);
        printf("[%s][co_schedule, release]tid:%d, cid:%d, free, lock\n", date_ms().c_str(), gettid(), co->_id);
        _lst_free.push_back(co);
    }
    printf("[%s][co_schedule, release]tid:%d, cid:%d, tree, unlock\n", date_ms().c_str(), gettid(), co->_id);
    g_ctx_handle->swap_context(co->get_context(), g_ctx_main);
}

CoTimerId CoSchedule::set_timer(int delay_ms, const function<void()>& func)
{
    lock_guard<mutex> lock(_mutex);
    return _timer->set(delay_ms, [this, func] {
        create(true, func);
    });
}

bool CoSchedule::stop_timer(const CoTimerId& timer_id)
{
    lock_guard<mutex> lock(_mutex);
    return _timer->cancel(timer_id);
}

vector<shared_ptr<Coroutine>> CoSchedule::get_global_co(int size)
{
    vector<shared_ptr<Coroutine>> cos;
    lock_guard<mutex> lock(_mutex);
    if (_lst_sleep.size() > 0) {
        auto now = now_ms();
        auto find_iter = _lst_sleep.upper_bound(now);
        for (auto iter = _lst_sleep.begin(); iter != find_iter; iter++) {
            _lst_ready.push_front(iter->second);
        }
        _lst_sleep.erase(_lst_sleep.begin(), find_iter);
    }

    for (int i = 0; i < size; i++) {
        shared_ptr<Coroutine> co;
        if (!_lst_ready.front(co)) {
            break ;
        }
        _lst_ready.pop_front();
        printf("[%s]get_global_co, get ready, cid:%d\n", date_ms().c_str(), co->_id);
        cos.push_back(co);
    }
    return cos;
}

shared_ptr<Coroutine> CoSchedule::get_running_co()
{
    return g_co_executor->get_running_co();
}
