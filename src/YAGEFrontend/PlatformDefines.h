#pragma once

// Platform detection
#if defined(_WIN32) || defined(WIN32)
    #define YAGE_WINDOWS 1
#elif defined(__linux__)
    #define YAGE_LINUX 1
#elif defined(__APPLE__)
    #define YAGE_MACOS 1
#else
    #error "Unsupported platform"
#endif

// Helper macro for platform-specific code
#define YAGE_PLATFORM_WINDOWS (YAGE_WINDOWS == 1)
#define YAGE_PLATFORM_LINUX (YAGE_LINUX == 1)
#define YAGE_PLATFORM_MACOS (YAGE_MACOS == 1)

// Combined Linux/macOS check (both Unix-like)
#define YAGE_PLATFORM_UNIX (YAGE_LINUX == 1 || YAGE_MACOS == 1) 