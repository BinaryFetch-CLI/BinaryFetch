#include "CompactCPU.h"
#include "platform/Platform.h"
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>

#if PLATFORM_LINUX

static long prev_idle = 0;
static long prev_total = 0;
static bool first_call = true;

std::string CompactCPU::getCPUName() {
    std::string content = Platform::readFile("/proc/cpuinfo");
    std::string model = Platform::parseValue(content, "model name");
    return model.empty() ? "Unknown CPU" : model;
}

std::string CompactCPU::getCPUCores() {
    std::string content = Platform::readFile("/proc/cpuinfo");
    std::string cores = Platform::parseValue(content, "cpu cores");
    if (!cores.empty()) {
        return cores;
    }
    int count = 0;
    std::istringstream iss(content);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.find("processor") == 0) count++;
    }
    return std::to_string(count > 0 ? count / 2 : 1);
}

std::string CompactCPU::getCPUThreads() {
    std::string content = Platform::readFile("/proc/cpuinfo");
    int count = 0;
    std::istringstream iss(content);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.find("processor") == 0) count++;
    }
    return std::to_string(count > 0 ? count : 1);
}

double CompactCPU::getClockSpeed() {
    std::string freq = Platform::readFileLine("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");
    if (!freq.empty()) {
        return std::stof(freq) / 1000000.0;
    }
    
    std::string content = Platform::readFile("/proc/cpuinfo");
    std::string mhz = Platform::parseValue(content, "cpu MHz");
    if (!mhz.empty()) {
        return std::stof(mhz) / 1000.0;
    }
    return 0.0;
}

double CompactCPU::getUsagePercent() {
    std::string stat = Platform::readFileLine("/proc/stat");
    if (stat.empty()) return 0.0;
    
    long user, nice, system, idle, iowait, irq, softirq, steal;
    sscanf(stat.c_str(), "cpu %ld %ld %ld %ld %ld %ld %ld %ld",
           &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);
    
    long idle_time = idle + iowait;
    long total_time = user + nice + system + idle + iowait + irq + softirq + steal;
    
    if (first_call) {
        prev_idle = idle_time;
        prev_total = total_time;
        first_call = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return getUsagePercent();
    }
    
    long idle_delta = idle_time - prev_idle;
    long total_delta = total_time - prev_total;
    
    prev_idle = idle_time;
    prev_total = total_time;
    
    if (total_delta == 0) return 0.0;
    
    return (1.0 - (double)idle_delta / (double)total_delta) * 100.0;
}

#elif PLATFORM_FREEBSD

static long prev_cp_time[5] = {0};
static bool first_call = true;

std::string CompactCPU::getCPUName() {
    std::string model = Platform::sysctlString("hw.model");
    return model.empty() ? "Unknown CPU" : model;
}

std::string CompactCPU::getCPUCores() {
    int ncpu = static_cast<int>(Platform::sysctlLong("hw.ncpu"));
    return std::to_string(ncpu > 0 ? ncpu : 1);
}

std::string CompactCPU::getCPUThreads() {
    int ncpu = static_cast<int>(Platform::sysctlLong("hw.ncpu"));
    return std::to_string(ncpu > 0 ? ncpu : 1);
}

double CompactCPU::getClockSpeed() {
    std::string freq = Platform::exec("sysctl -n dev.cpu.0.freq 2>/dev/null");
    freq = Platform::trim(freq);
    if (!freq.empty()) {
        try {
            return std::stof(freq) / 1000.0;
        } catch (...) {}
    }
    return 0.0;
}

double CompactCPU::getUsagePercent() {
    long cp_time[5];
    size_t len = sizeof(cp_time);
    
    if (sysctlbyname("kern.cp_time", cp_time, &len, nullptr, 0) != 0) {
        return 0.0;
    }
    
    if (first_call) {
        memcpy(prev_cp_time, cp_time, sizeof(cp_time));
        first_call = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return getUsagePercent();
    }
    
    long total_delta = 0;
    long idle_delta = 0;
    
    for (int i = 0; i < 5; i++) {
        total_delta += cp_time[i] - prev_cp_time[i];
    }
    idle_delta = cp_time[4] - prev_cp_time[4];
    
    memcpy(prev_cp_time, cp_time, sizeof(cp_time));
    
    if (total_delta == 0) return 0.0;
    
    return (1.0 - (double)idle_delta / (double)total_delta) * 100.0;
}

#endif
