#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <atomic>
#include "../co_api.h"
#include "../channel/co_channel.h"
#include "test_define.h"

using namespace std;

atomic<int>  g_end_count(0);
atomic<int>  g_acc_write(0);
atomic<int>  g_acc_read(0);

void channel_reader(CoChannel<int>& chan);
void channel_writer(CoChannel<int>& chan);

int main()
{
    int  value = 0;
    bool co_is_end = false;
    Singleton<CoSchedule>::get_instance()->set_logger(test_logger);
    try {
        CoChannel<int> chan;
        CoApi::create(channel_writer, ref(chan));
        CoApi::create(channel_reader, ref(chan));
        while (g_end_count != 2) {
            usleep(100);
        }
        assert(g_acc_write == g_acc_read);
        printf("test finish\n");
    } catch (exception& ex) {
        printf("exception:%s\n", ex.what());
    } catch (...) {
        printf("unknow exception\n");
    }
    return 0;
}

void channel_reader(CoChannel<int>& chan)
{
    printf("############### READER|cid:%d, reader is running\n", CoApi::getcid());
    try {
        while (1) {
            int v;
            printf("############### READER|cid:%d, ready to read chan\n", CoApi::getcid());
            chan >> v;
            g_acc_read += v;
        }
    } catch (CoException& ex) {
        if (ex.errorcode() == CO_ERROR_CHANNEL_CLOSE) {
            printf("############### READER|cid:%d, channel is closed\n", CoApi::getcid());
        } else {
            printf("############### READER|cid:%d, exception:%s\n", CoApi::getcid(), ex.what());
        }
    } catch (...) {
        printf("############### READER|cid:%d, unknow exception\n", CoApi::getcid());
    }
    g_end_count++;
}

void channel_writer(CoChannel<int>& chan)
{
    printf("############### WRITER|cid:%d, writter is running\n", CoApi::getcid());
    try {
        for (int i = 0; i < 10; i++) {
            chan << i;
            g_acc_write += i;
        }
        printf("############### WRITER|cid:%d, writer finish\n", CoApi::getcid());
    } catch (CoException& ex) {
        if (ex.errorcode() == CO_ERROR_CHANNEL_CLOSE) {
            printf("############### WRITER|cid:%d, channel is closed\n", CoApi::getcid());
        } else {
            printf("############### WRITER|cid:%d, exception:%s\n", CoApi::getcid(), ex.what());
        }
    } catch (...) {
        printf("############### WRITER|cid:%d, unknow exception\n", CoApi::getcid());
    }
    g_end_count++;
}
