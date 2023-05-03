#ifndef __UCONTEXT_HANDLE_H__
#define __UCONTEXT_HANDLE_H__

#include "co_context.h"

class CoUcontextHandle : public CoContext
{
public:
    bool init_context();
    bool swap_context(co_context_t* ctx_source, co_context_t* ctx_dest);

    co_context_t* create_context(context_func func, shared_ptr<void> argv);
    void release_context(co_context_t* ctx);
};

#endif