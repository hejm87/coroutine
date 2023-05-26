#ifndef __MAP_LIST_H__
#define __MAP_LIST_H__

#include <map>
#include <list>
#include <algorithm>

template <typename T>
class MapList
{
public:
	MapList() {
		;
	}

	~MapList() {
		;
	}

    MapList(MapList&& obj) {
        std::swap(_list, obj._list);
        std::swap(_map_list_iter, obj._map_list_iter);
    }

    MapList& operator=(MapList&& obj) {
        std::swap(_list, obj._list);
        std::swap(_map_list_iter, obj._map_list_iter);
    }

    void push_front(const T& obj) {
        _list.push_front(obj);
        _map_list_iter[obj] = _list.begin();
    }

    void push_back(const T& obj) {
        _list.push_back(obj);
        _map_list_iter[obj] = --_list.end();
    }

    void pop_front() {
        if (is_empty()) {
            return ;
        }
        auto obj = _list.front();
        _list.pop_front();
        _map_list_iter.erase(obj);
    }

    void pop_back() {
        if (is_empty()) {
            return ;
        }
        auto obj = _list.back();
        _list.pop_back();
        _map_list_iter.erase(obj);
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

    bool remove(T& obj) {
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
        auto iter = _map_list_iter.find(obj);
        return iter != _map_list_iter.end() ? true : false;
    }

	int size() {
		return (int)_list.size();
	}

private:
    typedef typename std::list<T>::iterator list_iter_t;

    std::list<T>    _list;
    std::map<T, list_iter_t>    _map_list_iter;
};

#endif
