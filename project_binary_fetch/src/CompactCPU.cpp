#include "CompactCPU.h"
#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <intrin.h>
#include <vector>
#include <thread>
#include <chrono>
#include <cstring>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "pdh.lib")

//---------------- Get CPU Name ------------------
std::string CompactCPU::getCPUName()
{
    int cpuInfo[4] = { -1 };
    char cpuBrand[0x40];
    __cpuid(cpuInfo, 0x80000000);
    unsigned int nExids = cpuInfo[0];
    memset(cpuBrand, 0, sizeof(cpuBrand));

    for (unsigned int i = 0x80000000; i <= nExids; ++i)
    {
        __cpuid(cpuInfo, i);
        if (i == 0x80000002) memcpy(cpuBrand, cpuInfo, sizeof(cpuInfo));
        else if (i == 0x80000003) memcpy(cpuBrand + 16, cpuInfo, sizeof(cpuInfo));
        else if (i == 0x80000004) memcpy(cpuBrand + 32, cpuInfo, sizeof(cpuInfo));
    }

    std::string name(cpuBrand);
    name.erase(name.find_last_not_of(' ') + 1);
    return name;
}

//---------------- Get CPU Core Count ------------------
std::string CompactCPU::getCPUCores()
{
    DWORD coreCount = 0;
    DWORD returnLength = 0;
    GetLogicalProcessorInformation(nullptr, &returnLength);
    std::vector<uint8_t> buffer(returnLength);
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION info =
        reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION>(buffer.data());

    if (GetLogicalProcessorInformation(info, &returnLength))
    {
        size_t count = returnLength / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
        for (size_t i = 0; i < count; ++i)
            if (info[i].Relationship == RelationProcessorCore) coreCount++;
    }
    return std::to_string(coreCount);
}

//---------------- Get CPU Thread Count ------------------
std::string CompactCPU::getCPUThreads()
{
    return std::to_string(GetActiveProcessorCount(ALL_PROCESSOR_GROUPS));
}

//---------------- Get CPU Clock Speed (GHz) ------------------
double CompactCPU::getClockSpeed()
{
    DWORD mhz = 0;
    DWORD bufSize = sizeof(DWORD);
    HKEY hKey;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        RegQueryValueExA(hKey, "~MHz", nullptr, nullptr, (LPBYTE)&mhz, &bufSize);
        RegCloseKey(hKey);
    }

    return static_cast<double>(mhz) / 1000.0;
}

//---------------- Get CPU Usage (%) Using PDH ------------------
double CompactCPU::getUsagePercent()
{
    PDH_HQUERY query;
    PDH_HCOUNTER counter;
    PDH_FMT_COUNTERVALUE counterVal;

    PdhOpenQuery(nullptr, 0, &query);
    PdhAddCounter(query, L"\\Processor(_Total)\\% Processor Time", 0, &counter);
    PdhCollectQueryData(query);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    PdhCollectQueryData(query);
    PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, nullptr, &counterVal);
    double usage = counterVal.doubleValue;
    PdhCloseQuery(query);
    return usage;
}
