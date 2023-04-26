#ifndef __ANY_FUNC_H__
#define __ANY_FUNC_H__

#include <functional>
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
	
	Any operator()() {
		if (!_set_func) {
			throw CoException(CO_ERROR_PACKAGE_FUNCTION);
		}
		return _func();
	}

private:
	bool			_set_func;
	function<Any()> _func;
};

#endif
