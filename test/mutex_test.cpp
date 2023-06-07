#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../co_api.h"
#include "../mutex/co_mutex.h"
#include "test_define.h"

using namespace std;

int main()
{
	printf("pid:%d\n", getpid());
    Singleton<CoSchedule>::get_instance()->set_logger(test_logger);
    try {
        int value = 100;
        CoMutex mutex;
        for (int i = 0; i < 2; i++) {
            CoApi::create([&mutex, &value] {
                while (1) {
                    printf(
                        "[%s]tid:%d, cid:%d prepare lock\n", 
                        date_ms().c_str(), 
                        gettid(), 
                        CoApi::getcid()
                    );
                    mutex.lock();
                    if (value <= 0) {
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
            });
        }
        while (value != 0) {
            sleep(1);
        }
        printf("########### test finish\n");
    } catch (exception& ex) {
        printf("exception:%s\n", ex.what());
    }    
}
