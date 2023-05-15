OBJS=obj/co_list.o obj/co_timer.o obj/co_ucontext_handle.o obj/co_mutex.o obj/co_executor.o obj/co_schedule.o

all: $(OBJS)
	ar -rc libco.a $(OBJS)

obj/co_list.o: co_common/co_list.cpp
	g++ -std=c++11 -o $@ -c $<

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
	rm -f libco.a obj/*.o
