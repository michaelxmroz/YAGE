#pragma once
#include "Helpers.h"

namespace Logger_Helpers
{
	void OutputToVisualStudioConsole(const char* message);
}

//#define _LOG_INSTRUCTIONS 1

#ifdef _LOG_INSTRUCTIONS
#define LOG_INSTRUCTION(...) \
{ \
 Logger_Helpers::OutputToVisualStudioConsole(string_format("%s: %s %x %x\n","OP",__VA_ARGS__).c_str()); \
}

#else
#define LOG_INSTRUCTION(message, ... ) \
{ \
 (void) (message); \
}
#endif

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
