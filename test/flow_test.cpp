#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include "../co_api.h"

using namespace std;

void test_logger(int level, const char* msg)
{
    static map<int, string> s_level = {
        {CO_LEVEL_DEBUG, "debug"},
        {CO_LEVEL_INFO, "info"},
        {CO_LEVEL_WARN, "warn"},
        {CO_LEVEL_ERROR, "error"},
        {CO_LEVEL_FATAL, "fatal"}
    };
    auto iter = s_level.find(level);
    if (iter != s_level.end()) {
        printf("%s|%s\n", iter->second.c_str(), msg);
    }
}

int main()
{
    try {
        Singleton<CoSchedule>::get_instance()->set_logger(test_logger);

        atomic<int> counter(0);
        for (int index = 0; index < 5; index++) {
            CoApi::create([index, &counter]() {
                for (int i = 0; i < 10; i++) {
                    printf("co[%d] value:%d\n", index, i);
                    counter++;
                    CoApi::sleep(100);
                }
                printf("co[%d] is end\n", index);
            });
        }
        while (counter < 50);
        CoApi::create([] {
            printf("final coroutine ...\n");
        });
        while (1);
    } catch (exception& ex) {
        printf("########## exception:%s\n", ex.what());
    }
    return 0;
}
