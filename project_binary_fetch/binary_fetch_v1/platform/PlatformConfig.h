#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS 1
    #define PLATFORM_LINUX 0
    #define PLATFORM_FREEBSD 0
    #define PLATFORM_POSIX 0
#elif defined(__linux__)
    #define PLATFORM_WINDOWS 0
    #define PLATFORM_LINUX 1
    #define PLATFORM_FREEBSD 0
    #define PLATFORM_POSIX 1
#elif defined(__FreeBSD__)
    #define PLATFORM_WINDOWS 0
    #define PLATFORM_LINUX 0
    #define PLATFORM_FREEBSD 1
    #define PLATFORM_POSIX 1
#else
    #error "Unsupported platform"
#endif

#if PLATFORM_WINDOWS
    #define PLATFORM_NAME "Windows"
#elif PLATFORM_LINUX
    #define PLATFORM_NAME "Linux"
#elif PLATFORM_FREEBSD
    #define PLATFORM_NAME "FreeBSD"
#endif
