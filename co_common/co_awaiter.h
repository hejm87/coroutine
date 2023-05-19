#ifndef __CO_AWAITER_H__
#define __CO_AWAITER_H__

#include <algorithm>
#include <memory>
#include "co_tools.h"
#include "../common/any.h"
#include "../channel/co_channel.h"

// 等待协程完成
class CoAwaiter
{
friend class CoSchedule;
public:
	CoAwaiter() {
		;
	}

	~CoAwaiter() {
		;
	}

	CoAwaiter(CoAwaiter&& obj) {
		std::swap(_wait_future, obj._wait_future);
		std::swap(_wait_chan, obj._wait_chan);
	}

	CoAwaiter& operator=(CoAwaiter&& obj) {
		std::swap(_wait_future, obj._wait_future);
		std::swap(_wait_chan, obj._wait_chan);
		return *this;
	}

	template <typename T>
	T await() {
		Any obj;
		if (is_in_co_thread() == true) {
			obj = _wait_future.wait();
		} else {
			_wait_chan >> obj;
		}
		return obj.AnyCast<T>();
	}

private:
	bool                _is_call_on_co_thread;
	std::future<Any>    _wait_future;
	CoChannel<Any>      _wait_chan;
};

#endif
