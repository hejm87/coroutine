#ifndef __CO_DEFINE_H__
#define __CO_DEFINE_H__

#include <memory>
#include "common/maplist.h"

class Coroutine;

typedef MapList<std::shared_ptr<Coroutine>> CoList;

#endif
