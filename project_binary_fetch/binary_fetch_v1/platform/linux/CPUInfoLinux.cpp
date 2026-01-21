#include "../Platform.h"
#include "../../CPUInfo.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <set>
#include <dirent.h>
#include <cstring>
#include <thread>
#include <chrono>

static long prev_idle = 0;
static long prev_total = 0;
static bool first_call = true;

std::string CPUInfo::get_cpu_info() {
    std::string content = Platform::readFile("/proc/cpuinfo");
    std::string model = Platform::parseValue(content, "model name");
    return model.empty() ? "Unknown CPU" : model;
}

float CPUInfo::get_cpu_utilization() {
    std::string stat = Platform::readFileLine("/proc/stat");
    if (stat.empty()) return 0.0f;
    
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
        return get_cpu_utilization();
    }
    
    long idle_delta = idle_time - prev_idle;
    long total_delta = total_time - prev_total;
    
    prev_idle = idle_time;
    prev_total = total_time;
    
    if (total_delta == 0) return 0.0f;
    
    return (1.0f - (float)idle_delta / (float)total_delta) * 100.0f;
}

std::string CPUInfo::get_cpu_base_speed() {
    std::string freq = Platform::readFileLine("/sys/devices/system/cpu/cpu0/cpufreq/base_frequency");
    if (freq.empty()) {
        freq = Platform::readFileLine("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
    }
    
    if (!freq.empty()) {
        float ghz = std::stof(freq) / 1000000.0f;
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2) << ghz << " GHz";
        return ss.str();
    }
    
    std::string content = Platform::readFile("/proc/cpuinfo");
    std::string mhz = Platform::parseValue(content, "cpu MHz");
    if (!mhz.empty()) {
        float ghz = std::stof(mhz) / 1000.0f;
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2) << ghz << " GHz";
        return ss.str();
    }
    
    return "N/A";
}

std::string CPUInfo::get_cpu_speed() {
    std::string freq = Platform::readFileLine("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");
    
    if (!freq.empty()) {
        float ghz = std::stof(freq) / 1000000.0f;
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2) << ghz << " GHz";
        return ss.str();
    }
    
    return get_cpu_base_speed();
}

int CPUInfo::get_cpu_sockets() {
    std::string content = Platform::readFile("/proc/cpuinfo");
    std::set<std::string> physical_ids;
    
    std::istringstream iss(content);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.find("physical id") == 0) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                physical_ids.insert(Platform::trim(line.substr(pos + 1)));
            }
        }
    }
    
    return physical_ids.empty() ? 1 : static_cast<int>(physical_ids.size());
}

int CPUInfo::get_cpu_cores() {
    std::string content = Platform::readFile("/proc/cpuinfo");
    std::string cores = Platform::parseValue(content, "cpu cores");
    
    if (!cores.empty()) {
        return std::stoi(cores) * get_cpu_sockets();
    }
    
    std::string siblings = Platform::parseValue(content, "siblings");
    if (!siblings.empty()) {
        return std::stoi(siblings) / 2;
    }
    
    return get_cpu_logical_processors() / 2;
}

int CPUInfo::get_cpu_logical_processors() {
    int count = 0;
    std::string content = Platform::readFile("/proc/cpuinfo");
    std::istringstream iss(content);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.find("processor") == 0) count++;
    }
    return count > 0 ? count : 1;
}

std::string CPUInfo::get_cpu_virtualization() {
    std::string content = Platform::readFile("/proc/cpuinfo");
    std::string flags = Platform::parseValue(content, "flags");
    
    if (flags.find("vmx") != std::string::npos) return "VT-x Enabled";
    if (flags.find("svm") != std::string::npos) return "AMD-V Enabled";
    
    return "Disabled";
}

static std::string readCacheSize(int level) {
    for (int i = 0; i < 4; i++) {
        std::string path = "/sys/devices/system/cpu/cpu0/cache/index" + std::to_string(i) + "/";
        std::string levelStr = Platform::readFileLine(path + "level");
        if (levelStr.empty()) continue;
        
        if (std::stoi(levelStr) == level) {
            std::string size = Platform::readFileLine(path + "size");
            if (!size.empty()) return Platform::trim(size);
        }
    }
    return "N/A";
}

std::string CPUInfo::get_cpu_l1_cache() {
    int total = 0;
    for (int i = 0; i < 2; i++) {
        std::string path = "/sys/devices/system/cpu/cpu0/cache/index" + std::to_string(i) + "/";
        std::string levelStr = Platform::readFileLine(path + "level");
        if (levelStr.empty() || std::stoi(levelStr) != 1) continue;
        
        std::string size = Platform::readFileLine(path + "size");
        if (!size.empty()) {
            int val = std::stoi(size);
            if (size.find('K') != std::string::npos) total += val;
            else if (size.find('M') != std::string::npos) total += val * 1024;
        }
    }
    
    if (total > 0) {
        std::ostringstream ss;
        ss << total << " KB";
        return ss.str();
    }
    return "N/A";
}

std::string CPUInfo::get_cpu_l2_cache() { return readCacheSize(2); }
std::string CPUInfo::get_cpu_l3_cache() { return readCacheSize(3); }

std::string CPUInfo::get_system_uptime() {
    std::string uptimeStr = Platform::readFileLine("/proc/uptime");
    if (uptimeStr.empty()) return "Unknown";
    
    double uptime_seconds = std::stod(uptimeStr);
    
    int days = static_cast<int>(uptime_seconds / 86400);
    int hours = static_cast<int>((uptime_seconds - days * 86400) / 3600);
    int minutes = static_cast<int>((uptime_seconds - days * 86400 - hours * 3600) / 60);
    int seconds = static_cast<int>(uptime_seconds) % 60;
    
    std::ostringstream ss;
    ss << days << ":" 
       << std::setw(2) << std::setfill('0') << hours << ":"
       << std::setw(2) << std::setfill('0') << minutes << ":"
       << std::setw(2) << std::setfill('0') << seconds;
    return ss.str();
}

int CPUInfo::get_process_count() {
    DIR* dir = opendir("/proc");
    if (!dir) return 0;
    
    int count = 0;
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_DIR) {
            bool is_pid = true;
            for (const char* p = entry->d_name; *p; p++) {
                if (*p < '0' || *p > '9') { is_pid = false; break; }
            }
            if (is_pid) count++;
        }
    }
    closedir(dir);
    return count;
}

int CPUInfo::get_thread_count() {
    std::string result = Platform::exec("ps -eo nlwp --no-headers 2>/dev/null | awk '{sum+=$1} END {print sum}'");
    if (!result.empty()) {
        try { return std::stoi(Platform::trim(result)); }
        catch (...) {}
    }
    return 0;
}

int CPUInfo::get_handle_count() {
    std::string result = Platform::exec("cat /proc/sys/fs/file-nr 2>/dev/null | awk '{print $1}'");
    if (!result.empty()) {
        try { return std::stoi(Platform::trim(result)); }
        catch (...) {}
    }
    return 0;
}
