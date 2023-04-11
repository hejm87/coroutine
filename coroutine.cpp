#include "coroutine.h"

Coroutine::Coroutine(int index)
{
    _index = index;    
    _executor_index = -1;
    _status = COROUTINE_IDLE;
    _active_time = 0;
}

Coroutine::~Coroutine()
{
    _func.reset();
    _prev = NULL;
    _next = NULL;
}

Any Coroutine::run() throw exception
{
    if (_func) {
        throw runtime_error("not exist _func");
    }
    return _func();
}
