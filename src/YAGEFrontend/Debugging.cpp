#include "Debugging.h"

#define DEBUG_VS 1


#if DEBUG_VS
#include <intrin.h>
#endif
#include "Logger.h"

void Debugging::TriggerBreakpoint(void* userData)
{
#if DEBUG_VS
	__debugbreak();
#endif
}

void Debugging::TriggerLog(void* userData)
{
	LOG_INFO("Log triggered");
}
