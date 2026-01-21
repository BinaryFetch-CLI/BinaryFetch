#pragma once

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

class CompactPerformance {
public:
    int getCPUUsage();
    int getRAMUsage();
    int getDiskUsage();
    int getGPUUsage(); // NVIDIA -> NVAPI, otherwise Windows PDH
};
