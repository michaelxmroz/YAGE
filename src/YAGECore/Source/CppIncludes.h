#pragma once

#ifdef _CPP
#define CPP_HOSTED 1
#endif

#ifdef CPP_HOSTED
#if _DEBUG
#include <map>
#include <stdexcept>
#include <memory>
#endif
#endif

#include <stdint.h>
#include <numeric>
#include <cstring>