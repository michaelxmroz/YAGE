#pragma once

#define WIN32_BACKEND 1

#if WIN32_BACKEND
#include "BackendWin32.h"
#endif


typedef BackendWin32 Backend;

