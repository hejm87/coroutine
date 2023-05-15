#ifndef __CO_SCHEDULE_H__
#define __CO_SCHEDULE_H__

#include <mutex>
#include <atomic>
#include <exception>
#include <thread>
#include <condition_variable>
#include <vector>
#include "co_common.h"
#include "co_exception.h"
#include "common/any.h"
#include "co_common/co_list.h"
#include "co_common/co_timer.h"

class Coroutine;
class CoTimerId;
class CoAwaiter;
class CoExecutor;

extern thread_local shared_ptr<CoExecutor> g_co_executor;

class CoSchedule
{
public:
    static CoSchedule* get_instance() {
        static CoSchedule instance;
        return &instance;
    }

    void create(const AnyFunc& func, bool priority = false);

    CoAwaiter create_with_promise(const AnyFunc& func, bool priority = false);

    void sleep(int sleep_ms);

    void yield(std::function<void()> do_after = nullptr);

    void resume(std::shared_ptr<Coroutine> co);

    /*
    �������ܣ����ö�ʱ��
    ������
    func:   ��ʱ���ص�����
    delay:  �ӳ�ִ��ʱ�䣨��λ�����룩
    period: ������ִ�����գ���λ�����룩��Ĭ�ϲ���Ϊ0��ִֻ��һ�� 
    */
    CoTimerId set_timer(const AnyFunc& func, int delay_ms, int period_ms = 0);

    bool stop_timer(const CoTimerId& timer_id);

    std::shared_ptr<Coroutine> get_cur_co();

    std::vector<std::shared_ptr<Coroutine>> get_global_co(int size = 1);

    void free(shared_ptr<Coroutine> co) {
        _lst_free.push_back(co);
    }

    bool is_set_end() {
        return _is_set_end;
    }

private:
    CoSchedule();
    ~CoSchedule();

    void run_timer();

    bool get_free_co(std::shared_ptr<Coroutine> &co) {
        lock_guard<mutex> lock(_mutex);
        if (!_lst_free.front(co)) {
            return false;
        }
        _lst_free.pop_front();
    }

private:
    CoList  _lst_free;
    CoList  _lst_ready;

    int     _stack_size;
    int     _max_co_size;

    int     _cur_co_size;
    int     _executor_count;

    CoTimerList     _lst_timer;

    std::atomic<bool>    _is_set_end;
    std::vector<std::shared_ptr<CoExecutor>>  _executors;

    std::condition_variable _cv;
    std::mutex  _mutex;
    std::thread _timer_thread;
};

#endif
