#include "co_list.h"

using namespace std;

void CoList::push_front(shared_ptr<Coroutine> node)
{
    append(node);
    _header = node;
}

void CoList::push_back(shared_ptr<Coroutine> node)
{
    append(node);
}

bool CoList::pop_front(shared_ptr<Coroutine>& node)
{
    if (is_empty()) {
        return false;
    }
    return remove(_header);
}

bool CoList::pop_back(shared_ptr<Coroutine>& node)
{
    if (is_empty()) {
        return false;
    }
    return remove(_header->prev);
}

bool CoList::front(shared_ptr<Coroutine>& node)
{
    if (is_empty()) {
        return false;
    }
    node = _header;
    return true;
}

bool CoList::back(shared_ptr<Coroutine>& node)
{
    if (is_empty()) {
        return false;
    }
    node = _header->prev;
    return true;
}

bool CoList::remove(shared_ptr<Coroutine> node)
{
    if (!node->prev || !node->next) {
        return false;
    }

    auto prev = node->prev;
    auto next = node->next;

    node->prev.reset();
    node->next.reset();

    if (prev == next) {
        _header.reset();
        return true;
    }

    prev->next = next;
    next->prev = prev;
    if (_header == node) {
        _header = next;
    }
    return true;
}

void CoList::clear()
{
    // ??? 需要自行编写代码
}

void init_list(shared_ptr<Coroutine>& node)
{
    node->prev = node;
    node->next = node;
    _header = node;
}

void append(shared_ptr<Coroutine> node)
{
    if (!_header) {
        init_list(node);
        return ;
    }

    auto last = _header->prev;

    node->next = _header;
    node->prev = _header->prev;

    last->next = node;

    _header->prev = node;
    if (_header == _header->next) {
        _header->next = node;
    }
}