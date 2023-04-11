#ifndef __COROUTINE_COMMON_H__
#define __COROUTINE_COMMON_H__

#include <functional>

using namespace std;

#define coroutine_id    int

class Any
{

};

class AnyFunction
{
public:
    virtual Any run() = 0;
};

template <class T>
class AnyFunctionT : public AnyFunction
{
public:
    Any run();

public:
    function<T()> _func;
};

#endif