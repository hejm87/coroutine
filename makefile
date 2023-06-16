OBJS=obj/co_ucontext_handle.o obj/co_mutex.o obj/co_executor.o obj/co_schedule.o obj/coroutine.o
LIB_DIR=./lib

all: mkobj $(OBJS) make_lib make_test

mkobj:
	test -d ./obj || mkdir -p ./obj
	test -d ./test || mkdir -p ./test
	test -d $(LIB_DIR) || mkdir -p $(LIB_DIR)

make_lib:
	ar -rc $(LIB_DIR)/libco.a $(OBJS)

make_test:
	cd test && rm -f flow_test && g++ -std=c++11 -g -o flow_test flow_test.cpp ../lib/libco.a -lpthread
	cd test && rm -f mutex_test && g++ -std=c++11 -g -o mutex_test mutex_test.cpp ../obj/co_mutex.o ../lib/libco.a -lpthread
	cd test && rm -f channel_test && g++ -std=c++11 -g -o channel_test channel_test.cpp ../obj/co_mutex.o ../lib/libco.a -lpthread

obj/co_timer.o: co_common/co_timer.cpp
	g++ -std=c++11 -g -o $@ -c $<

obj/co_ucontext_handle.o: context/co_ucontext_handle.cpp
	g++ -std=c++11 -g -o $@ -c $<

obj/co_mutex.o: mutex/co_mutex.cpp
	g++ -std=c++11 -g -o $@ -c $<

obj/co_executor.o: co_executor.cpp
	g++ -std=c++11 -g -o $@ -c $<

obj/co_schedule.o: co_schedule.cpp
	g++ -std=c++11 -g -o $@ -c $<

obj/coroutine.o: coroutine.cpp
	g++ -std=c++11 -g -o $@ -c $<

clean:
	rm -f ${LIB_DIR}/libco.a
	rm -rf obj
