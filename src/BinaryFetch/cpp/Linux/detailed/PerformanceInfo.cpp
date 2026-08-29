#include "PerformanceInfo.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <cstdio>
#include <memory>
#include <sys/statvfs.h>
#include <dirent.h>
#include <dlfcn.h>
#include <unistd.h>
#include <iostream>
using namespace std;

//  NVML minimal runtime binding 
// We declare just enough of the NVML ABI ourselves so this file compiles
// with zero NVIDIA SDK/headers present. At runtime we dlopen the driver's
// libnvidia-ml.so; if it's missing (no NVIDIA GPU/driver), everything here
// simply fails to load and we fall through to other GPU detection methods.

typedef int nvmlReturn_t;
typedef void* nvmlDevice_t;

struct nvmlUtilization_t {
    unsigned int gpu;    // percent of time over the past sample period during which one or more kernels was executing
    unsigned int memory; // percent of time over the past sample period during which global memory was being read/written
};

typedef nvmlReturn_t (*nvmlInit_v2_t)();
typedef nvmlReturn_t (*nvmlShutdown_t)();
typedef nvmlReturn_t (*nvmlDeviceGetCount_v2_t)(unsigned int*);
typedef nvmlReturn_t (*nvmlDeviceGetHandleByIndex_v2_t)(unsigned int, nvmlDevice_t*);
typedef nvmlReturn_t (*nvmlDeviceGetUtilizationRates_t)(nvmlDevice_t, nvmlUtilization_t*);

struct NvmlBinding {
    void* handle = nullptr;
    nvmlInit_v2_t init = nullptr;
    nvmlShutdown_t shutdown = nullptr;
    nvmlDeviceGetCount_v2_t getCount = nullptr;
    nvmlDeviceGetHandleByIndex_v2_t getHandle = nullptr;
    nvmlDeviceGetUtilizationRates_t getUtilization = nullptr;
    bool loaded = false;

    ~NvmlBinding() {
        if (handle) {
            if (shutdown) shutdown();
            dlclose(handle);
        }
    }
};

static NvmlBinding& nvml() {
    static NvmlBinding binding;
    static bool attempted = false;

    if (!attempted) {
        attempted = true;

        // Try common library names/paths across distros
        static const char* candidates[] = {
            "libnvidia-ml.so.1",
            "libnvidia-ml.so",
            "/usr/lib/x86_64-linux-gnu/libnvidia-ml.so.1",
            "/usr/lib64/libnvidia-ml.so.1"
        };

        for (const char* path : candidates) {
            binding.handle = dlopen(path, RTLD_LAZY);
            if (binding.handle) break;
        }

        if (!binding.handle) {
            cerr << "[PerformanceInfo] NVML not found (no NVIDIA driver installed) — "
                    "will try other GPU detection methods.\n";
            return binding;
        }

        binding.init = (nvmlInit_v2_t)dlsym(binding.handle, "nvmlInit_v2");
        binding.shutdown = (nvmlShutdown_t)dlsym(binding.handle, "nvmlShutdown");
        binding.getCount = (nvmlDeviceGetCount_v2_t)dlsym(binding.handle, "nvmlDeviceGetCount_v2");
        binding.getHandle = (nvmlDeviceGetHandleByIndex_v2_t)dlsym(binding.handle, "nvmlDeviceGetHandleByIndex_v2");
        binding.getUtilization = (nvmlDeviceGetUtilizationRates_t)dlsym(binding.handle, "nvmlDeviceGetUtilizationRates");

        if (!binding.init || !binding.getCount || !binding.getHandle || !binding.getUtilization) {
            cerr << "[PerformanceInfo] NVML loaded but missing expected symbols — driver version mismatch?\n";
            dlclose(binding.handle);
            binding.handle = nullptr;
            return binding;
        }

        if (binding.init() != 0) {
            cerr << "[PerformanceInfo] NVML init failed — driver present but not functional.\n";
            dlclose(binding.handle);
            binding.handle = nullptr;
            return binding;
        }

        binding.loaded = true;
    }

    return binding;
}

// Returns average utilization across all NVIDIA GPUs found, or -1.0f if NVML unavailable/no NVIDIA GPU
static float getGpuUsageViaNvml() {
    NvmlBinding& b = nvml();
    if (!b.loaded) return -1.0f;

    unsigned int deviceCount = 0;
    if (b.getCount(&deviceCount) != 0 || deviceCount == 0) {
        return -1.0f;
    }

    float totalUsage = 0.0f;
    unsigned int validSamples = 0;

    for (unsigned int i = 0; i < deviceCount; ++i) {
        nvmlDevice_t device;
        if (b.getHandle(i, &device) != 0) continue;

        nvmlUtilization_t util;
        if (b.getUtilization(device, &util) == 0) {
            totalUsage += static_cast<float>(util.gpu);
            validSamples++;
        }
    }

    if (validSamples == 0) return -1.0f;
    return totalUsage / static_cast<float>(validSamples);
}

//  Generic sysfs GPU busy-percent (works for AMD amdgpu, and some others) 
// Many non-NVIDIA drivers (notably amdgpu) expose a ready-made percentage file directly in sysfs.
// This walks every /sys/class/drm/card*/device/gpu_busy_percent and averages what it finds,
// so it naturally supports multiple/mixed GPUs without hardcoding vendor names or card indices.
static float getGpuUsageViaSysfs() {
    DIR* dir = opendir("/sys/class/drm");
    if (!dir) {
        cerr << "[PerformanceInfo] Could not open /sys/class/drm\n";
        return -1.0f;
    }

    float totalUsage = 0.0f;
    int validSamples = 0;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        string name = entry->d_name;
        // Only look at top-level "cardN" entries, not "cardN-HDMI-..." connector subentries
        if (name.rfind("card", 0) != 0) continue;
        if (name.find('-') != string::npos) continue;

        string path = "/sys/class/drm/" + name + "/device/gpu_busy_percent";
        ifstream file(path);
        if (!file.is_open()) continue; // not every driver exposes this (e.g. NVIDIA's own doesn't)

        int percent = -1;
        file >> percent;
        if (percent >= 0 && percent <= 100) {
            totalUsage += static_cast<float>(percent);
            validSamples++;
        }
    }
    closedir(dir);

    if (validSamples == 0) return -1.0f;
    return totalUsage / static_cast<float>(validSamples);
}

//  PIMPL 
struct PerformanceInfo::Impl {
    unsigned long long prevIdle = 0;
    unsigned long long prevTotal = 0;
    bool havePrevCpuSample = false;
};

PerformanceInfo::PerformanceInfo() : pImpl(new Impl()) {}
PerformanceInfo::~PerformanceInfo() { delete pImpl; }

//  uptime 
string PerformanceInfo::format_uptime(unsigned long long totalMilliseconds) {
    unsigned long long totalSeconds = totalMilliseconds / 1000;
    unsigned long long days = totalSeconds / (24 * 3600);
    unsigned long long hours = (totalSeconds % (24 * 3600)) / 3600;
    unsigned long long minutes = (totalSeconds % 3600) / 60;

    string result;
    if (days > 0) result += to_string(days) + (days == 1 ? " day, " : " days, ");
    if (hours > 0) result += to_string(hours) + (hours == 1 ? " hour, " : " hours, ");
    result += to_string(minutes) + (minutes == 1 ? " minute" : " minutes");
    return result;
}

string PerformanceInfo::get_system_uptime() {
    ifstream file("/proc/uptime");
    if (!file.is_open()) {
        cerr << "[PerformanceInfo] ERROR: could not open /proc/uptime\n";
        return "Unknown";
    }
    double uptimeSeconds = 0.0;
    file >> uptimeSeconds;
    return format_uptime(static_cast<unsigned long long>(uptimeSeconds * 1000.0));
}

//  CPU usage 
static bool readCpuTicks(unsigned long long& idleTime, unsigned long long& totalTime) {
    ifstream file("/proc/stat");
    if (!file.is_open()) return false;

    string line;
    getline(file, line); // aggregate "cpu  ..." line

    istringstream iss(line);
    string label;
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    iss >> label >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

    idleTime = idle + iowait;
    totalTime = user + nice + system + idle + iowait + irq + softirq + steal;
    return true;
}

float PerformanceInfo::get_cpu_usage_percent() {
    unsigned long long idleTime = 0, totalTime = 0;
    if (!readCpuTicks(idleTime, totalTime)) {
        cerr << "[PerformanceInfo] ERROR: could not read /proc/stat\n";
        return -1.0f;
    }

    if (!pImpl->havePrevCpuSample) {
        pImpl->prevIdle = idleTime;
        pImpl->prevTotal = totalTime;
        pImpl->havePrevCpuSample = true;

        usleep(100000); // 100ms so the very first call still returns a real number

        if (!readCpuTicks(idleTime, totalTime)) return -1.0f;
    }

    unsigned long long deltaIdle = idleTime - pImpl->prevIdle;
    unsigned long long deltaTotal = totalTime - pImpl->prevTotal;

    pImpl->prevIdle = idleTime;
    pImpl->prevTotal = totalTime;

    if (deltaTotal == 0) return 0.0f;

    float usage = (1.0f - (static_cast<float>(deltaIdle) / static_cast<float>(deltaTotal))) * 100.0f;
    if (usage < 0.0f) usage = 0.0f;
    if (usage > 100.0f) usage = 100.0f;
    return usage;
}

//  RAM usage 
float PerformanceInfo::get_ram_usage_percent() {
    ifstream file("/proc/meminfo");
    if (!file.is_open()) {
        cerr << "[PerformanceInfo] ERROR: could not open /proc/meminfo\n";
        return -1.0f;
    }

    unsigned long long memTotalKB = 0, memAvailableKB = 0;
    bool haveTotal = false, haveAvailable = false;

    string line;
    while (getline(file, line)) {
        if (line.rfind("MemTotal:", 0) == 0) {
            istringstream(line.substr(9)) >> memTotalKB;
            haveTotal = true;
        }
        else if (line.rfind("MemAvailable:", 0) == 0) {
            istringstream(line.substr(13)) >> memAvailableKB;
            haveAvailable = true;
        }
        if (haveTotal && haveAvailable) break;
    }

    if (!haveTotal || !haveAvailable || memTotalKB == 0) return -1.0f;

    float used = static_cast<float>(memTotalKB - memAvailableKB);
    float percent = (used / static_cast<float>(memTotalKB)) * 100.0f;
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    return percent;
}

//  Disk usage (root filesystem) 
float PerformanceInfo::get_disk_usage_percent() {
    struct statvfs stat;
    if (statvfs("/", &stat) != 0) {
        cerr << "[PerformanceInfo] ERROR: statvfs(\"/\") failed\n";
        return -1.0f;
    }

    unsigned long long total = static_cast<unsigned long long>(stat.f_blocks) * stat.f_frsize;
    unsigned long long free = static_cast<unsigned long long>(stat.f_bfree) * stat.f_frsize;

    if (total == 0) return -1.0f;

    float used = static_cast<float>(total - free);
    float percent = (used / static_cast<float>(total)) * 100.0f;
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    return percent;
}

//  GPU usage (NVML first, sysfs fallback for AMD/others) 
float PerformanceInfo::get_gpu_usage_percent() {
    float nvmlResult = getGpuUsageViaNvml();
    if (nvmlResult >= 0.0f) return nvmlResult;

    float sysfsResult = getGpuUsageViaSysfs();
    if (sysfsResult >= 0.0f) return sysfsResult;

    cerr << "[PerformanceInfo] No GPU usage source available "
            "(no NVIDIA driver via NVML, no amdgpu-style sysfs entry found).\n";
    return -1.0f;
}