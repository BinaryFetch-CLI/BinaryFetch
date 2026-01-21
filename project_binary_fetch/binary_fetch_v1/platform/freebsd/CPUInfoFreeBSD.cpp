#include "../Platform.h"
#include "../../CPUInfo.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <sys/sysctl.h>
#include <sys/resource.h>
#include <thread>
#include <chrono>
#include <cstring>

static long prev_cp_time[5] = {0};
static bool first_call = true;

std::string CPUInfo::get_cpu_info() {
    std::string model = Platform::sysctlString("hw.model");
    return model.empty() ? "Unknown CPU" : model;
}

float CPUInfo::get_cpu_utilization() {
    long cp_time[5];
    size_t len = sizeof(cp_time);
    
    if (sysctlbyname("kern.cp_time", cp_time, &len, nullptr, 0) != 0) {
        return 0.0f;
    }
    
    if (first_call) {
        memcpy(prev_cp_time, cp_time, sizeof(cp_time));
        first_call = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return get_cpu_utilization();
    }
    
    long total_delta = 0;
    long idle_delta = 0;
    
    for (int i = 0; i < 5; i++) {
        total_delta += cp_time[i] - prev_cp_time[i];
    }
    idle_delta = cp_time[4] - prev_cp_time[4];
    
    memcpy(prev_cp_time, cp_time, sizeof(cp_time));
    
    if (total_delta == 0) return 0.0f;
    
    return (1.0f - (float)idle_delta / (float)total_delta) * 100.0f;
}

std::string CPUInfo::get_cpu_base_speed() {
    std::string freq = Platform::exec("sysctl -n dev.cpu.0.freq 2>/dev/null");
    freq = Platform::trim(freq);
    
    if (!freq.empty()) {
        try {
            float mhz = std::stof(freq);
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(2) << (mhz / 1000.0f) << " GHz";
            return ss.str();
        } catch (...) {}
    }
    
    std::string result = Platform::exec("sysctl -n hw.clockrate 2>/dev/null");
    result = Platform::trim(result);
    if (!result.empty()) {
        try {
            float mhz = std::stof(result);
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(2) << (mhz / 1000.0f) << " GHz";
            return ss.str();
        } catch (...) {}
    }
    
    return "N/A";
}

std::string CPUInfo::get_cpu_speed() {
    std::string freq = Platform::exec("sysctl -n dev.cpu.0.freq 2>/dev/null");
    freq = Platform::trim(freq);
    
    if (!freq.empty()) {
        try {
            float mhz = std::stof(freq);
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(2) << (mhz / 1000.0f) << " GHz";
            return ss.str();
        } catch (...) {}
    }
    
    return get_cpu_base_speed();
}

int CPUInfo::get_cpu_sockets() {
    long packages = Platform::sysctlLong("hw.packages");
    return packages > 0 ? static_cast<int>(packages) : 1;
}

int CPUInfo::get_cpu_cores() {
    int ncpu = static_cast<int>(Platform::sysctlLong("hw.ncpu"));
    
    std::string result = Platform::exec("sysctl -n kern.smp.cpus 2>/dev/null");
    if (!result.empty()) {
        try {
            int cores = std::stoi(Platform::trim(result));
            if (cores > 0) return cores;
        } catch (...) {}
    }
    
    return ncpu > 0 ? ncpu : 1;
}

int CPUInfo::get_cpu_logical_processors() {
    int ncpu = static_cast<int>(Platform::sysctlLong("hw.ncpu"));
    return ncpu > 0 ? ncpu : 1;
}

std::string CPUInfo::get_cpu_virtualization() {
    std::string features = Platform::exec("sysctl -n hw.vmm.vmx.cap.guest 2>/dev/null");
    if (!features.empty() && Platform::trim(features) != "0") {
        return "VT-x Enabled";
    }
    
    features = Platform::exec("sysctl -n hw.vmm.svm.features 2>/dev/null");
    if (!features.empty() && Platform::trim(features) != "0") {
        return "AMD-V Enabled";
    }
    
    return "Disabled";
}

std::string CPUInfo::get_cpu_l1_cache() {
    std::string result = Platform::exec("sysctl -n hw.cacheconfig 2>/dev/null | awk '{print $2}'");
    result = Platform::trim(result);
    if (!result.empty()) {
        try {
            int kb = std::stoi(result) / 1024;
            return std::to_string(kb) + " KB";
        } catch (...) {}
    }
    
    result = Platform::exec("dmesg | grep -i 'L1 cache' | head -1");
    if (!result.empty()) {
        return Platform::trim(result);
    }
    
    return "N/A";
}

std::string CPUInfo::get_cpu_l2_cache() {
    std::string result = Platform::exec("sysctl -n hw.cacheconfig 2>/dev/null | awk '{print $3}'");
    result = Platform::trim(result);
    if (!result.empty()) {
        try {
            int size = std::stoi(result);
            if (size >= 1024 * 1024) {
                return std::to_string(size / (1024 * 1024)) + " MB";
            }
            return std::to_string(size / 1024) + " KB";
        } catch (...) {}
    }
    return "N/A";
}

std::string CPUInfo::get_cpu_l3_cache() {
    std::string result = Platform::exec("sysctl -n hw.cacheconfig 2>/dev/null | awk '{print $4}'");
    result = Platform::trim(result);
    if (!result.empty()) {
        try {
            int size = std::stoi(result);
            if (size >= 1024 * 1024) {
                return std::to_string(size / (1024 * 1024)) + " MB";
            }
            return std::to_string(size / 1024) + " KB";
        } catch (...) {}
    }
    return "N/A";
}

std::string CPUInfo::get_system_uptime() {
    struct timeval boottime;
    size_t len = sizeof(boottime);
    int mib[2] = {CTL_KERN, KERN_BOOTTIME};
    
    if (sysctl(mib, 2, &boottime, &len, nullptr, 0) == 0) {
        time_t now = time(nullptr);
        time_t uptime_seconds = now - boottime.tv_sec;
        
        int days = static_cast<int>(uptime_seconds / 86400);
        int hours = static_cast<int>((uptime_seconds % 86400) / 3600);
        int minutes = static_cast<int>((uptime_seconds % 3600) / 60);
        int seconds = static_cast<int>(uptime_seconds % 60);
        
        std::ostringstream ss;
        ss << days << ":" 
           << std::setw(2) << std::setfill('0') << hours << ":"
           << std::setw(2) << std::setfill('0') << minutes << ":"
           << std::setw(2) << std::setfill('0') << seconds;
        return ss.str();
    }
    return "Unknown";
}

int CPUInfo::get_process_count() {
    std::string result = Platform::exec("ps ax 2>/dev/null | wc -l");
    result = Platform::trim(result);
    if (!result.empty()) {
        try {
            int count = std::stoi(result);
            return count > 0 ? count - 1 : 0;
        } catch (...) {}
    }
    return 0;
}

int CPUInfo::get_thread_count() {
    std::string result = Platform::exec("ps -axH 2>/dev/null | wc -l");
    result = Platform::trim(result);
    if (!result.empty()) {
        try {
            int count = std::stoi(result);
            return count > 0 ? count - 1 : 0;
        } catch (...) {}
    }
    return 0;
}

int CPUInfo::get_handle_count() {
    std::string result = Platform::exec("sysctl -n kern.openfiles 2>/dev/null");
    result = Platform::trim(result);
    if (!result.empty()) {
        try { return std::stoi(result); }
        catch (...) {}
    }
    return 0;
}
