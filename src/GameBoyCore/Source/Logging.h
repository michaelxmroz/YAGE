#pragma once
#include "Helpers.h"
#include "../Include/Emulator.h"

namespace Logger_Helpers
{
	extern Emulator::LoggerCallback loggerCallback;
}

//#define _LOG_INSTRUCTIONS 1

#ifdef _LOG_INSTRUCTIONS
#define LOG_INSTRUCTION(...) \
{ \
 Logger_Helpers::loggerCallback(string_format("%s: %s %x %x\n","OP",__VA_ARGS__).c_str(), 0); \
}

#else
#define LOG_INSTRUCTION(message, ... ) \
{ \
 (void) (message); \
}
#endif

#define LOG_INFO(message) \
{ \
 Logger_Helpers::loggerCallback(message, 0); \
}

#define LOG_WARNING(message) \
{ \
Logger_Helpers::loggerCallback(message, 1); \
}

#define LOG_ERROR(message) \
{ \
Logger_Helpers::loggerCallback(message, 2); \
}
