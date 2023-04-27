#ifndef __UCONTEXT_HANDLE_H__
#define __UCONTEXT_HANDLE_H__

#include <ucontext.h>
#include "../co_define.h"

typedef ucontext_t context_t;

void init_context()
{
    getcontext(&g_main_ctx);

    int size = CoroutineSchedule::get_instance()->get_stack_size();
    ctx.uc_stack.ss_sp = malloc(size);
    ctx.uc_stack.ss_size = size;
    ctx.uc_stack.ss_flags = 0;
}

inline void make_context(context_t* ctx, context_func func, void* argv)
{
    makecontext(ctx, reinterpret_cast<void(*)()>(func), 1, argv);
}

inline int swap_context(context_t* source_ctx, context_t* dest_ctx)
{
    return swapcontext(source_ctx, dest_ctx);
}

#endif
