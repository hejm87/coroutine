#include "coroutine.h"
#include "co_define.h"
#include "co_schedule.h"

Coroutine::Coroutine(int id) {
	_id = id;
	_status = CO_STATUS_IDLE;	
}

Coroutine::~Coroutine() {
	g_ctx_handle->release_context(_ctx);
}

bool Coroutine::init() {
	_ctx = g_ctx_handle->create_context(
		Coroutine::co_run, 
		reinterpret_cast<void*>(_id)
	);
	return true;
}

void Coroutine::set_func(const function<void()>& f) {
	_func = f;
}

co_context_handle Coroutine::get_context() {
    return _ctx;
} 

void Coroutine::run() {
	_func();
	_status = CO_STATUS_FINISH;
}

void Coroutine::co_run(void* argv) {
	auto index = reinterpret_cast<long>(argv);
	auto co = Singleton<CoSchedule>::get_instance()->get_coroutine(index);
	if (!co) {
		throw CoException(CO_ERROR_INIT_ENV_FAIL);
	}
	while (!Singleton<CoSchedule>::get_instance()->is_set_end()) {
		co->run();
		Singleton<CoSchedule>::get_instance()->release();
	}
}