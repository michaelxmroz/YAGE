#pragma once

#include "PlatformDefines.h"

#if YAGE_PLATFORM_WINDOWS
#include "BackendWin32.h"
typedef BackendWin32 Backend;
#elif YAGE_PLATFORM_UNIX
#include "BackendLinux.h"
typedef BackendLinux Backend;
#endif

