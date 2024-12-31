#pragma once

#include "CppIncludes.h"

#define CPU_FREQUENCY 4194304
#define CYCLES_PER_FRAME 17556
#define MCYCLES_TO_CYCLES 4

#define FORCE_INLINE inline

#ifdef _DEBUG

// Taken from https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
// License: https://creativecommons.org/publicdomain/zero/1.0/ 
template<typename ... Args>
std::string string_format(const std::string& format, Args ... args)
{
    int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
    if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
    auto size = static_cast<size_t>(size_s);
    auto buf = std::make_unique<char[]>(size);
    std::snprintf(buf.get(), size, format.c_str(), args ...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
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
    FORCE_INLINE unsigned int GetFirstSetBit(int n)
    {
        return static_cast<uint32_t>(log2(n & -n));
    }
}
