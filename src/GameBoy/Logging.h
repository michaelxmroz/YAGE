#pragma once
#include <memory>
#include <string>
#include <stdexcept>

namespace Logger_Helpers
{
	void OutputToVisualStudioConsole(const char* message);
}

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


#define LOG_INFO(message) \
{ \
 Logger_Helpers::OutputToVisualStudioConsole(string_format("%s: %s\n","INFO",message).c_str()); \
}

#define LOG_WARNING(message) \
{ \
Logger_Helpers::OutputToVisualStudioConsole(string_format("%s: %s\n","WARNING",message).c_str()); \
}

#define LOG_ERROR(message) \
{ \
Logger_Helpers::OutputToVisualStudioConsole(string_format("%s: %s\n","ERROR",message).c_str()); \
}

void LogMessage(const char* message, uint8_t severity);
