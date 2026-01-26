#include "CompactPerformance.h"
#include <pdh.h>
#include <pdhmsg.h>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include "nvapi.h"

#pragma comment(lib, "pdh.lib")
#pragma comment(lib, "nvapi64.lib")

// -------------------- Helpers --------------------
static bool isNvapiAvailable() {
    HMODULE hNvapi = LoadLibraryA("nvapi64.dll");
    if (!hNvapi) return false;
    FreeLibrary(hNvapi);
    return true;
}

// NVAPI Utilization Enum (for older headers)
#ifndef NVAPI_GPU_UTILIZATION_GPU
enum NV_GPU_UTILIZATION {
    NVAPI_GPU_UTILIZATION_GPU = 0,
    NVAPI_GPU_UTILIZATION_MEMORY = 1,
    NVAPI_GPU_UTILIZATION_DECODER = 2,
    NVAPI_GPU_UTILIZATION_ENCODER = 3,
    NVAPI_GPU_UTILIZATION_COUNT = 4
};
#endif

// -------------------- CPU Usage --------------------
int CompactPerformance::getCPUUsage() {
    PDH_HQUERY query;
    PDH_HCOUNTER counter;
    PDH_FMT_COUNTERVALUE counterVal;

    if (PdhOpenQuery(nullptr, 0, &query) != ERROR_SUCCESS) return -1;
    if (PdhAddCounter(query, L"\\Processor(_Total)\\% Processor Time", 0, &counter) != ERROR_SUCCESS) {
        PdhCloseQuery(query);
        return -1;
    }

    // Sample twice for accurate reading
    PdhCollectQueryData(query);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    PdhCollectQueryData(query);

    if (PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, nullptr, &counterVal) != ERROR_SUCCESS) {
        PdhCloseQuery(query);
        return -1;
    }

    double usage = counterVal.doubleValue;
    PdhCloseQuery(query);
    return static_cast<int>(usage);
}

// -------------------- RAM Usage --------------------
int CompactPerformance::getRAMUsage() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (!GlobalMemoryStatusEx(&memInfo)) return -1;
    return static_cast<int>(100.0 * (memInfo.ullTotalPhys - memInfo.ullAvailPhys) / memInfo.ullTotalPhys);
}

// -------------------- Disk Usage --------------------
int CompactPerformance::getDiskUsage() {
    ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;
    if (!GetDiskFreeSpaceEx(L"C:", &freeBytesAvailable, &totalBytes, &totalFreeBytes)) return -1;
    return static_cast<int>(100.0 * (totalBytes.QuadPart - totalFreeBytes.QuadPart) / totalBytes.QuadPart);
}

// -------------------- GPU Usage --------------------
int CompactPerformance::getGPUUsage() {
    // --- NVIDIA GPU via NVAPI ---
    if (isNvapiAvailable() && NvAPI_Initialize() == NVAPI_OK) {
        NvPhysicalGpuHandle nvGPU[64];
        NvU32 count = 0;
        if (NvAPI_EnumPhysicalGPUs(nvGPU, &count) == NVAPI_OK && count > 0) {
            NV_GPU_DYNAMIC_PSTATES_INFO_EX dynStates = {};
            dynStates.version = NV_GPU_DYNAMIC_PSTATES_INFO_EX_VER; // correct version
            if (NvAPI_GPU_GetDynamicPstatesInfoEx(nvGPU[0], &dynStates) == NVAPI_OK) {
                NvAPI_Unload();
                return static_cast<int>(dynStates.utilization[NVAPI_GPU_UTILIZATION_GPU].percentage);
            }
        }
        NvAPI_Unload();
    }

    // --- Non-NVIDIA: PDH GPU counter (3D engines only) ---
    PDH_HQUERY query;
    PDH_HCOUNTER counter;
    PDH_FMT_COUNTERVALUE value;

    if (PdhOpenQuery(NULL, 0, &query) != ERROR_SUCCESS) return -1;

    std::vector<std::wstring> engines = { L"\\GPU Engine(*_3D)\\Utilization Percentage" };
    int total = 0, count = 0;

    for (auto& eng : engines) {
        if (PdhAddEnglishCounter(query, eng.c_str(), 0, &counter) != ERROR_SUCCESS) continue;

        PdhCollectQueryData(query);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        PdhCollectQueryData(query);

        if (PdhGetFormattedCounterValue(counter, PDH_FMT_LONG, NULL, &value) == ERROR_SUCCESS) {
            total += value.longValue;
            count++;
        }
        PdhRemoveCounter(counter);
    }

    PdhCloseQuery(query);
    return (count > 0) ? total / count : 0;
}
