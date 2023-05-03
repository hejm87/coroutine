#ifndef __UCONTEXT_HANDLE_H__
#define __UCONTEXT_HANDLE_H__

#include <ucontext.h>
#include <memory>
#include "../co_common.h"

typedef ucontext_t context_t;
typedef void (*context_func)(std::shared_ptr<void>);

extern thread_local context_t* g_ctx_main;

void init_context()
{
    getcontext(g_ctx_main);
}

inline void make_context(context_t* ctx, context_func func, shared_ptr<void> argv)
{
	int size = get_stack_size();

	ctx->uc_stack.ss_sp = malloc(size);
	ctx->uc_stack.ss_size = size;
	ctx->uc_stack.ss_flags = 0;

    makecontext(ctx, reinterpret_cast<void(*)()>(func), 1, argv);
}

inline int swap_context(context_t* source_ctx, context_t* dest_ctx)
{
    return swapcontext(source_ctx, dest_ctx);
}

#endif
