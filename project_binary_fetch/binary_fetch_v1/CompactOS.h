#pragma once
#include <iostream>
#include <string>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <VersionHelpers.h>
#endif

#if defined(__linux__)
#include <sys/utsname.h>
#endif

#if defined(__FreeBSD__)
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#endif

class CompactOS {
public:
    std::string getOSName();
    std::string getOSBuild();
    std::string getArchitecture();
    std::string getUptime();
};