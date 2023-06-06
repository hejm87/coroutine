#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include "../co_api.h"
#include "test_define.h"

using namespace std;

int main()
{
    Singleton<CoSchedule>::get_instance()->set_logger(test_logger);
    try {
        atomic<int> counter(0);
        for (int index = 0; index < 5; index++) {
            CoApi::create([index, &counter]() {
                for (int i = 0; i < 10; i++) {
                    CO_LOG_INFO("co[%d] value:%d", index, i);
                    counter++;
                    CoApi::sleep(100);
                }
                CO_LOG_INFO("co[%d] is end", index);
            });
        }
        while (counter < 50);
        CoApi::create([] {
            CO_LOG_INFO("final coroutine");
        });
        while (1);
    } catch (exception& ex) {
        printf("########## exception:%s\n", ex.what());
    }
    return 0;
}
