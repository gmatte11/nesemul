#pragma once

#ifdef _WIN32
#define IS_WINDOWS 1
#endif

#ifndef IS_WINDOWS
#defined IS_WINDOWS 0
#endif


#if IS_WINDOWS
#include <windows.h>
#undef min
#undef max
#endif

namespace threading
{
    inline void set_thread_high_priority()
    {
#if IS_WINDOWS
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
#endif
    }
}