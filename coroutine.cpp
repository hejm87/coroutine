#include "coroutine.h"
#include "co_define.h"
#include "co_schedule.h"

Coroutine::Coroutine(int id) {
	_id = id;
	_status = CO_STATUS_IDLE;	
	_ctx = g_ctx_handle->create_context(Coroutine::co_run, shared_from_this());
}

Coroutine::~Coroutine() {
	g_ctx_handle->release_context(_ctx);
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

void Coroutine::co_run(std::shared_ptr<void>& ptr) {
	auto co_ptr = std::static_pointer_cast<Coroutine>(ptr);
	while (!Singleton<CoSchedule>::get_instance()->is_set_end()) {
		co_ptr->run();
		Singleton<CoSchedule>::get_instance()->yield();
	}
}