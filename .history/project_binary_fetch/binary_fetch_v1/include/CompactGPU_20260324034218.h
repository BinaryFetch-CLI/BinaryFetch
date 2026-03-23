#pragma once
#include <string>
using namespace std;
class CompactGPU {
public:
    static string getGPUName();
    static double getVRAMGB();
    static int getGPUUsagePercent();  // Keep this
    static string getGPUFrequency();
    static double getGPUTemperature();
};
