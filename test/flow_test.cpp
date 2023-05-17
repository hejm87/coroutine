#include <stdio.h>
#include <stdlib.h>
#include "../co_api.h"

using namespace std;

int main()
{
    for (int index = 0; index < 5; index++) {
        CoApi::create([index]() {
            for (int i = 0; i < 10; i++) {
                printf("co[%d] value:%d\n", index, i);
                CoApi::sleep(100);
            }
            printf("co[%d] is end\n", index);
        });
    }
    while (1);
    return 0;
}