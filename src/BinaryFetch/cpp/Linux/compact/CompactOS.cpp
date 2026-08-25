#include "CompactOS.h"
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <fstream>
#include <sstream>
#include <iomanip>

// Helper to extract a value from /etc/os-release
static std::string getOSReleaseValue(const std::string& key) {
    std::ifstream file("/etc/os-release");
    if (!file.is_open()) return "";

    std::string line;
    while (std::getline(file, line)) {
        if (line.find(key + "=") == 0) {
            std::string val = line.substr(key.length() + 1);
            // Strip quotes if present
            if (!val.empty() && (val.front() == '"' || val.front() == '\'')) {
                val = val.substr(1);
            }
            if (!val.empty() && (val.back() == '"' || val.back() == '\'')) {
                val.pop_back();
            }
            return val;
        }
    }
    return "";
}

// Get the OS name (e.g. "Ubuntu 22.04 LTS")
std::string CompactOS::getOSName() {
    std::string prettyName = getOSReleaseValue("PRETTY_NAME");
    if (!prettyName.empty()) {
        return prettyName;
    }
    std::string name = getOSReleaseValue("NAME");
    if (!name.empty()) {
        return name;
    }
    return "Linux";
}

// Get OS Build / Kernel Version
std::string CompactOS::getOSBuild() {
    struct utsname buffer;
    if (uname(&buffer) == 0) {
        return std::string(buffer.release);
    }
    return "Unknown Kernel";
}

// Get CPU/OS Architecture
std::string CompactOS::getArchitecture() {
    struct utsname buffer;
    if (uname(&buffer) == 0) {
        std::string arch(buffer.machine);
        if (arch == "x86_64") return "64-bit";
        if (arch == "i686" || arch == "i386") return "32-bit";
        return arch; // e.g. aarch64, armv7l
    }
    return "Unknown";
}

// Get System Uptime formatted as e.g. "1d 2h 3m" or "5h 12m"
std::string CompactOS::getUptime() {
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        long seconds = info.uptime;
        int days = seconds / 86400;
        int hours = (seconds % 86400) / 3600;
        int minutes = (seconds % 3600) / 60;

        std::ostringstream oss;
        if (days > 0) oss << days << "d ";
        if (hours > 0) oss << hours << "h ";
        oss << minutes << "m";
        return oss.str();
    }
    return "Unknown Uptime";
}
