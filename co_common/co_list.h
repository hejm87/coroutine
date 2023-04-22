#ifndef __CO_LIST_H__
#define __CO_LIST_H__

#include <memory>
#include "../coroutine.h"

class CoList
{
public:
    CoList() {
        _size = 0;
    }

    void push_front(std::shared_ptr<Coroutine> node);
    void push_back(std::shared_ptr<Coroutine> node);

    bool pop_front(std::shared_ptr<Coroutine>& node);
    bool pop_back(std::shared_ptr<Coroutine>& node);

    bool front(std::shared_ptr<Coroutine>& node);
    bool back(std::shared_ptr<Coroutine>& node);

    bool remove(std::shared_ptr<Coroutine> node)
    void clear();

    bool is_empty() {return _header ? true : false;}

    int size() {return _size;}

private:
    void init_list(std::shared_ptr<Coroutine> node);
    void append(std::shared_ptr<Coroutine> node);

private:
    std::shared_ptr<Coroutine>   _header;
    int     _size;
};

#endif