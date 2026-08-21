#pragma once
using namespace std;
class CompactPerformance {
public:
    int getCPUUsage();
    int getRAMUsage();
    int getDiskUsage();
    int getGPUUsage(); // NVIDIA -> NVAPI, otherwise Windows PDH
};
