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

/*
    typedef std::function<void()> fun_t;

    auto f = []() {
        printf("hello world\n");
    };  
    auto sp_fun = std::shared_ptr<fun_t>(new fun_t(f));
    (*sp_fun)();
*/

class CoTimer
{
public:
    CoTimerId set(int delay_ms, const std::function<void()>& func) {
        auto time = now_ms() + delay_ms;
        auto ptr = std::shared_ptr<std::function<void()>>(new std::function<void()>(func));
        _map_list_iter[ptr] = _list.insert(make_pair(time, ptr));
        return CoTimerId(ptr);
    }

    bool cancel(const CoTimerId& id) {
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

    std::vector<std::function<void()>> get_expires(int size) {
        if (size <= 0) {
            throw CoException(CO_ERROR_PARAM_INVALID);
        }
        std::vector<std::function<void()>> expires;
        auto now = now_ms();
        auto find_iter = _list.lower_bound(now);
        do {
            if (find_iter == _list.end()) {
                break ;
            }
            auto del_end_iter = _list.begin();
            for (auto iter = _list.begin(); iter != find_iter; iter++) {
                del_end_iter = iter;
                _map_list_iter.erase(iter->second);
                expires.push_back(*(iter->second));
                if (expires.size() >= size) {
                    break ;
                }
            }
            if (_list.begin() == del_end_iter) {
                _list.erase(_list.begin());
            } else {
                _list.erase(_list.begin(), del_end_iter);
            }
        } while (0);
        return expires;
    }

private:
    co_timer_list_t     _list;
    co_map_timer_list_t _map_list_iter;
};

#endif