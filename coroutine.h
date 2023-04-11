#ifndef __COROUTINE_H__
#define __COROUTINE_H__

#include <memory>
#include <exception>

#include "coroutine_common.h"
#include "context/ucontext_helper.h"

// coroutine status
enum {
    COROUTINE_IDLE = 0,
    COROUTINE_READY,
    COROUTINE_RUNNING,
    COROUTINE_SUSPEND,
    COROUTINE_FINISH,
}

class Coroutine
{
public:
    Coroutine(int index);
    ~Coroutine();

    context_t* get_context() {
        return &_ctx;
    } 

    Any run() throw exception;

public:
    int     _executor_index;    // 协程执行器索引
    int     _status;            // 协程状态
    int     _active_time;       // 下次活跃时间（单位：毫秒级别）

    shared_ptr<AnyFunction> _func;

private:
    context_t  _ctx;
    int         _index;
};

#endif
