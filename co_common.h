#ifndef __CO_COMMON_H__
#define __CO_COMMON_H__

#include <unistd.h>
#include <memory>

#include "context/co_context.h"

const int DEF_STACK_SIZE = 4 * 1024;

inline bool is_in_co_thread() {
    return g_ctx_main ? true : false;
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
    return stack_size;
}

#endif