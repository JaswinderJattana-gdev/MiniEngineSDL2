#include "Log.h"

#include <chrono>
#include <cstdio>
#include <iostream>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#endif

static const char* LevelToString(LogLevel level)
{
    switch (level)
    {
    case LogLevel::Info:  return "INFO";
    case LogLevel::Warn:  return "WARN";
    case LogLevel::Error: return "ERROR";
    default:              return "UNK";
    }
}

static void GetTimestamp(char out[16])
{
    // HH:MM:SS
    using clock = std::chrono::system_clock;
    const auto now = clock::now();
    const std::time_t t = clock::to_time_t(now);

    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    std::snprintf(out, 16, "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
}

void Log::Info(std::string_view msg) { Write(LogLevel::Info, msg); }
void Log::Warn(std::string_view msg) { Write(LogLevel::Warn, msg); }
void Log::Error(std::string_view msg) { Write(LogLevel::Error, msg); }

void Log::Write(LogLevel level, std::string_view msg)
{
    char ts[16];
    GetTimestamp(ts);

    char line[1024];
    std::snprintf(line, sizeof(line), "[%s] %s: %.*s\n",
        ts,
        LevelToString(level),
        static_cast<int>(msg.size()),
        msg.data()
    );

    // Console
    std::cout << line;

    // File log (works even when double-clicking the exe)
    {
        std::ofstream f("engine.log", std::ios::app);
        if (f.is_open())
            f << line;
    }

#ifdef _WIN32
    // Visual Studio Output window (Debug)
    OutputDebugStringA(line);
#endif
}
