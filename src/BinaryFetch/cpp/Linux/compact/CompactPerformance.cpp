#include "CompactPerformance.h"
#include <sys/statvfs.h>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <string>
#include <dirent.h>

// Helper to read CPU times from /proc/stat
static bool getCPUTimes(long long& idle, long long& total) {
    std::ifstream file("/proc/stat");
    if (!file.is_open()) return false;

    std::string cpu;
    long long user, nice, system, idleTime, iowait, irq, softirq, steal;
    if (file >> cpu >> user >> nice >> system >> idleTime >> iowait >> irq >> softirq >> steal) {
        idle = idleTime + iowait;
        total = user + nice + system + idleTime + iowait + irq + softirq + steal;
        return true;
    }
    return false;
}

// Get CPU usage percentage
int CompactPerformance::getCPUUsage() {
    long long idle1 = 0, total1 = 0;
    long long idle2 = 0, total2 = 0;

    if (!getCPUTimes(idle1, total1)) return -1;
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    if (!getCPUTimes(idle2, total2)) return -1;

    long long totalDiff = total2 - total1;
    long long idleDiff = idle2 - idle1;

    if (totalDiff == 0) return 0;
    return static_cast<int>(100.0 * (totalDiff - idleDiff) / totalDiff);
}

// Helper to parse values in kB from /proc/meminfo
static double getMemoryKB(const std::string& key) {
    std::ifstream file("/proc/meminfo");
    if (!file.is_open()) return 0.0;

    std::string line;
    while (std::getline(file, line)) {
        if (line.find(key + ":") == 0) {
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                double val = 0.0;
                std::stringstream ss(line.substr(colon + 1));
                if (ss >> val) return val;
            }
        }
    }
    return 0.0;
}

// Get RAM usage percentage
int CompactPerformance::getRAMUsage() {
    double total = getMemoryKB("MemTotal");
    double available = getMemoryKB("MemAvailable");
    if (available == 0.0) {
        double free = getMemoryKB("MemFree");
        double buffers = getMemoryKB("Buffers");
        double cached = getMemoryKB("Cached");
        available = free + buffers + cached;
    }

    if (total <= 0.0) return -1;
    return static_cast<int>(100.0 * (total - available) / total);
}

// Get Disk usage percentage for the root directory '/'
int CompactPerformance::getDiskUsage() {
    struct statvfs stat;
    if (statvfs("/", &stat) == 0) {
        double total = static_cast<double>(stat.f_blocks) * stat.f_frsize;
        double free = static_cast<double>(stat.f_bfree) * stat.f_frsize;
        if (total <= 0.0) return 0;
        return static_cast<int>(100.0 * (total - free) / total);
    }
    return -1;
}

// Get GPU usage percentage from sysfs
int CompactPerformance::getGPUUsage() {
    // Check AMD GPU usage
    std::ifstream amdFile("/sys/class/drm/card0/device/gpu_busy_percent");
    if (amdFile.is_open()) {
        int usage = 0;
        if (amdFile >> usage) return usage;
    }

    // Check Intel GPU usage (sometimes exposed under /sys/class/drm/card0/device/intel_gpu_top or similar)
    // There is no standard uniform Intel sysfs usage percent without debugfs.
    
    return -1; // Fallback for unsupported/unavailable info
}
