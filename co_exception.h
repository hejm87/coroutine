#ifndef __CO_EXCEPTION_H__
#define __CO_EXCEPTION_H__

#include <map>
#include <string>
#include <exception>

#define DEF_KV(err) {err, #err}

//const int CO_ERROR_UNKNOW = 1;
//const int CO_ERROR_PARAM_INVALID = 2;           // 无效参数
//const int CO_ERROR_NO_RESOURCE = 3;             // 缺乏资源
//const int CO_ERROR_NO_EXIST_DATA = 4;           // 不存在数据
//const int CO_ERROR_CHANNEL_CLOSE = 5;           // channel已关闭
//const int CO_ERROR_SCHEDULE = 6;                // 调度失败
//const int CO_ERROR_NOT_IN_CO_THREAD = 7;        // 不在协程线程
//const int CO_ERROR_INVALID_CO_EXECUTOR = 8;     // 无效协程调度器
//const int CO_ERROR_COROUTINE_EXCEPTION = 9;     // 协程异常
//const int CO_ERROR_SET_TIMER = 10;              // 设置定时器失败
//const int CO_ERROR_PACKAGE_FUNCTION = 11;       // 无效处理函数
//const int CO_ERROR_UNLOCK_EXCEPTION = 12;       // 解锁错误
//const int CO_ERROR_AWAIT_CALL_EXCEPTION = 13;   // await调用异常

enum {
    CO_ERROR_UNKNOW = 1,              // 未知错误
    CO_ERROR_INIT_ENV_FAIL,           // 环境初始化失败
    CO_ERROR_PARAM_INVALID,           // 无效参数
    CO_ERROR_NO_RESOURCE,             // 缺乏资源
    CO_ERROR_NO_EXIST_DATA,           // 不存在数据
    CO_ERROR_CHANNEL_CLOSE,           // channel已关闭
    CO_ERROR_SCHEDULE,                // 调度失败
    CO_ERROR_NOT_IN_CO_THREAD,        // 不在协程线程
    CO_ERROR_INVALID_CO_EXECUTOR,     // 无效协程调度器
    CO_ERROR_COROUTINE_EXCEPTION,     // 协程异常
    CO_ERROR_SET_TIMER,               // 设置定时器失败
    CO_ERROR_PACKAGE_FUNCTION,        // 无效处理函数
    CO_ERROR_UNLOCK_EXCEPTION,        // 解锁异常
    CO_ERROR_AWAIT_CALL_EXCEPTION,    // await调用异常
};

static std::map<int, std::string> s_errors = 
{
    DEF_KV(CO_ERROR_UNKNOW),
    // !!! 继续补充
};

class CoException : public std::exception
{
public:
    CoException(int error) {
        _error = error;
    }

    const char* what() {
        auto iter = s_errors.find(_error);
        return iter != s_errors.end() ? iter->second.c_str() : "CO_ERROR_UNKNOW";
    }

private:
    int _error;
};

#endif