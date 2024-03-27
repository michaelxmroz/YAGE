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
if(Logger_Helpers::loggerCallback != nullptr) \
 Logger_Helpers::loggerCallback(string_format("%s: %s %x %x\n","OP",__VA_ARGS__).c_str(), 0); \
}

#else
#define LOG_INSTRUCTION(message, ... ) \
{ \
 (void) (message); \
}
#endif

#if _LOGGING

#define LOG_INFO(message) \
{ \
if(Logger_Helpers::loggerCallback != nullptr) \
 Logger_Helpers::loggerCallback(message, 0); \
}

#define LOG_WARNING(message) \
{ \
if(Logger_Helpers::loggerCallback != nullptr) \
Logger_Helpers::loggerCallback(message, 1); \
}

#define LOG_ERROR(message) \
{ \
if(Logger_Helpers::loggerCallback != nullptr) \
Logger_Helpers::loggerCallback(message, 2); \
}

#else
#define LOG_INFO(message) \
{ \
 (void) (message); \
}

#define LOG_WARNING(message) \
{ \
 (void) (message); \
}

#define LOG_ERROR(message) \
{ \
 (void) (message); \
}
#endif

#define LOG_CPU_STATE(message) \
{ \
if(Logger_Helpers::loggerCallback != nullptr) \
Logger_Helpers::loggerCallback(message, 3); \
}
