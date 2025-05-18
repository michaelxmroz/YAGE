#include "DebuggerUtils.h"

#define DEBUG_VS 1


#if DEBUG_VS
#include <intrin.h>
#endif
#include "Logger.h"

void DebuggerUtils::TriggerBreakpoint(void* /*userData*/)
{
#if DEBUG_VS
	__debugbreak();
#endif
}

void DebuggerUtils::TriggerLog(void* /*userData*/)
{
	LOG_INFO("Log triggered");
}
