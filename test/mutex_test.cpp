#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <atomic>
#include "../co_api.h"
#include "../mutex/co_mutex.h"
#include "test_define.h"

using namespace std;

const int g_coroutine_count = 2;
const int g_ticket_count = 100;

int main()
{
	printf("pid:%d\n", getpid());
    int value = g_ticket_count;
    atomic<int> end_count(0);
    atomic<int> total_count(0);
    CoMutex mutex;
    Singleton<CoSchedule>::get_instance()->set_logger(test_logger);
    try {
        for (int i = 0; i < g_coroutine_count; i++) {
            CoApi::create([&mutex, &value, &end_count, &total_count] {
                while (1) {
                   // printf(
                   //     "[%s]tid:%d, cid:%d prepare lock\n", 
                   //     date_ms().c_str(), 
                   //     gettid(), 
                   //     CoApi::getcid()
                   // );
                    mutex.lock();
                    if (value <= 0) {
                        printf(
                            "[%s]tid:%d, cid:%d break\n",
                            date_ms().c_str(),
                            gettid(),
                            CoApi::getcid()
                        );
                        mutex.unlock();
                        break ;
                    }
                    printf(
                        "[%s]tid:%d, cid:%d get ticket, left %d\n", 
                        date_ms().c_str(),
                        gettid(),
                        CoApi::getcid(),
                        --value
                    );
                    total_count++;
                    mutex.unlock();
                   // printf(
                   //     "[%s]tid:%d, cid:%d unlock\n", 
                   //     date_ms().c_str(), 
                   //     gettid(), 
                   //     CoApi::getcid()
                   // );
                   // CoApi::sleep(2);
                   // printf(
                   //     "[%s]tid:%d, cid:%d after sleep\n", 
                   //     date_ms().c_str(), 
                   //     gettid(), 
                   //     CoApi::getcid()
                   // );
                }
                end_count++;
            });
        }
        CoInfo info;
        CoApi::get_info(info);
        while (1) {
            if (value == 0 && end_count == g_coroutine_count) {
                break ;
            }
            usleep(100);
        }
        assert(total_count == g_ticket_count);
        printf("########### test finish\n");
    } catch (exception& ex) {
        printf("exception:%s\n", ex.what());
    } catch (...) {
        printf("unknow exception\n");
    }
    printf("########## test end\n");
    usleep(100);
    return 0;
}
