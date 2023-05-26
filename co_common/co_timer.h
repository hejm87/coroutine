#ifndef __CO_TIMER_H__
#define __CO_TIMER_H__

#include <map>
#include <unordered_map>
#include <vector>
#include <memory>
#include "../common/helper.h"
//#include "../common/any_func.h"

class AnyFunc;
class CoTimer;

typedef std::multimap<long, std::shared_ptr<AnyFunc>> co_timer_list_t;
typedef std::unordered_map<std::shared_ptr<AnyFunc>, co_timer_list_t::iterator> co_map_timer_list_t;

class CoTimerId
{
friend class CoTimer;
private:
    CoTimer(std::shared_ptr<AnyFunc>& ptr) {
        _ptr = ptr;
    }
    std::weak_ptr<AnyFunc>  _ptr;
};

class CoTimer
{
public:
    template <class F, class... Args> 
    CoTimerId set(int delay_ms, F&& f, Args&&... args) {
        auto time = now_ms() + delay_ms;
        auto ptr  = std::shared_ptr<AnyFunc>(new AnyFunc(std::forward<F>(f), std::forward<Args>(args)...));
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

    std::vector<AnyFunc> get_expires() {
        if (_list.size() == 0) {
            return std::vector<AnyFunc>();
        }
        auto now = now_ms();
        auto find_iter = _list.lower_bound(now);
        if (find_iter == _list.end()) {
            return std::vector<AnyFunc>();
        }
        std::vector<AnyFunc> expires;
        for (auto iter = _list.begin(); iter != find_iter; iter++) {
            _map_list_iter.erase(iter->second);
            expires.emplace_back(std::move(*(iter->second)));
        }
        _list.erase(_list.begin(), find_iter);
        return expires;
    }

private:
    co_timer_list_t     _list;
    co_map_timer_list_t _map_list_iter;
};

#endif