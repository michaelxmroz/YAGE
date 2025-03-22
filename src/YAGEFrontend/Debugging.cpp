#include "Debugging.h"
#include "PlatformDefines.h"

#define DEBUG_VS 1

// Only use Visual Studio debugging on Windows
#if YAGE_PLATFORM_WINDOWS && DEBUG_VS
#include <intrin.h>
#endif

void Debugging::TriggerBreakpoint(void* userData)
{
#if YAGE_PLATFORM_WINDOWS && DEBUG_VS
	__debugbreak();
#endif
}
