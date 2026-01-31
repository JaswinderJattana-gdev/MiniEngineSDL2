#pragma once

#include "Log.h"
#include <cstdlib>
#include <string>

#ifdef _WIN32
#include <intrin.h>
#endif

#if defined(_DEBUG) || !defined(NDEBUG)
static constexpr int ME_SOMETHING = 123;
#else
#define ME_DEBUG 0
#endif

#if ME_DEBUG
inline void ME_DebugBreak()
{
#ifdef _WIN32
    __debugbreak();
#else
    std::abort();
#endif
}

#define ME_ASSERT(cond, msg) \
        do { \
            if (!(cond)) { \
                Log::Error(std::string("ASSERT FAILED: ") + (msg)); \
                ME_DebugBreak(); \
            } \
        } while (0)
#else
#define ME_ASSERT(cond, msg) do { (void)sizeof(cond); } while (0)
#endif
