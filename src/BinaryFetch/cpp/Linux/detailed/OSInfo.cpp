#include "OSInfo.h"
#include <sys/utsname.h>
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <string>
#include <array>
#include <memory>
#include <cstdio>
#include <unistd.h>
#include <ctime>
#include <iostream>
using namespace std;

// helpers
static string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n\"");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n\"");
    return s.substr(start, end - start + 1);
}

static bool runCommand(const string& cmd, string& output) {
    array<char, 256> buffer;
    output.clear();
    unique_ptr<FILE, decltype(&pclose)> pipe(popen((cmd + " 2>/dev/null").c_str(), "r"), pclose);
    if (!pipe) return false;
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        output += buffer.data();
    }
    return true;
}

static bool isRoot() {
    return geteuid() == 0;
}

// Reads a single KEY=value pair out of /etc/os-release (or /usr/lib/os-release fallback)
static string readOsReleaseField(const string& key) {
    static const char* paths[] = { "/etc/os-release", "/usr/lib/os-release" };

    for (const char* path : paths) {
        ifstream file(path);
        if (!file.is_open()) continue;

        string line;
        while (getline(file, line)) {
            if (line.rfind(key + "=", 0) == 0) {
                return trim(line.substr(key.size() + 1));
            }
        }
    }
    return "";
}

// kernel version string (Major.Minor.Patch, distro-agnostic)
string OSInfo::GetOSVersion() {
    struct utsname buf;
    if (uname(&buf) == 0) {
        return string("Linux ") + buf.release; // e.g. "Linux 6.8.0-45-generic"
    }
    return "Unknown Linux version";
}

// architecture
string OSInfo::GetOSArchitecture() {
    struct utsname buf;
    if (uname(&buf) == 0) {
        string machine = buf.machine; // e.g. "x86_64", "aarch64", "i686"
        if (machine == "x86_64" || machine == "aarch64") return "64-bit";
        if (machine.find("64") != string::npos) return "64-bit";
        if (!machine.empty()) return "32-bit";
    }
    return "Unknown";
}

// distro name/edition, analogous to Win32_OperatingSystem Caption
string OSInfo::GetOSName() {
    string prettyName = readOsReleaseField("PRETTY_NAME");
    if (!prettyName.empty()) return prettyName;

    string name = readOsReleaseField("NAME");
    string version = readOsReleaseField("VERSION");
    if (!name.empty()) {
        return version.empty() ? name : (name + " " + version);
    }

    // Last-resort fallback: kernel name/release via uname
    struct utsname buf;
    if (uname(&buf) == 0) {
        return string(buf.sysname) + " " + buf.release;
    }

    return "Unknown Distribution";
}

// "serial number" -> closest Linux analogue is machine-id / DMI product UUID
string OSInfo::get_os_serial_number() {
    // 1) /etc/machine-id: stable, unique per-install identifier, always readable
    {
        ifstream file("/etc/machine-id");
        if (file.is_open()) {
            string id;
            getline(file, id);
            id = trim(id);
            if (!id.empty()) return id;
        }
    }

    // 2) DMI product UUID (closer analogue to a hardware "serial", but usually root-only)
    {
        ifstream file("/sys/class/dmi/id/product_uuid");
        if (file.is_open()) {
            string id;
            getline(file, id);
            id = trim(id);
            if (!id.empty()) return id;
        }
        if (!isRoot()) {
            cerr << "[OSInfo] Note: /sys/class/dmi/id/product_uuid requires root; "
                    "falling back to other identifiers.\n";
        }
    }

    // 3) dmidecode fallback (also typically root-only)
    {
        string output;
        if (runCommand("dmidecode -s system-uuid", output)) {
            string id = trim(output);
            if (!id.empty() && id.find("not") == string::npos) return id;
        }
    }

    return "Unknown";
}

// uptime, from /proc/uptime (seconds, no root needed)
string OSInfo::get_os_uptime() {
    ifstream file("/proc/uptime");
    if (!file.is_open()) {
        cerr << "[OSInfo] ERROR: could not open /proc/uptime\n";
        return "Unknown";
    }

    double uptimeSeconds = 0.0;
    file >> uptimeSeconds;

    unsigned long long totalSeconds = static_cast<unsigned long long>(uptimeSeconds);
    unsigned long long days = totalSeconds / (24 * 3600);
    unsigned long long hours = (totalSeconds % (24 * 3600)) / 3600;
    unsigned long long minutes = (totalSeconds % 3600) / 60;

    string result;

    if (days > 0) {
        result += to_string(days) + (days == 1 ? " day, " : " days, ");
    }
    if (hours > 0) {
        result += to_string(hours) + (hours == 1 ? " hour, " : " hours, ");
    }
    result += to_string(minutes) + (minutes == 1 ? " minute" : " minutes");

    return result;
}

// install date: no direct equivalent on Linux; use root filesystem's creation time
// Common approach (used by neofetch/fastfetch too): the birth time of the root filesystem,
// approximated by the ctime of a directory that's created once at install and never touched again.
string OSInfo::get_os_install_date() {
    static const char* candidates[] = {
        "/lost+found", // ext-family filesystems create this once, at mkfs/install time
        "/etc",
        "/"
    };

    for (const char* path : candidates) {
        struct stat st;
        if (stat(path, &st) == 0) {
            // st_ctime = last inode metadata change; closest portable proxy for "creation"
            // available via plain stat() without filesystem-specific tools like tune2fs/xfs_db
            time_t t = st.st_ctime;
            char buf[11];
            struct tm tmResult;
            if (localtime_r(&t, &tmResult) != nullptr) {
                strftime(buf, sizeof(buf), "%Y-%m-%d", &tmResult);
                return string(buf);
            }
        }
    }

    return "Unknown";
}

// kernel info: full uname -a style string
string OSInfo::get_os_kernel_info() {
    struct utsname buf;
    if (uname(&buf) != 0) {
        return "Unknown";
    }

    // sysname release version machine, e.g.:
    // "Linux 6.8.0-45-generic #45-Ubuntu SMP PREEMPT_DYNAMIC ... x86_64"
    string result = string(buf.sysname) + " " + buf.release + " " + buf.version + " " + buf.machine;
    return result;
}