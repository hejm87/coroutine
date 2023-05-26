#ifndef __COROUTINE_H__
#define __COROUTINE_H__

#include <memory>
#include <future>
#include <exception>

#include "common/any.h"
#include "common/any_func.h"
#include "common/helper.h"
#include "context/co_context.h"
#include "co_executor.h"
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

// coroutine suspend type
enum {
	CO_SUSPEND_SLEEP= 0,
	CO_SUSPEND_IO_BLOCK,
};

struct CoParam
{
	int		type;		// coroutine param
	Any		value;
};

class Coroutine : std::enable_shared_from_this<Coroutine>
{
public:
    Coroutine(int id) {
		_id = id;
		_status = CO_STATUS_IDLE;	
		_ctx = g_ctx_handle->create_context(Coroutine::co_run, shared_from_this());
	}
    ~Coroutine() {
		g_ctx_handle->release_context(_ctx);
	}

	void set_func(const AnyFunc& f) {
		_func = f;
	}

    co_context_handle get_context() {
        return _ctx;
    } 

    void run() {
		_result = _func();
		_status = CO_STATUS_FINISH;
	}

	static void co_run(std::shared_ptr<void>& ptr) {
		auto co_ptr = std::static_pointer_cast<Coroutine>(ptr);
		while (!Singleton<CoSchedule>::get_instance()->is_set_end()) {
			co_ptr->run();
			Singleton<CoSchedule>::get_instance()->yield();
		}
	}

public:
	int		_id;				// 协程id
    int		_status;			// 协程状态
	int		_suspend_status;	// 协程暂停状态
	bool	_priority;			// 协程执行优先级

	AnyFunc _func;		// 协程执行函数
	Any		_result;	// 协程执行结果

	CoParam	_param;

//	weak_ptr<CoExecutor>	_co_executor;

private:
    co_context_handle		_ctx;
};

#endif
