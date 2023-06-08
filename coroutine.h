#ifndef __COROUTINE_H__
#define __COROUTINE_H__

#include <memory>
#include <functional>

#include "common/any.h"
#include "common/helper.h"
#include "context/co_context.h"

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
	CO_SUSPEND_LOCK,
	CO_SUSPEND_CHANNEL,
	CO_SUSPEND_IO_BLOCK,
};

struct CoParam
{
	int		type;		// coroutine param
	Any		value;
};

class Coroutine// : public std::enable_shared_from_this<Coroutine>
{
public:
    Coroutine(int id);
    ~Coroutine();

	bool init();

	void set_func(const std::function<void()>& f);

    co_context_handle get_context();

    void run();

	static void co_run(void* argv);

public:
	int		_id;				// 协程id
    int		_status;			// 协程状�?
	int		_suspend_status;	// 协程暂停状�?
	bool	_priority;			// 协程执�?�优先级

	std::function<void()>	_func;	// 协程执�?�函�?

	CoParam	_param;

private:
    co_context_handle		_ctx;
};

#endif
