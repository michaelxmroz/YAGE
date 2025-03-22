#pragma once

#include "PlatformDefines.h"

#if YAGE_PLATFORM_WINDOWS
#include "TimerWin32.h"
typedef TimerWin32 Timer;
#elif YAGE_PLATFORM_UNIX
#include "TimerLinux.h"
typedef TimerLinux Timer;
#endif 