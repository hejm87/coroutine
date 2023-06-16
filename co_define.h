#ifndef __CO_DEFINE_H__
#define __CO_DEFINE_H__

#include <memory>
#include "common/maplist.h"

const int DEF_STACK_SIZE = 4 * 1024;
const int DEF_COROUTINE_COUNT = 10000;
const int DEF_TIMER_THREAD_COUNT = 1;

struct CoInfo
{
    int     stack_size;
    int     executor_count;
    int     coroutine_count;
};

class Coroutine;

typedef MapList<std::shared_ptr<Coroutine>> CoList;

#endif
