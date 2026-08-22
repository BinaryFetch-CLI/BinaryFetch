#include "CompactPerformance.h"
#include "CompactGPU.h"

#include <sys/statvfs.h>

#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <string>


// Read the CPU counters from /proc/stat.
static bool getCPUTimes(long long& idle, long long& total) {

    std::ifstream file("/proc/stat");

    if (!file.is_open())
        return false;

    std::string cpu;

    long long user;
    long long nice;
    long long system;
    long long idleTime;
    long long iowait;
    long long irq;
    long long softirq;
    long long steal;

    if (file >> cpu
             >> user
             >> nice
             >> system
             >> idleTime
             >> iowait
             >> irq
             >> softirq
             >> steal) {

        idle = idleTime + iowait;

        total = user
              + nice
              + system
              + idleTime
              + iowait
              + irq
              + softirq
              + steal;

        return true;
    }

    return false;
}


// Calculate CPU usage by comparing two samples.
int CompactPerformance::getCPUUsage() {

    long long idle1 = 0;
    long long total1 = 0;

    long long idle2 = 0;
    long long total2 = 0;

    if (!getCPUTimes(idle1, total1))
        return -1;

    // A short delay gives us two different CPU samples.
    std::this_thread::sleep_for(
        std::chrono::milliseconds(250)
    );

    if (!getCPUTimes(idle2, total2))
        return -1;

    long long totalDiff = total2 - total1;
    long long idleDiff = idle2 - idle1;

    if (totalDiff == 0)
        return 0;

    return static_cast<int>(
        100.0 *
        (totalDiff - idleDiff) /
        totalDiff
    );
}


// Read a value from /proc/meminfo.
// Values returned by meminfo are in kB.
static double getMemoryKB(const std::string& key) {

    std::ifstream file("/proc/meminfo");

    if (!file.is_open())
        return 0.0;

    std::string line;

    while (std::getline(file, line)) {

        if (line.find(key + ":") == 0) {

            size_t colon = line.find(':');

            if (colon != std::string::npos) {

                double value = 0.0;

                std::stringstream ss(
                    line.substr(colon + 1)
                );

                if (ss >> value)
                    return value;
            }
        }
    }

    return 0.0;
}


// Calculate RAM usage from MemTotal and MemAvailable.
int CompactPerformance::getRAMUsage() {

    double total = getMemoryKB("MemTotal");
    double available = getMemoryKB("MemAvailable");

    // Older systems may not expose MemAvailable.
    if (available == 0.0) {

        double free = getMemoryKB("MemFree");
        double buffers = getMemoryKB("Buffers");
        double cached = getMemoryKB("Cached");

        available = free + buffers + cached;
    }

    if (total <= 0.0)
        return -1;

    return static_cast<int>(
        100.0 *
        (total - available) /
        total
    );
}


// Get disk usage for the root filesystem.
int CompactPerformance::getDiskUsage() {

    struct statvfs stat{};

    if (statvfs("/", &stat) == 0) {

        double total =
            static_cast<double>(stat.f_blocks) *
            stat.f_frsize;

        double free =
            static_cast<double>(stat.f_bfree) *
            stat.f_frsize;

        if (total <= 0.0)
            return 0;

        return static_cast<int>(
            100.0 *
            (total - free) /
            total
        );
    }

    return -1;
}


// Use the GPU backend to get current GPU usage.
int CompactPerformance::getGPUUsage() {

    CompactGPU gpu;

    int usage = gpu.getGPUUsagePercent();

    if (usage >= 0)
        return usage;

    // Keep this as a fallback for drivers that expose
    // utilization through the DRM sysfs interface.
    std::ifstream amdFile(
        "/sys/class/drm/card0/device/gpu_busy_percent"
    );

    if (amdFile.is_open()) {

        int amdUsage = 0;

        if (amdFile >> amdUsage)
            return amdUsage;
    }

    // The GPU driver doesn't expose usage information.
    return -1;
}