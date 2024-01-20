#pragma once

#include <memory>
#include <string>
#include <stdexcept>

#define CPU_FREQUENCY 4194304
#define CYCLES_PER_FRAME 17556

#define FORCE_INLINE inline

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

namespace Helpers
{
    FORCE_INLINE unsigned int GetFirstSetBit(int n)
    {
        return static_cast<uint32_t>(log2(n & -n));
    }
}
