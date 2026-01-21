#include "../Platform.h"
#include "../../PerformanceInfo.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <sys/statvfs.h>

struct PerformanceInfo::Impl {
    long prev_idle = 0;
    long prev_total = 0;
    bool first_call = true;
};

PerformanceInfo::PerformanceInfo() {
    pImpl = new Impl();
}

PerformanceInfo::~PerformanceInfo() {
    delete pImpl;
    pImpl = nullptr;
}

std::string PerformanceInfo::format_uptime(unsigned long long totalMilliseconds) {
    unsigned long long totalSeconds = totalMilliseconds / 1000ULL;
    int hours = static_cast<int>(totalSeconds / 3600ULL);
    int minutes = static_cast<int>((totalSeconds % 3600ULL) / 60ULL);
    int seconds = static_cast<int>(totalSeconds % 60ULL);
    return std::to_string(hours) + "h " + std::to_string(minutes) + "m " + std::to_string(seconds) + "s";
}

std::string PerformanceInfo::get_system_uptime() {
    std::string uptimeStr = Platform::readFileLine("/proc/uptime");
    if (uptimeStr.empty()) return "Unknown";
    
    double uptime_seconds = std::stod(uptimeStr);
    unsigned long long ms = static_cast<unsigned long long>(uptime_seconds * 1000);
    return format_uptime(ms);
}

float PerformanceInfo::get_cpu_usage_percent() {
    std::string stat = Platform::readFileLine("/proc/stat");
    if (stat.empty()) return 0.0f;
    
    long user, nice, system, idle, iowait, irq, softirq, steal;
    sscanf(stat.c_str(), "cpu %ld %ld %ld %ld %ld %ld %ld %ld",
           &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);
    
    long idle_time = idle + iowait;
    long total_time = user + nice + system + idle + iowait + irq + softirq + steal;
    
    if (pImpl->first_call) {
        pImpl->prev_idle = idle_time;
        pImpl->prev_total = total_time;
        pImpl->first_call = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return get_cpu_usage_percent();
    }
    
    long idle_delta = idle_time - pImpl->prev_idle;
    long total_delta = total_time - pImpl->prev_total;
    
    pImpl->prev_idle = idle_time;
    pImpl->prev_total = total_time;
    
    if (total_delta == 0) return 0.0f;
    
    return (1.0f - (float)idle_delta / (float)total_delta) * 100.0f;
}

float PerformanceInfo::get_ram_usage_percent() {
    std::string content = Platform::readFile("/proc/meminfo");
    
    auto parseKB = [&](const std::string& key) -> long long {
        std::string val = Platform::parseValue(content, key);
        if (val.empty()) return 0;
        return std::stoll(val);
    };
    
    long long memTotal = parseKB("MemTotal");
    long long memAvailable = parseKB("MemAvailable");
    
    if (memAvailable == 0) {
        long long memFree = parseKB("MemFree");
        long long buffers = parseKB("Buffers");
        long long cached = parseKB("Cached");
        memAvailable = memFree + buffers + cached;
    }
    
    if (memTotal == 0) return 0.0f;
    
    long long used = memTotal - memAvailable;
    return static_cast<float>((used * 100.0) / memTotal);
}

float PerformanceInfo::get_disk_usage_percent() {
    struct statvfs stat;
    if (statvfs("/", &stat) != 0) return 0.0f;
    
    unsigned long long total = stat.f_blocks * stat.f_frsize;
    unsigned long long free = stat.f_bfree * stat.f_frsize;
    
    if (total == 0) return 0.0f;
    
    unsigned long long used = total - free;
    return static_cast<float>((used * 100.0) / total);
}

float PerformanceInfo::get_gpu_usage_percent() {
    if (Platform::commandExists("nvidia-smi")) {
        std::string result = Platform::exec("nvidia-smi --query-gpu=utilization.gpu --format=csv,noheader,nounits 2>/dev/null");
        if (!result.empty()) {
            try { return std::stof(Platform::trim(result)); }
            catch (...) {}
        }
    }
    
    std::string busyPath = "/sys/class/drm/card0/device/gpu_busy_percent";
    std::string busy = Platform::readFileLine(busyPath);
    if (!busy.empty()) {
        try { return std::stof(busy); }
        catch (...) {}
    }
    
    return 0.0f;
}
