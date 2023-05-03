#include <ucontext.h>
#include "co_ucontext_handle.h"
#include "../co_common.h"

thread_local context_t* g_ctx_main = NULL;

g_ctx_handle = new CoUcontextHandle;

bool CoUcontextHandle::init_context()
{
    auto ctx = (ucontext_t*)malloc(sizeof(ucontext_t));
    auto res = getcontext(ctx);
    if (res != 0) {
        return false;
    }
    g_ctx_main = ctx;
    return true;
}

bool CoUcontextHandle::swap_context(co_context_t* ctx_source, co_context_t* ctx_dest)
{
    return swapcontext(ctx_source, ctx_dest) == 0 ? true : false;
}

co_context_t* CoUcontextHandle::create_context(context_func func, shared_ptr<void> argv)
{
    int stack_size = get_stack_size();

    auto ctx = (ucontext_t*)malloc(sizeof(ucontext_t))
    auto res = getcontext(ctx);

    ctx->uc_stack.ss_sp = malloc(stack_size);
    ctx->uc_stack.ss_size = stack_size;
    ctx->uc_stack.ss_flags = 0;

    makecontext(ctx, reinterpret_cast<void(*)()>(func), 1, argv);

    return (co_context_t*)ctx;
}

void CoUcontextHandle::release_context(co_context_t* ctx)
{
    free(((ucontext_t*)ctx)->uc_stack.ss_sp);
    free((ucontext_t*)ctx);
}