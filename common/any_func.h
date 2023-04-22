#ifndef __ANY_FUNC_H__
#define __ANY_FUNC_H__

#include <functional>

using namespace std;

class AnyFunc
{
public:
	template <class F, class... Args>
	AnyFunc(F&& f, Args&&... args) {
		auto task = bind(forward<F>(f), forward<Args>(args)...);
		_func = [task]() -> Any {
			return task();
		};
	}
	
	Any operator()() {
		return _func();
	}

private:
	function<Any()> _func;
};

#endif
