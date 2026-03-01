#pragma once
#include <string_view>

enum class LogLevel
{
    Info,
    Warn,
    Error
};

class Log
{
public:
    static void Info(std::string_view msg);
    static void Warn(std::string_view msg);
    static void Error(std::string_view msg);

private:
    static void Write(LogLevel level, std::string_view msg);
};
