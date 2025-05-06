#pragma once
#include "../Include/Emulator.h"

namespace Logger_Helpers
{
	extern Emulator::LoggerCallback loggerCallback;
}

#if _DEBUG
#define CPU_STATE_LOGGING 0
constexpr bool LOG_CPU = CPU_STATE_LOGGING;

#define PPU_STATE_LOGGING 0
constexpr bool LOG_PPU = PPU_STATE_LOGGING;
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

#define LOG_CPU_STATE(message) \
{ \
if(Logger_Helpers::loggerCallback != nullptr && LOG_CPU) \
Logger_Helpers::loggerCallback(message, 3); \
}

#define LOG_PPU_STATE(message) \
{ \
if(Logger_Helpers::loggerCallback != nullptr && LOG_PPU) \
Logger_Helpers::loggerCallback(message, 3); \
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

#define LOG_CPU_STATE(message) \
{ \
 (void) (message); \
}

#define LOG_PPU_STATE(message) \
{ \
 (void) (message); \
}
#endif


