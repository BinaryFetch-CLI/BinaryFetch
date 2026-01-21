#include "CompactPerformance.h"
#include "platform/Platform.h"
#include <sys/statvfs.h>
#include <thread>
#include <chrono>
#include <cstdio>
#include <cstring>

#if PLATFORM_LINUX

static long prev_idle = 0;
static long prev_total = 0;
static bool cpu_first_call = true;

int CompactPerformance::getCPUUsage() {
    std::string stat = Platform::readFileLine("/proc/stat");
    if (stat.empty()) return 0;
    
    long user, nice, system, idle, iowait, irq, softirq, steal;
    if (sscanf(stat.c_str(), "cpu %ld %ld %ld %ld %ld %ld %ld %ld",
               &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal) != 8) {
        return 0;
    }
    
    long idle_time = idle + iowait;
    long total_time = user + nice + system + idle + iowait + irq + softirq + steal;
    
    if (cpu_first_call) {
        prev_idle = idle_time;
        prev_total = total_time;
        cpu_first_call = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return getCPUUsage();
    }
    
    long idle_delta = idle_time - prev_idle;
    long total_delta = total_time - prev_total;
    
    prev_idle = idle_time;
    prev_total = total_time;
    
    if (total_delta == 0) return 0;
    return static_cast<int>((1.0 - static_cast<double>(idle_delta) / total_delta) * 100.0);
}

int CompactPerformance::getRAMUsage() {
    std::string content = Platform::readFile("/proc/meminfo");
    
    long long memTotal = 0, memAvailable = 0;
    std::string val = Platform::parseValue(content, "MemTotal");
    if (!val.empty()) memTotal = std::stoll(val);
    
    val = Platform::parseValue(content, "MemAvailable");
    if (!val.empty()) {
        memAvailable = std::stoll(val);
    } else {
        long long memFree = 0, buffers = 0, cached = 0;
        val = Platform::parseValue(content, "MemFree");
        if (!val.empty()) memFree = std::stoll(val);
        val = Platform::parseValue(content, "Buffers");
        if (!val.empty()) buffers = std::stoll(val);
        val = Platform::parseValue(content, "Cached");
        if (!val.empty()) cached = std::stoll(val);
        memAvailable = memFree + buffers + cached;
    }
    
    if (memTotal == 0) return 0;
    return static_cast<int>(((memTotal - memAvailable) * 100) / memTotal);
}

int CompactPerformance::getDiskUsage() {
    struct statvfs stat;
    if (statvfs("/", &stat) != 0) return 0;
    
    unsigned long long total = stat.f_blocks * stat.f_frsize;
    unsigned long long available = stat.f_bavail * stat.f_frsize;
    
    if (total == 0) return 0;
    return static_cast<int>(((total - available) * 100) / total);
}

int CompactPerformance::getGPUUsage() {
    if (Platform::commandExists("nvidia-smi")) {
        std::string result = Platform::exec("nvidia-smi --query-gpu=utilization.gpu --format=csv,noheader,nounits 2>/dev/null | head -1");
        result = Platform::trim(result);
        if (!result.empty()) {
            try { return std::stoi(result); }
            catch (...) {}
        }
    }
    
    std::string busy = Platform::readFileLine("/sys/class/drm/card0/device/gpu_busy_percent");
    if (!busy.empty()) {
        try { return std::stoi(Platform::trim(busy)); }
        catch (...) {}
    }
    
    return 0;
}

#elif PLATFORM_FREEBSD

static long prev_cp_time[5] = {0};
static bool cpu_first_call = true;

int CompactPerformance::getCPUUsage() {
    long cp_time[5];
    size_t len = sizeof(cp_time);
    
    if (sysctlbyname("kern.cp_time", cp_time, &len, nullptr, 0) != 0) {
        return 0;
    }
    
    if (cpu_first_call) {
        memcpy(prev_cp_time, cp_time, sizeof(cp_time));
        cpu_first_call = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return getCPUUsage();
    }
    
    long total_delta = 0;
    for (int i = 0; i < 5; i++) {
        total_delta += cp_time[i] - prev_cp_time[i];
    }
    long idle_delta = cp_time[4] - prev_cp_time[4];
    
    memcpy(prev_cp_time, cp_time, sizeof(cp_time));
    
    if (total_delta == 0) return 0;
    return static_cast<int>((1.0 - static_cast<double>(idle_delta) / total_delta) * 100.0);
}

int CompactPerformance::getRAMUsage() {
    unsigned long physmem = Platform::sysctlULong("hw.physmem");
    unsigned long pagesize = Platform::sysctlULong("hw.pagesize");
    if (pagesize == 0) pagesize = 4096;
    
    unsigned long free_count = Platform::sysctlULong("vm.stats.vm.v_free_count");
    unsigned long inactive = Platform::sysctlULong("vm.stats.vm.v_inactive_count");
    unsigned long cache = Platform::sysctlULong("vm.stats.vm.v_cache_count");
    
    unsigned long available = (free_count + inactive + cache) * pagesize;
    
    if (physmem == 0) return 0;
    return static_cast<int>(((physmem - available) * 100) / physmem);
}

int CompactPerformance::getDiskUsage() {
    struct statvfs stat;
    if (statvfs("/", &stat) != 0) return 0;
    
    unsigned long long total = stat.f_blocks * stat.f_frsize;
    unsigned long long available = stat.f_bavail * stat.f_frsize;
    
    if (total == 0) return 0;
    return static_cast<int>(((total - available) * 100) / total);
}

int CompactPerformance::getGPUUsage() {
    if (Platform::commandExists("nvidia-smi")) {
        std::string result = Platform::exec("nvidia-smi --query-gpu=utilization.gpu --format=csv,noheader,nounits 2>/dev/null | head -1");
        result = Platform::trim(result);
        if (!result.empty()) {
            try { return std::stoi(result); }
            catch (...) {}
        }
    }
    return 0;
}

#endif
