#ifndef __CO_LIST_H__
#define __CO_LIST_H__

#include <algorithm>
#include <memory>
#include <list>
#include <map>

class Coroutine;

class CoList
{
public:
//    CoList() {
//        _size = 0;
//    }
//
//    CoList(CoList&& obj) {
//        std::swap(_header, obj._header);
//        std::swap(_size, obj._size);
//    }
//
//    CoList& operator=(CoList&& obj) {
//        std::swap(_header, obj._header);
//        std::swap(_size, obj._size);
//        return *this;
//    }

    CoList(CoList&& obj);
    CoList& operator=(CoList&& obj);

    void push_front(std::shared_ptr<Coroutine> node);
    void push_back(std::shared_ptr<Coroutine> node);

    void pop_front();
    void pop_back();

    bool front(std::shared_ptr<Coroutine>& node);
    bool back(std::shared_ptr<Coroutine>& node);

    bool remove(std::shared_ptr<Coroutine> node);
    void clear();

    bool is_empty() {
        return _list.size() == 0 ? true : false;
    }

    int size() {return (int)_list.size();}

//private:
//    void init_list(std::shared_ptr<Coroutine> node);
//    void append(std::shared_ptr<Coroutine> node);

private:
//    std::shared_ptr<Coroutine>   _header;
//    int     _size;

    typedef std::list<std::shared_ptr<Coroutine>>::iterator CO_LIST_ITER;

    std::list<std::shared_ptr<Coroutine>>   _list;
    std::map<CO_LIST_ITER>                  _map_list_iter;

};

#endif
