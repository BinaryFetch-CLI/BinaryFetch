#include "PerformanceInfo.h"
#include <pdhmsg.h>
#include <thread>
#include <chrono>
#include <vector>
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

// NVAPI Utilization Enum (in case header is old)
#ifndef NVAPI_GPU_UTILIZATION_GPU
enum NV_GPU_UTILIZATION {
    NVAPI_GPU_UTILIZATION_GPU = 0,
    NVAPI_GPU_UTILIZATION_MEMORY = 1,
    NVAPI_GPU_UTILIZATION_DECODER = 2,
    NVAPI_GPU_UTILIZATION_ENCODER = 3,
    NVAPI_GPU_UTILIZATION_COUNT = 4
};
#endif

// -------------------- Impl --------------------
struct PerformanceInfo::Impl {
    PDH_HQUERY cpuQuery;
    PDH_HCOUNTER cpuTotal;
    bool cpuInitialized;

    Impl() : cpuQuery(nullptr), cpuTotal(nullptr), cpuInitialized(false) {}
};

// -------------------- Constructor --------------------
PerformanceInfo::PerformanceInfo() {
    pImpl = new Impl();

    // CPU initialization
    PDH_STATUS status = PdhOpenQuery(NULL, 0, &pImpl->cpuQuery);
    if (status == ERROR_SUCCESS) {
        status = PdhAddCounter(pImpl->cpuQuery, TEXT("\\Processor(_Total)\\% Processor Time"), 0, &pImpl->cpuTotal);
        if (status == ERROR_SUCCESS) {
            PdhCollectQueryData(pImpl->cpuQuery); // prime
            pImpl->cpuInitialized = true;
        }
        else {
            PdhCloseQuery(pImpl->cpuQuery);
            pImpl->cpuQuery = nullptr;
            pImpl->cpuInitialized = false;
        }
    }
}

// -------------------- Destructor --------------------
PerformanceInfo::~PerformanceInfo() {
    if (pImpl) {
        if (pImpl->cpuInitialized && pImpl->cpuQuery) {
            PdhCloseQuery(pImpl->cpuQuery);
        }
        delete pImpl;
        pImpl = nullptr;
    }
}

// -------------------- Uptime --------------------
std::string PerformanceInfo::format_uptime(unsigned long long totalMilliseconds) {
    unsigned long long totalSeconds = totalMilliseconds / 1000ULL;
    int hours = static_cast<int>(totalSeconds / 3600ULL);
    int minutes = static_cast<int>((totalSeconds % 3600ULL) / 60ULL);
    int seconds = static_cast<int>(totalSeconds % 60ULL);
    return std::to_string(hours) + "h " + std::to_string(minutes) + "m " + std::to_string(seconds) + "s";
}

std::string PerformanceInfo::get_system_uptime() {
    return format_uptime(GetTickCount64());
}

// -------------------- CPU Usage --------------------
float PerformanceInfo::get_cpu_usage_percent() {
    if (!pImpl || !pImpl->cpuInitialized) return 0.0f;

    PDH_FMT_COUNTERVALUE counterVal;
    PdhCollectQueryData(pImpl->cpuQuery);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    PdhCollectQueryData(pImpl->cpuQuery);

    if (PdhGetFormattedCounterValue(pImpl->cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal) != ERROR_SUCCESS) {
        return 0.0f;
    }

    double val = counterVal.doubleValue;
    if (val < 0.0) val = 0.0;
    if (val > 100.0) val = 100.0;
    return static_cast<float>(val);
}

// -------------------- RAM Usage --------------------
float PerformanceInfo::get_ram_usage_percent() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(memInfo);
    if (!GlobalMemoryStatusEx(&memInfo)) return 0.0f;
    return static_cast<float>(memInfo.dwMemoryLoad);
}

// -------------------- Disk Usage --------------------
float PerformanceInfo::get_disk_usage_percent() {
    ULARGE_INTEGER freeBytesAvailable, totalBytes, freeBytes;
    if (!GetDiskFreeSpaceEx(L"C:\\", &freeBytesAvailable, &totalBytes, &freeBytes)) return 0.0f;
    if (totalBytes.QuadPart == 0) return 0.0f;
    double used = 1.0 - (static_cast<double>(freeBytes.QuadPart) / static_cast<double>(totalBytes.QuadPart));
    if (used < 0.0) used = 0.0;
    if (used > 1.0) used = 1.0;
    return static_cast<float>(used * 100.0);
}

// -------------------- GPU Usage --------------------
float PerformanceInfo::get_gpu_usage_percent() {
    // --- NVIDIA via NVAPI ---
    if (isNvapiAvailable() && NvAPI_Initialize() == NVAPI_OK) {
        NvPhysicalGpuHandle gpuHandles[64];
        NvU32 gpuCount = 0;

        if (NvAPI_EnumPhysicalGPUs(gpuHandles, &gpuCount) == NVAPI_OK && gpuCount > 0) {
            NV_GPU_DYNAMIC_PSTATES_INFO_EX dynStates = {};
            dynStates.version = NV_GPU_DYNAMIC_PSTATES_INFO_EX_VER;

            if (NvAPI_GPU_GetDynamicPstatesInfoEx(gpuHandles[0], &dynStates) == NVAPI_OK) {
                NvAPI_Unload();
                return static_cast<float>(dynStates.utilization[NVAPI_GPU_UTILIZATION_GPU].percentage);
            }
        }
        NvAPI_Unload();
    }

    // --- Non-NVIDIA: PDH GPU Engine (_3D) ---
    PDH_HQUERY query;
    PDH_HCOUNTER counter;
    PDH_FMT_COUNTERVALUE value;

    if (PdhOpenQuery(nullptr, 0, &query) != ERROR_SUCCESS) return 0.0f;

    std::vector<std::wstring> engines = { L"\\GPU Engine(*_3D)\\Utilization Percentage" };
    int total = 0, count = 0;

    for (auto& eng : engines) {
        if (PdhAddEnglishCounter(query, eng.c_str(), 0, &counter) != ERROR_SUCCESS) continue;

        PdhCollectQueryData(query);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        PdhCollectQueryData(query);

        if (PdhGetFormattedCounterValue(counter, PDH_FMT_LONG, nullptr, &value) == ERROR_SUCCESS) {
            total += value.longValue;
            count++;
        }

        PdhRemoveCounter(counter);
    }

    PdhCloseQuery(query);
    return (count > 0) ? static_cast<float>(total / count) : 0.0f;
}
