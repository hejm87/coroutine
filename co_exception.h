#ifndef __CO_EXCEPTION_H__
#define __CO_EXCEPTION_H__

#include <string>
#include <exception>

#define DEF_KV(err) {err, #err}

enum
{
    CO_ERROR_UNKNOW = 1,
    CO_ERROR_NO_RESOURCE,           // ��Դ����
    CO_ERROR_NO_EXIST_DATA,         // û������
    CO_ERROR_CHANNEL_CLOSE,         // channel�ѹر�
    CO_ERROR_SCHEDULE,              // Э�̵���ʧ��
    CO_ERROR_NOT_IN_CO_THREAD,      // ������Э�̹����߳�
    CO_ERROR_INVALID_CO_EXECUTOR,   // ��Ч��Э�̵�����
    CO_ERROR_COROUTINE_EXCEPTION,   // Э����Ϣ�쳣
    CO_ERROR_SET_TIMER,             // ���ö�ʱ���쳣
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