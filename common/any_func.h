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
		;
	}

	template <class F, class... Args>
	AnyFunc(F&& f, Args&&... args) {
		create_func(std::forward<F>(f), std::forward<Args>(args)...);
	}

	AnyFunc(AnyFunc& obj) {
		_is_return_void = obj._is_return_void;
		_func = obj._func;
	}

	AnyFunc(const AnyFunc& obj) {
		_is_return_void = obj._is_return_void;
		_func = obj._func;
	}

	AnyFunc& operator=(const AnyFunc& obj) {
		_is_return_void = obj._is_return_void;
		_func = obj._func;
	}
	
	Any operator()() const throw (CoException) {
		if (!_func) {
			throw CoException(CO_ERROR_PACKAGE_FUNCTION);
		}
		return _func();
	}

private:
	template <class F, class... Args>
	auto create_func(F&& f, Args&&... args) -> typename std::enable_if<std::is_void<typename std::result_of<F(Args...)>::type>::value>::type {
		auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
		_func = [task]() -> Any {
			task();
			return Any();
		};
		_is_return_void = true;
	}

	template <class F, class... Args>
	auto create_func(F&& f, Args&&... args) -> typename std::enable_if<!std::is_void<typename std::result_of<F(Args...)>::type>::value>::type {
		auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
		_func = [task]() -> Any {
			return task();
		};
		_is_return_void = false;
	}

private:
	bool				_is_return_void;
	function<Any()>		_func;
};

#endif