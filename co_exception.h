#ifndef __CO_EXCEPTION_H__
#define __CO_EXCEPTION_H__

#include <string>
#include <exception>

#define DEF_KV(err) {err, #err}

enum
{
    CO_ERROR_UNKNOW = 1,
    CO_ERROR_NO_RESOURCE,           // 资源不足
    CO_ERROR_NO_EXIST_DATA,         // 没有数据
    CO_ERROR_CHANNEL_CLOSE,         // channel已关闭
    CO_ERROR_SCHEDULE,              // 协程调度失败
    CO_ERROR_NOT_IN_CO_THREAD,      // 不处于协程工作线程
    CO_ERROR_INVALID_CO_EXECUTOR,   // 无效的协程调度器
    CO_ERROR_COROUTINE_EXCEPTION,   // 协程信息异常
    CO_ERROR_SET_TIMER,             // 设置定时器异常
}

static map<int, string> s_errors = 
{
    DEF_KV(CO_ERROR_NO_RESOURCE),
};

class CoException : public exception
{
public:
    CoException(int errno) {
        _errno = errno;
    }

    const char* what() {
        auto iter = s_errors.find(_errno);
        return iter != s_errors.end() ? iter->second.c_str() : "CO_ERROR_UNKNOW";
    }

private:
    int _errno;
};

#endif