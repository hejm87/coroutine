//#ifndef __COROUTINE_H__
//#define __COROUTINE_H__

#ifndef __A_H__
#define __A_H__

#include <memory>
#include <future>
#include <exception>

#include "common/any.h"
#include "common/any_func.h"
#include "context/ucontext_handle.h"

#include "co_schedule.h"

// coroutine status
enum {
    CO_STATUS_IDLE = 0,
    CO_STATUS_READY,
    CO_STATUS_RUNNING,
    CO_STATUS_SUSPEND,
    CO_STATUS_FINISH,
};

// coroutine param
enum {
	CO_PARAM_NIL = 0,
	CO_PARAM_CHANNEL_SEND,
	CO_PARAM_CHANNEL_RECV,
};

struct CoParam
{
	int		type;		// coroutine param
	Any		value;
};

class Coroutine
{
public:
    Coroutine() {
		_status = CO_STATUS_IDLE;	

		getcontext(&_ctx);

		int size = CoSchedule::get_instance()->get_stack_size();
		_stack = malloc(size);

		_ctx.uc_stack.ss_sp = _stack;
		_ctx.uc_stack.ss_size  = size;
		_ctx.uc_stack.ss_flags = 0;
	}
    ~Coroutine() {
		free(_stack);	
	}

	void set_func(const AnyFunc& f) {
		_func = f;
	}

    context_t* get_context() {
        return &_ctx;
    } 

    void run() {
		_result = _func();
		_status = CO_STATUS_FINISH;
	}

public:
    int		_status;	// 协程状态
	bool	_priority;	// 优先执行

	AnyFunc _func;		// 协程执行函数
	Any		_result;	// 协程执行结果

	CoParam	_param;

	shared_ptr<Coroutine> 	_prev;
	shared_ptr<Coroutine> 	_next;

	weak_ptr<CoExecutor>	_co_executor;

private:
    context_t	_ctx;

	void*		_stack;
};

#endif
