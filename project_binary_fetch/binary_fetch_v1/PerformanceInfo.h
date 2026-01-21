#pragma once

#include <string>

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#include <Pdh.h>
#pragma comment(lib, "pdh.lib")
#pragma comment(lib, "nvapi64.lib")
#endif

class PerformanceInfo {
private:
    struct Impl;
    Impl* pImpl;

    std::string format_uptime(unsigned long long totalMilliseconds);

public:
    PerformanceInfo();
    ~PerformanceInfo();

    std::string get_system_uptime();
    float get_cpu_usage_percent();
    float get_ram_usage_percent();
    float get_disk_usage_percent();
    float get_gpu_usage_percent();
};
