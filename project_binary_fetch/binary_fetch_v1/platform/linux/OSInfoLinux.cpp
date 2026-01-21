#include "../Platform.h"
#include "../../OSInfo.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <ctime>

std::string OSInfo::GetOSVersion() {
    std::string content = Platform::readFile("/etc/os-release");
    std::string version = Platform::parseValue(content, "VERSION_ID", '=');
    if (version.empty()) {
        version = Platform::parseValue(content, "VERSION", '=');
    }
    version.erase(std::remove(version.begin(), version.end(), '"'), version.end());
    return version.empty() ? "Unknown" : version;
}

std::string OSInfo::GetOSArchitecture() {
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

std::string OSInfo::GetOSName() {
    std::string content = Platform::readFile("/etc/os-release");
    std::string name = Platform::parseValue(content, "PRETTY_NAME", '=');
    name.erase(std::remove(name.begin(), name.end(), '"'), name.end());
    
    if (name.empty()) {
        name = Platform::parseValue(content, "NAME", '=');
        name.erase(std::remove(name.begin(), name.end(), '"'), name.end());
        std::string version = GetOSVersion();
        if (!version.empty() && version != "Unknown") {
            name += " " + version;
        }
    }
    
    return name.empty() ? "Linux" : name;
}

std::string OSInfo::get_os_install_date() {
    struct stat st;
    if (stat("/", &st) == 0) {
        char buf[64];
        struct tm* tm_info = localtime(&st.st_ctime);
        strftime(buf, sizeof(buf), "%Y-%m-%d", tm_info);
        return std::string(buf);
    }
    
    std::string result = Platform::exec("ls -lact --full-time /etc 2>/dev/null | tail -1 | awk '{print $6}'");
    return Platform::trim(result).empty() ? "N/A" : Platform::trim(result);
}

std::string OSInfo::get_os_serial_number() {
    std::string serial = Platform::readFileLine("/sys/class/dmi/id/product_serial");
    if (!serial.empty() && serial != "To Be Filled By O.E.M.") {
        return Platform::trim(serial);
    }
    
    serial = Platform::readFileLine("/sys/class/dmi/id/board_serial");
    if (!serial.empty() && serial != "To Be Filled By O.E.M.") {
        return Platform::trim(serial);
    }
    
    return "N/A (requires root)";
}

std::string OSInfo::get_os_uptime() {
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

std::string OSInfo::get_os_kernel_info() {
    struct utsname buf;
    if (uname(&buf) == 0) {
        return std::string(buf.sysname) + " " + buf.release;
    }
    return "Linux";
}
