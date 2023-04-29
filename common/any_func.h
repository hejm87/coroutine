#ifndef __ANY_FUNC_H__
#define __ANY_FUNC_H__

#include <functional>
#include <exception>
#include "any.h"
#include "../co_exception.h"

class AnyFunc
{
public:
	AnyFunc() {
		_set_func = false;
	}

	AnyFunc(const AnyFunc& obj) {
		_set_func = obj._set_func;
		_func = obj._func;
	}

	AnyFunc(AnyFunc&& obj) {
		_set_func = obj._set_func;
		std::swap(_func, obj._func);
	}

	template <class F, class... Args>
	AnyFunc(F&& f, Args&&... args) {
		auto task = bind(forward<F>(f), forward<Args>(args)...);
		_func = [task]() -> Any {
			return task();
		};
		_set_func = true;
	}

	AnyFunc& operator=(const AnyFunc& obj) {
		_set_func = obj._set_func;
		_func = obj._func;
	}
	
	Any operator()() throw(std::exception) {
		if (!_set_func) {
			throw std::runtime_error("package function not exists");
		}
		return _func();
	}

private:
	bool			_set_func;
	function<Any()> _func;
};

#endif
