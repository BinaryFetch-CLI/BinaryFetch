#include "../Platform.h"
#include "../../PerformanceInfo.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <sys/sysctl.h>
#include <sys/mount.h>
#include <cstring>
#include <ctime>

struct PerformanceInfo::Impl {
    long prev_cp_time[5] = {0};
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
    struct timeval boottime;
    size_t len = sizeof(boottime);
    int mib[2] = {CTL_KERN, KERN_BOOTTIME};
    
    if (sysctl(mib, 2, &boottime, &len, nullptr, 0) == 0) {
        time_t now = time(nullptr);
        time_t uptime_seconds = now - boottime.tv_sec;
        unsigned long long ms = static_cast<unsigned long long>(uptime_seconds * 1000);
        return format_uptime(ms);
    }
    return "Unknown";
}

float PerformanceInfo::get_cpu_usage_percent() {
    long cp_time[5];
    size_t len = sizeof(cp_time);
    
    if (sysctlbyname("kern.cp_time", cp_time, &len, nullptr, 0) != 0) {
        return 0.0f;
    }
    
    if (pImpl->first_call) {
        memcpy(pImpl->prev_cp_time, cp_time, sizeof(cp_time));
        pImpl->first_call = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return get_cpu_usage_percent();
    }
    
    long total_delta = 0;
    long idle_delta = 0;
    
    for (int i = 0; i < 5; i++) {
        total_delta += cp_time[i] - pImpl->prev_cp_time[i];
    }
    idle_delta = cp_time[4] - pImpl->prev_cp_time[4];
    
    memcpy(pImpl->prev_cp_time, cp_time, sizeof(cp_time));
    
    if (total_delta == 0) return 0.0f;
    
    return (1.0f - (float)idle_delta / (float)total_delta) * 100.0f;
}

float PerformanceInfo::get_ram_usage_percent() {
    unsigned long physmem = Platform::sysctlULong("hw.physmem");
    unsigned long pagesize = Platform::sysctlULong("hw.pagesize");
    if (pagesize == 0) pagesize = 4096;
    
    unsigned long free_count = Platform::sysctlULong("vm.stats.vm.v_free_count");
    unsigned long inactive = Platform::sysctlULong("vm.stats.vm.v_inactive_count");
    unsigned long cache = Platform::sysctlULong("vm.stats.vm.v_cache_count");
    
    unsigned long available = (free_count + inactive + cache) * pagesize;
    
    if (physmem == 0) return 0.0f;
    
    unsigned long used = physmem - available;
    return static_cast<float>((used * 100.0) / physmem);
}

float PerformanceInfo::get_disk_usage_percent() {
    struct statfs* mntbuf;
    int mntsize = getmntinfo(&mntbuf, MNT_NOWAIT);
    
    if (mntsize <= 0) return 0.0f;
    
    for (int i = 0; i < mntsize; i++) {
        if (std::string(mntbuf[i].f_mntonname) == "/") {
            unsigned long long total = (unsigned long long)mntbuf[i].f_blocks * mntbuf[i].f_bsize;
            unsigned long long free = (unsigned long long)mntbuf[i].f_bfree * mntbuf[i].f_bsize;
            
            if (total == 0) return 0.0f;
            
            unsigned long long used = total - free;
            return static_cast<float>((used * 100.0) / total);
        }
    }
    
    return 0.0f;
}

float PerformanceInfo::get_gpu_usage_percent() {
    if (Platform::commandExists("nvidia-smi")) {
        std::string result = Platform::exec("nvidia-smi --query-gpu=utilization.gpu --format=csv,noheader,nounits 2>/dev/null");
        if (!result.empty()) {
            try { return std::stof(Platform::trim(result)); }
            catch (...) {}
        }
    }
    
    return 0.0f;
}
