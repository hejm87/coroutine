#ifndef __CO_MUTEX_H__
#define __CO_MUTEX_H__

#include <atomic>
#include <memory>
#include "../co_common/co_list.h"

class Coroutine;

class CoMutex
{
public:
    CoMutex();
    CoMutex(CoMutex&& obj);

    CoMutex& operator=(CoMutex&& obj);

    void lock();
    void unlock();

private:
    std::atomic<int>    _value;
    CoList              _block_list;
    std::shared_ptr<Coroutine>  _lock_co;
};

#endif
