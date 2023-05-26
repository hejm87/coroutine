OBJS=obj/co_ucontext_handle.o obj/co_mutex.o obj/co_executor.o obj/co_schedule.o
LIB_DIR=./lib

all: mkobj $(OBJS) make_lib

mkobj:
	test -d ./obj || mkdir -p ./obj
	test -d $(LIB_DIR) || mkdir -p $(LIB_DIR)

make_lib:
	ar -rc $(LIB_DIR)/libco.a $(OBJS)

obj/co_timer.o: co_common/co_timer.cpp
	g++ -std=c++11 -o $@ -c $<

obj/co_ucontext_handle.o: context/co_ucontext_handle.cpp
	g++ -std=c++11 -o $@ -c $<

obj/co_mutex.o: mutex/co_mutex.cpp
	g++ -std=c++11 -o $@ -c $<

obj/co_executor.o: co_executor.cpp
	g++ -std=c++11 -o $@ -c $<

obj/co_schedule.o: co_schedule.cpp
	g++ -std=c++11 -o $@ -c $<

clean:
	rm -f ${LIB_DIR}/libco.a
	rm -rf obj
