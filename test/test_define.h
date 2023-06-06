#ifndef __TEST_DEFINE_H__
#define __TEST_DEFINE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>

inline void test_logger(int level, const char* msg)
{
    static std::map<int, std::string> s_level = {
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

#endif