#pragma once
#include "../Include/Emulator.h"

namespace Logger_Helpers
{
	extern Emulator::LoggerCallback loggerCallback;
}

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
