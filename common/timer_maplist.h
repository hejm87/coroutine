#ifndef __TIMER_MAP_LIST_H__
#define __TIMER_MAP_LIST_H__

#include <map>
#include <unordered_map>
#include "helper.h"

template <class T>
class TimerMapList
{
public:
    TimerMapList() {;}
    ~TimerMapList() {;}

    bool insert(const T& obj, int delay_ms) {
        if (_map_list_iter.find(obj) != _map_list_iter.end()) {
            return false;
        }
        auto delay = now_ms() + delay_ms;
        _map_list_iter[obj] = _list.insert(make_pair(delay, obj));
        return true;
    }

    bool remove(const T& obj) {
        auto iter = _map_list_iter.find(obj);
        if (iter == _map_list_iter.end()) {
            return false;
        }
        _list.erase(iter->second);
        _map_list_iter.erase(obj);
        return true;
    }

    vector<T> get_expires() {
        auto now = now_ms();
        auto find_iter = _list.lower_bound(now);
        if (find_iter == _list.end()) {
            return vector<T>();
        }

        vector<T> expires;
        for (auto iter = _list.begin(); iter != find_iter; iter++) {
            expires.push_back(iter->second);
            _map_list_iter.erase(iter->second);
        }
        _list.erase(_list.begin(), find_iter);
        return expires;
    }

private:
    typedef typename std::unordered_map<long, T> TIMER_LIST;
    typedef typename std::unordered_map<long, T>::iterator TIMER_LIST_ITER;

    TIMER_LIST  _list;
    std::map<T, TIMER_LIST_ITER>    _map_list_iter;
};

#endif