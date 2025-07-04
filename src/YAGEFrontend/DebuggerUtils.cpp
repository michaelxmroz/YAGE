#include "DebuggerUtils.h"
#include "EngineState.h"

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

void DebuggerUtils::TriggerDebuggerBreakpoint(void* userData)
{
	DebuggerState* debuggerState = static_cast<DebuggerState*>(userData);
	if (debuggerState)
	{
		// Stop the emulator by setting debugger steps to 0
		debuggerState->m_debuggerSteps = 0;
		debuggerState->m_forceGatherStats = true;
		LOG_INFO("Breakpoint hit - emulator stopped");
	}
}
