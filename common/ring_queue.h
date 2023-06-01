#ifndef __RING_QUEUE_H__
#define __RING_QUEUE_H__

#include <vector>

template <class T>
class RingQueue
{
public:
    RingQueue(int size) {
        _beg = 0;
        _end = 0;
        _cur_size = 0;
        _queue.resize(size);
    }

    bool push(const T& obj) {
        if (_cur_size == (int)_queue.size()) {
            return false;
        }   
        _queue[_end] = obj;
        _end = ++_end % _queue.size();
        _cur_size++;
        return true;
    }

    bool front(T& obj) {
        if (_cur_size == 0) {
            return false;
        }   
        obj = _queue[_beg];
        return true;
    }

    void pop() {
        if (_cur_size == 0) {
            return ;
        }
        _beg = ++_beg % _queue.size();
        _cur_size--;
    }

    bool is_full() {
        return (_cur_size == (int)_queue.size() ? true : false);
    }

    bool is_empty() {
        return _cur_size == 0 ? true : false;
    }

    int size() {
        return (int)_queue.size();
    }

    int cur_size() {
        return _cur_size;
    }

private:
    int     _beg;
    int     _end;
    int     _cur_size;
    std::vector<T>  _queue;
};

#endif