#pragma once
#include <string>

class CompactGPU {
public:
    static std::string getGPUName();
    static double getVRAMGB();
    static int getGPUUsagePercent();  // Keep this
    static std::string getGPUFrequency();
    static double getGPUTemperature();
};
