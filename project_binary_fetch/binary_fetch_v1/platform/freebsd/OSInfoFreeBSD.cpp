#include "../Platform.h"
#include "../../OSInfo.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <sys/utsname.h>
#include <sys/sysctl.h>
#include <sys/stat.h>
#include <ctime>

std::string OSInfo::GetOSVersion() {
    std::string release = Platform::sysctlString("kern.osrelease");
    if (!release.empty()) {
        size_t dash = release.find('-');
        if (dash != std::string::npos) {
            return release.substr(0, dash);
        }
        return release;
    }
    return "Unknown";
}

std::string OSInfo::GetOSArchitecture() {
    std::string arch = Platform::sysctlString("hw.machine_arch");
    if (!arch.empty()) {
        if (arch == "amd64" || arch == "x86_64") return "64-bit";
        if (arch == "i386" || arch == "i686") return "32-bit";
        if (arch == "aarch64" || arch == "arm64") return "ARM64";
        if (arch == "armv7" || arch == "armv6") return "ARM32";
        return arch;
    }
    return "Unknown";
}

std::string OSInfo::GetOSName() {
    std::string ostype = Platform::sysctlString("kern.ostype");
    std::string version = GetOSVersion();
    
    if (!ostype.empty()) {
        if (!version.empty() && version != "Unknown") {
            return ostype + " " + version;
        }
        return ostype;
    }
    return "FreeBSD";
}

std::string OSInfo::get_os_install_date() {
    struct stat st;
    if (stat("/", &st) == 0) {
        char buf[64];
        struct tm* tm_info = localtime(&st.st_ctime);
        strftime(buf, sizeof(buf), "%Y-%m-%d", tm_info);
        return std::string(buf);
    }
    return "N/A";
}

std::string OSInfo::get_os_serial_number() {
    std::string result = Platform::exec("kenv smbios.system.serial 2>/dev/null");
    result = Platform::trim(result);
    if (!result.empty() && result != "To Be Filled By O.E.M." && result != "None") {
        return result;
    }
    
    result = Platform::exec("kenv smbios.planar.serial 2>/dev/null");
    result = Platform::trim(result);
    if (!result.empty() && result != "To Be Filled By O.E.M." && result != "None") {
        return result;
    }
    
    return "N/A (requires root)";
}

std::string OSInfo::get_os_uptime() {
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

std::string OSInfo::get_os_kernel_info() {
    struct utsname buf;
    if (uname(&buf) == 0) {
        return std::string(buf.sysname) + " " + buf.release;
    }
    return "FreeBSD";
}
