#ifndef __UCONTEXT_HANDLE_H__
#define __UCONTEXT_HANDLE_H__

#include "co_context.h"

class CoUContextHandle : public CoContext
{
public:
    bool init_context();
    bool swap_context(co_context_handle ctx_source, co_context_handle ctx_dest);

    co_context_handle create_context(context_func func, std::shared_ptr<void> argv);
    void release_context(co_context_handle ctx);
};

#endif
