#ifndef __COROUTINE_DEFINE_H__
#define __COROUTINE_DEFINE_H__

const int DEF_STACK_SIZE = 1024 * 4;

void (*context_func)(void* argv);

#endif