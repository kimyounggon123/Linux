#ifndef LOGS_H
#define LOGS_H

#include <iostream>
#include <string>
namespace LogTool
{
    inline constexpr bool isOff = false;

    inline void Log(const char* where, const char* msg)
    {
        if (isOff) return;
        printf("[%s] %s\n", where, msg);
    }
    inline void Log(const std::string& where, const std::string& msg)
    {
        Log(where.c_str(), msg.c_str());
    }
};
#endif