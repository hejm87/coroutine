#ifndef __MAP_LIST_H__
#define __MAP_LIST_H__

#include <assert.h>
#include <sys/time.h>

#include <map>
#include <list>
#include <algorithm>

#include "helper.h"

#define now_usss() \
({ \
    struct timeval tv; \
    gettimeofday(&tv, NULL); \
	(tv.tv_sec * 1000000 + tv.tv_usec); \
})

template <typename T>
class MapList
{
public:
	MapList() {
		;
	}

	~MapList() {
        printf("############## [%ld]release MapList, pt:%p\n", now_usss(), this);
    }

    MapList(MapList&& obj) {
        printf("############## goto MapList(MapList&&)\n");
        exit(0);
        std::swap(_list, obj._list);
        std::swap(_map_list_iter, obj._map_list_iter);
    }

    MapList& operator=(MapList&& obj) {
        printf("############## goto MapList operation=\n");
        exit(0);
        std::swap(_list, obj._list);
        std::swap(_map_list_iter, obj._map_list_iter);
    }

    void push_front(const T& obj) {
        printf("[%ld] !!!!!!!!!!!!!!!! tid:%d, colist.push_front, beg, ptr:%p ...\n", now_usss(), gettid(), this);
        _map_list_iter[obj] = _list.insert(_list.begin(), obj);
        assert(_map_list_iter.size() == _list.size());
        printf("[%ld] !!!!!!!!!!!!!!!! tid:%d, colist.push_front, end, ptr:%p ...\n", now_usss(), gettid(), this);
    }

    void push_back(const T& obj) {
        printf("[%ld] !!!!!!!!!!!!!!!! tid:%d, colist.push_back, beg, ptr:%p ...\n", now_usss(), gettid(), this);
        _map_list_iter[obj] = _list.insert(_list.end(), obj);
        assert(_map_list_iter.size() == _list.size());
        printf("[%ld] !!!!!!!!!!!!!!!! tid:%d, colist.push_back, end, ptr:%p ...\n", now_usss(), gettid(), this);
    }

    void pop_front() {
        if (is_empty()) {
            return ;
        }
        auto obj = _list.front();
        _map_list_iter.erase(obj);
        _list.pop_front();
    }

    void pop_back() {
        if (is_empty()) {
            return ;
        }
        auto obj = _list.back();
        _map_list_iter.erase(obj);
        _list.pop_back();
    }

    bool front(T& obj) {
        if (is_empty()) {
            return false;
        }
        obj = *(_list.begin());
        return true;
    }

    bool back(T& obj) {
        if (is_empty()) {
            return false;
        }
        obj = *(--_list.end());
        return true;
    }

    bool remove(const T& obj) {
        auto iter = _map_list_iter.find(obj);
        if (iter == _map_list_iter.end()) {
            return false;
        }
        _list.erase(iter->second);
        _map_list_iter.erase(iter);
        return true;
    }

    void clear() {
        _list.clear();
        _map_list_iter.clear();
    }

	bool is_empty() {
		return _list.size() == 0 ? true : false;
	}

    bool is_exist(const T& obj) {
        return _map_list_iter.find(obj) != _map_list_iter.end() ? true : false;
    }

	int size() {
		return (int)_list.size();
	}

private:
    typedef typename std::list<T>::iterator list_iter_t;

    std::map<T, list_iter_t>    _map_list_iter;
    std::list<T>    _list;
};

#endif
