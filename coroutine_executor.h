#ifndef __COROUTINE_EXECUTOR_H__
#define __COROUTINE_EXECUTOR_H__

#include "coroutine.h"
#include "double_link_list.h"

#include <mutex>
#include <memory>
#include <exception>
#include <list>

using namespace std;

class CoroutineSchedule;

class CoroutineExecutor
{
public:
    CoroutineExecutor(); 
    ~CoroutineExecutor();

    bool run();
	void put(shared_ptr<Coroutine> coroutine);

private:
    void schedule(shared_ptr<Coroutine> coroutine) throw exception;

    shared_ptr<Coroutine> get_local_coroutine();
    vector<shared_ptr<Coroutine>> get_global_coroutine(int size = 1);

private:
	DoubleLinkList<shared_ptr<Coroutine>>	_list_wait;
	DoubleLinkList<shared_ptr<Coroutine>>	_list_ready;

    shared_ptr<Coroutine>   _running_coroutine;

    mutex   _mutex;
};

#endif
