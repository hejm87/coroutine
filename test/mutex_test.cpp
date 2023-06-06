#include <stdio.h>
#include <stdlib.h>
#include "../co_api.h"
#include "../mutex/co_mutex.h"
#include "test_define.h"

using namespace std;

int main()
{
    Singleton<CoSchedule>::get_instance()->set_logger(test_logger);
    try {
        int value = 100;
        CoMutex mutex;
        for (int i = 0; i < 2; i++) {
            CoApi::create([&mutex, &value] {
                while (1) {
                    mutex.lock();
                    if (value <= 0) {
                        break ;
                    }
                    CO_LOG_INFO(
                        "tid:%d, cid:%d get ticket, left %d", 
                        gettid(),
                        CoApi::getcid(),
                        --value
                    );
                    mutex.unlock();
                    CoApi::sleep(10);
                }
            });
        }
        while (value != 0);
        printf("########### test finish\n");
    } catch (exception& ex) {
        printf("exception:%s\n", ex.what());
    }    
}
