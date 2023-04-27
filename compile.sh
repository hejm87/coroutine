g++ -std=c++11 -g -o co_executor.o -c co_executor.cpp
g++ -std=c++11 -g -o co_schedule.o -c co_schedule.cpp

g++ -std=c++11 -g -o co_list.o -c co_common/co_list.cpp
g++ -std=c++11 -g -o co_timer.o -c co_common/co_timer.cpp

ar crv libco.a co_common.o co_executor.o co_schedule.o co_list.o co_timer.o
