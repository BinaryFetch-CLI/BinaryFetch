#include "CompactOS.h"
#include "platform/Platform.h"
#include <sstream>
#include <iomanip>

#if PLATFORM_LINUX

std::string CompactOS::getOSName() {
    std::string content = Platform::readFile("/etc/os-release");
    std::string name = Platform::parseValue(content, "PRETTY_NAME", '=');
    name.erase(std::remove(name.begin(), name.end(), '"'), name.end());
    
    if (name.empty()) {
        name = Platform::parseValue(content, "NAME", '=');
        name.erase(std::remove(name.begin(), name.end(), '"'), name.end());
    }
    
    return name.empty() ? "Linux" : name;
}

std::string CompactOS::getOSBuild() {
    struct utsname buf;
    if (uname(&buf) == 0) {
        return std::string(buf.release);
    }
    return "Unknown";
}

std::string CompactOS::getUptime() {
    std::string uptimeStr = Platform::readFileLine("/proc/uptime");
    if (uptimeStr.empty()) return "Unknown";
    
    double uptime_seconds = std::stod(uptimeStr);
    int days = static_cast<int>(uptime_seconds / 86400);
    int hours = static_cast<int>((uptime_seconds - days * 86400) / 3600);
    int minutes = static_cast<int>((uptime_seconds - days * 86400 - hours * 3600) / 60);
    
    std::ostringstream oss;
    if (days > 0) oss << days << "d ";
    if (hours > 0) oss << hours << "h ";
    oss << minutes << "m";
    return oss.str();
}

std::string CompactOS::getArchitecture() {
    struct utsname buf;
    if (uname(&buf) == 0) {
        std::string machine = buf.machine;
        if (machine == "x86_64" || machine == "amd64") return "64-bit";
        if (machine == "i386" || machine == "i686") return "32-bit";
        if (machine == "aarch64") return "ARM64";
        if (machine == "armv7l") return "ARM32";
        return machine;
    }
    return "Unknown";
}

#elif PLATFORM_FREEBSD

std::string CompactOS::getOSName() {
    std::string ostype = Platform::sysctlString("kern.ostype");
    std::string release = Platform::sysctlString("kern.osrelease");
    
    if (!ostype.empty()) {
        size_t dash = release.find('-');
        if (dash != std::string::npos) {
            release = release.substr(0, dash);
        }
        return ostype + " " + release;
    }
    return "FreeBSD";
}

std::string CompactOS::getOSBuild() {
    std::string release = Platform::sysctlString("kern.osrelease");
    return release.empty() ? "Unknown" : release;
}

std::string CompactOS::getUptime() {
    struct timeval boottime;
    size_t len = sizeof(boottime);
    int mib[2] = {CTL_KERN, KERN_BOOTTIME};
    
    if (sysctl(mib, 2, &boottime, &len, nullptr, 0) == 0) {
        time_t now = time(nullptr);
        time_t uptime_seconds = now - boottime.tv_sec;
        
        int days = static_cast<int>(uptime_seconds / 86400);
        int hours = static_cast<int>((uptime_seconds % 86400) / 3600);
        int minutes = static_cast<int>((uptime_seconds % 3600) / 60);
        
        std::ostringstream oss;
        if (days > 0) oss << days << "d ";
        if (hours > 0) oss << hours << "h ";
        oss << minutes << "m";
        return oss.str();
    }
    return "Unknown";
}

std::string CompactOS::getArchitecture() {
    std::string arch = Platform::sysctlString("hw.machine_arch");
    if (!arch.empty()) {
        if (arch == "amd64" || arch == "x86_64") return "64-bit";
        if (arch == "i386" || arch == "i686") return "32-bit";
        if (arch == "aarch64" || arch == "arm64") return "ARM64";
        return arch;
    }
    return "Unknown";
}

#endif
