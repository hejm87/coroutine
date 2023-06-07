#ifndef __CO_TIMER_H__
#define __CO_TIMER_H__

#include <map>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>

#include "../common/helper.h"
#include "../co_schedule.h"

class CoTimer;

typedef std::multimap<long, std::shared_ptr<std::function<void()>>> co_timer_list_t;
typedef std::unordered_map<std::shared_ptr<std::function<void()>>, co_timer_list_t::iterator> co_map_timer_list_t;

class CoTimerId
{
friend class CoTimer;
friend class CoSchedule;
private:
    CoTimerId(CoTimerId&& obj) {
        std::swap(_ptr, obj._ptr);
    }
    CoTimerId(const std::shared_ptr<std::function<void()>>& ptr) {
        _ptr = ptr;
    }
    std::weak_ptr<std::function<void()>>  _ptr;
};

class CoTimer
{
public:
    CoTimer(int workers) {
        for (int i = 0; i  < workers; i++) {
            _workers.emplace_back([this] {
                run();
            });
        }
    }

    ~CoTimer() {
        stop();
    }

    void stop(bool wait = false) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_is_set_end) {
                return ;
            }
            _is_set_end = true;
        }
        for (auto& worker : _workers) {
            if (wait) {
                worker.join();
            } else {
                worker.detach();
            }
        }
    }

    CoTimerId set(int delay_ms, const std::function<void()>& func) {
        auto notify = true;
        auto trigger = now_ms() + delay_ms;
        auto co = Singleton<CoSchedule>::get_instance()->get_running_co();
        printf(
            "[%s]tid:%d, cid:%d set timer sleep, wake time:%s\n", 
            date_ms().c_str(), 
            gettid(), 
            co->_id,
            date_ms(trigger).c_str()
        );
        std::lock_guard<std::mutex> lock(_mutex);
        if (_list.size() > 0 && trigger >= _list.begin()->first) {
            notify = false;
        }
        auto ptr = std::shared_ptr<std::function<void()>>(new std::function<void()>(func));
        _map_list_iter[ptr] = _list.insert(make_pair(trigger, ptr));
        if (notify) {
            printf("[%s]tid:%d, cid:%d timer notify\n", date_ms().c_str(), gettid(), co->_id);
            _cv.notify_one();
        }
        return CoTimerId(ptr);
    }

    bool cancel(const CoTimerId& id) {
        std::lock_guard<std::mutex> lock(_mutex);
        auto ptr = id._ptr.lock();
        if (!ptr) {
            return false;
        }
        auto iter = _map_list_iter.find(ptr);
        if (iter == _map_list_iter.end()) {
            return false;
        }
        _list.erase(iter->second);
        _map_list_iter.erase(iter);
        return true;
    }

    int size() {
        return (int)_list.size();
    }

private:
    void run() {
        printf("[%s]########## tid:%d timer thread running\n", date_ms().c_str(), gettid());
        while (1) {
            int wait = -1;
            auto now = now_ms();
            std::function<void()> func;
            {
                std::lock_guard<std::mutex> lock(_mutex);
                if (_is_set_end) {
                    break ;
                }
                if (_list.size() > 0 && _list.begin()->first >= now) {
                    auto iter = _list.begin();
                    printf(
                        "############ timer, first_ms:%s, now:%s\n", 
                        date_ms(iter->first).c_str(), 
                        date_ms(now).c_str()
                    );
                    func = *(iter->second);
                    _map_list_iter.erase(iter->second);
                    _list.erase(iter);
                }
            }
            if (func) {
                func();
                continue ;
            }
            std::unique_lock<std::mutex> lock(_mutex);
            if (wait >= 0) {
                printf("[%s]########### tid:%d timer thread wait %dms\n", date_ms().c_str(), gettid(), wait);
                _cv.wait_for(lock, std::chrono::milliseconds(wait));
            } else {
                printf("[%s]########### tid:%d timer thread wait\n", date_ms().c_str(), gettid());
                _cv.wait(lock);
            }
        }    
    }

//    long get_latest_expire() {
//        if (!_list.size()) {
//            return 0;
//        }
//        return _list.begin()->first;
//    }

private:
    co_timer_list_t     _list;
    co_map_timer_list_t _map_list_iter;

    bool _is_set_end;

    std::vector<std::thread> _workers;

    std::mutex _mutex;
    std::condition_variable _cv;
};

#endif