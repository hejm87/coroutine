#ifndef __CO_EXCEPTION_H__
#define __CO_EXCEPTION_H__

#include <map>
#include <string>
#include <exception>

#define DEF_KV(err) {err, #err}

const int CO_ERROR_UNKNOW = 1;
const int CO_ERROR_NO_RESOURCE = 2;           // ��Դ����
const int CO_ERROR_NO_EXIST_DATA = 3;         // û������
const int CO_ERROR_CHANNEL_CLOSE = 4;         // channel�ѹر�
const int CO_ERROR_SCHEDULE = 5;              // Э�̵���ʧ��
const int CO_ERROR_NOT_IN_CO_THREAD = 6;      // ������Э�̹����߳�
const int CO_ERROR_INVALID_CO_EXECUTOR = 7;   // ��Ч��Э�̵�����
const int CO_ERROR_COROUTINE_EXCEPTION = 8;   // Э����Ϣ�쳣
const int CO_ERROR_SET_TIMER = 9;             // ���ö�ʱ���쳣
const int CO_ERROR_PACKAGE_FUNCTION = 10;     // ��װ�����쳣

static map<int, string> s_errors = 
{
    DEF_KV(CO_ERROR_NO_RESOURCE),
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