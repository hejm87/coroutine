#ifndef __CO_CONTEXT_H__
#define __CO_CONTEXT_H__

#include <memory>

typedef void* co_context_handle;
typedef void (*context_func)(std::shared_ptr<void>);

class CoContext
{
public:
    virtual bool init_context() = 0;
    virtual bool swap_context(co_context_handle ctx_source, co_context_handle ctx_dest) = 0;

    virtual co_context_handle create_context(context_func func, std::shared_ptr<void> argv) = 0;
    virtual void release_context(co_context_handle ctx) = 0;
};

extern thread_local co_context_handle g_ctx_main;
extern CoContext* g_ctx_handle;

#endif