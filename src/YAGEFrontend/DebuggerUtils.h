#pragma once
#include <cstdint>

// Forward declaration
struct DebuggerState;

namespace DebuggerUtils
{
	void TriggerBreakpoint(void* userData);
	void TriggerLog(void* userData);
	void TriggerDebuggerBreakpoint(void* userData);
};

