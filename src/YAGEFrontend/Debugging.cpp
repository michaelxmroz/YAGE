#include "Debugging.h"

#define DEBUG_VS 1


#if DEBUG_VS
#include <intrin.h>
#endif

void Debugging::TriggerBreakpoint(void* userData)
{
#if DEBUG_VS
	__debugbreak();
#endif
}
