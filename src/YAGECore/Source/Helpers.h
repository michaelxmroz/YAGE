#pragma once

#include "CppIncludes.h"
#include "YString.h"

#define CPU_FREQUENCY 4194304
#define CYCLES_PER_FRAME 17556
#define MCYCLES_TO_CYCLES 4

#define FORCE_INLINE inline

#ifdef _DEBUG

// Taken from https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
// License: https://creativecommons.org/publicdomain/zero/1.0/ 
template<typename ... Args>
yString string_format(const char* fmt, Args ... args)
{
    int size_s = std::snprintf(nullptr, 0, fmt, args ...) + 1; // Extra space for '\0'
    if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
    auto size = static_cast<size_t>(size_s);
    auto buf = std::make_unique<char[]>(size);
    std::snprintf(buf.get(), size, fmt, args ...);
    return yString(buf.get()); // We don't want the '\0' inside
}
#else

struct DummyString
{
    void c_str() {};
};

template<typename ... Args>
DummyString string_format(Args ... args)
{
    return DummyString();
}

#endif

namespace Helpers
{
    inline int32_t GetFirstSetBit(uint8_t n)
    {
        for(uint8_t i = 0 ; i < 8; ++i)
        {
	        if( (n & 0x1) > 0 )
	        {
                return i;
	        }
            n = n >> 1;
        }
        return -1;
    }
}
