#ifndef __CO_COMMON_H__
#define __CO_COMMON_H__

#include <stdlib.h>
#include <memory>

#include "coroutine.h"

inline bool check_in_co_thread() {
    return _g_ctx_main ? true : false;
}

inline int get_executor_count() {
    int count;
    char* p = getenv("CO_EXECUTOR_COUNT");
    if (!p) {
        count = sysconf(_SC_NPROCESSORS_ONLN);
    } else {
        count = atoi(p);
    }
    return count;
}

inline int get_stack_size() {
    int stack_size = DEF_STACK_SIZE;
    char* p = getenv("CO_STACK_SIZE");
    if (p) {
        stack_size = atoi(p);
    }
    return stack_size
}

#endif