#ifndef __CO_TOOLS_H__
#define __CO_TOOLS_H__

#include <unistd.h>
#include <memory>

#include "../co_define.h"
#include "../context/co_context.h"

inline bool is_in_co_thread() {
    return g_ctx_main ? true : false;
}

inline int get_timer_thread_count() {
    int count = DEF_TIMER_THREAD_COUNT;    
    char* p = getenv("CO_TIMER_THREAD_COUNT");
    if (p) {
        count = atoi(p);
    }
    return count;
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
    int size = DEF_STACK_SIZE;
    char* p = getenv("CO_STACK_SIZE");
    if (p) {
        size = atoi(p);
    }
    return size;
}

inline int get_coroutine_count() {
    int count = DEF_COROUTINE_COUNT;
    char* p = getenv("COROUTINE_COUNT");
    if (p) {
        count = atoi(p);
    }
    return count;
}

#endif