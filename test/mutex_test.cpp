#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <atomic>
#include "../co_api.h"
#include "../mutex/co_mutex.h"
#include "test_define.h"

using namespace std;

int main()
{
	printf("pid:%d\n", getpid());
    atomic<int> end_count(0);
    int value = 100;
    CoMutex mutex;
    Singleton<CoSchedule>::get_instance()->set_logger(test_logger);
    try {
        for (int i = 0; i < 2; i++) {
            CoApi::create([&mutex, &value, &end_count] {
                while (1) {
                    printf(
                        "[%s]tid:%d, cid:%d prepare lock\n", 
                        date_ms().c_str(), 
                        gettid(), 
                        CoApi::getcid()
                    );
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
                    mutex.unlock();
                    printf(
                        "[%s]tid:%d, cid:%d unlock\n", 
                        date_ms().c_str(), 
                        gettid(), 
                        CoApi::getcid()
                    );
                    CoApi::sleep(2);
                    printf(
                        "[%s]tid:%d, cid:%d after sleep\n", 
                        date_ms().c_str(), 
                        gettid(), 
                        CoApi::getcid()
                    );
                }
                end_count++;
            });
        }
        while (1) {
            if (value == 0 && end_count == 2) {
                break ;
            }
        }
        printf("########### test finish\n");
    } catch (exception& ex) {
        printf("exception:%s\n", ex.what());
    }
    printf("########## test end\n");
    return 0;
}
