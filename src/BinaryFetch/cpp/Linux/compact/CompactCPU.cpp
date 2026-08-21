#include "CompactCPU.h"
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <vector>

// Get the friendly CPU name/brand from /proc/cpuinfo
std::string CompactCPU::getCPUName() {
    std::ifstream file("/proc/cpuinfo");
    if (!file.is_open()) return "Unknown CPU";

    std::string line;
    while (std::getline(file, line)) {
        if (line.find("model name") == 0) {
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                std::string name = line.substr(colon + 1);
                // Strip leading spaces
                size_t first = name.find_first_not_of(" \t");
                if (first != std::string::npos) {
                    name = name.substr(first);
                }
                return name;
            }
        }
    }
    return "Unknown CPU";
}

// Get the physical CPU core count
std::string CompactCPU::getCPUCores() {
    std::ifstream file("/proc/cpuinfo");
    if (!file.is_open()) return "1";

    std::string line;
    int cores = 1;
    while (std::getline(file, line)) {
        if (line.find("cpu cores") == 0) {
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                std::stringstream ss(line.substr(colon + 1));
                ss >> cores;
                return std::to_string(cores);
            }
        }
    }
    return std::to_string(cores);
}

// Get logical thread count
std::string CompactCPU::getCPUThreads() {
    long threads = sysconf(_SC_NPROCESSORS_ONLN);
    if (threads > 0) {
        return std::to_string(threads);
    }
    return "1";
}

// Get CPU Clock Speed in GHz
double CompactCPU::getClockSpeed() {
    // Try reading the maximum frequency from sysfs
    std::ifstream freqFile("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
    if (!freqFile.is_open()) {
        freqFile.open("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq");
    }
    
    if (freqFile.is_open()) {
        double khz = 0;
        if (freqFile >> khz) {
            return khz / 1000000.0; // kHz to GHz
        }
    }

    // Fallback: parse /proc/cpuinfo for current frequency (cpu MHz)
    std::ifstream file("/proc/cpuinfo");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.find("cpu MHz") == 0) {
                size_t colon = line.find(':');
                if (colon != std::string::npos) {
                    double mhz = 0;
                    std::stringstream ss(line.substr(colon + 1));
                    if (ss >> mhz) {
                        return mhz / 1000.0; // MHz to GHz
                    }
                }
            }
        }
    }
    return 0.0;
}

// Helper to read total and idle CPU times from /proc/stat
static bool readCPUTimes(long long& idle, long long& total) {
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

// Compute total CPU utilization percentage
double CompactCPU::getUsagePercent() {
    long long idle1 = 0, total1 = 0;
    long long idle2 = 0, total2 = 0;

    if (!readCPUTimes(idle1, total1)) return 0.0;
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    if (!readCPUTimes(idle2, total2)) return 0.0;

    long long totalDiff = total2 - total1;
    long long idleDiff = idle2 - idle1;

    if (totalDiff == 0) return 0.0;
    return 100.0 * (totalDiff - idleDiff) / totalDiff;
}
