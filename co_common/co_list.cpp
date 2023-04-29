#include "co_list.h"
#include "../coroutine.h"

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

void CoList::pop_front()
{
	if (!is_empty()) {
		remove(_header);
	}
}

void CoList::pop_back()
{
	if (!is_empty()) {
		remove(_header->_prev);
	}
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
    node = _header->_prev;
    return true;
}

bool CoList::remove(shared_ptr<Coroutine> node)
{
    if (!node->_prev || !node->_next) {
        return false;
    }

    auto prev = node->_prev;
    auto next = node->_next;

    node->_prev.reset();
    node->_next.reset();

    if (prev == next) {
        _header.reset();
        return true;
    }

    prev->_next = next;
    next->_prev = prev;
    if (_header == node) {
        _header = next;
    }
    return true;
}

void CoList::clear()
{
    // ??? 需要自行编写代�?
}

void CoList::init_list(shared_ptr<Coroutine> node)
{
    node->_prev = node;
    node->_next = node;
    _header = node;
}

void CoList::append(shared_ptr<Coroutine> node)
{
    if (!_header) {
        init_list(node);
        return ;
    }

    auto last = _header->_prev;

    node->_next = _header;
    node->_prev = _header->_prev;

    last->_next = node;

    _header->_prev = node;
    if (_header == _header->_next) {
        _header->_next = node;
    }
}
