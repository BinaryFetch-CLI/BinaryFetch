#include "CPUInfo.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <thread>
#include <chrono>
#include <dirent.h>
#include <cctype>
#include <regex>

using namespace std;

// ---------------------------------------------------------------------
// small helpers
// ---------------------------------------------------------------------

// Slurp a whole file into a string. Returns "" if it can't be opened
// (some /sys files simply don't exist on every CPU/board, that's normal).
static string readFile(const string& path) {
    ifstream file(path);
    if (!file.is_open()) return "";
    stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

static string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// True if a directory name is all digits, i.e. a PID folder under /proc
static bool isAllDigits(const string& s) {
    if (s.empty()) return false;
    for (char c : s) if (!isdigit((unsigned char)c)) return false;
    return true;
}

// Format a size given in kB as a human string, e.g. "1024 KB" or "8 MB"
static string formatKB(long kb) {
    if (kb <= 0) return "Unknown";
    if (kb >= 1024) {
        double mb = kb / 1024.0;
        stringstream ss;
        ss.precision(mb == (long)mb ? 0 : 1);
        ss << fixed << mb << " MB";
        return ss.str();
    }
    return to_string(kb) + " KB";
}

// Turn a frequency in kHz into a "x.xx GHz" string
static string khzToGHzString(long khz) {
    if (khz <= 0) return "Unknown";
    double ghz = khz / 1000000.0;
    stringstream ss;
    ss.precision(2);
    ss << fixed << ghz << " GHz";
    return ss.str();
}

// ---------------------------------------------------------------------
// CPU brand / model
// ---------------------------------------------------------------------

// "model name" in /proc/cpuinfo is the human-readable brand string,
// e.g. "AMD Ryzen 5 5600X 6-Core Processor"
string CPUInfo::get_cpu_info() {
    ifstream file("/proc/cpuinfo");
    if (!file.is_open()) return "Unknown";

    string line;
    while (getline(file, line)) {
        if (line.find("model name") == 0) {
            size_t colon = line.find(':');
            if (colon != string::npos) {
                return trim(line.substr(colon + 1));
            }
        }
    }
    return "Unknown";
}

// ---------------------------------------------------------------------
// CPU utilization (like Task Manager's overall % bar)
// ---------------------------------------------------------------------

// Reads the aggregate "cpu " line from /proc/stat twice, a short moment
// apart, and compares the deltas. A single snapshot can't give you a
// percentage — /proc/stat only has cumulative tick counters since boot,
// so you need two readings to see how busy the CPU was in between them.
static bool readCpuTicks(unsigned long long& idle, unsigned long long& total) {
    ifstream file("/proc/stat");
    if (!file.is_open()) return false;

    string line;
    getline(file, line); // first line is always the aggregate "cpu  ..." line
    if (line.find("cpu ") != 0) return false;

    istringstream ss(line.substr(4));
    vector<unsigned long long> vals;
    unsigned long long v;
    while (ss >> v) vals.push_back(v);

    // Columns: user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice
    if (vals.size() < 4) return false;

    unsigned long long idleTime = vals[3] + (vals.size() > 4 ? vals[4] : 0); // idle + iowait
    unsigned long long totalTime = 0;
    for (auto x : vals) totalTime += x;

    idle = idleTime;
    total = totalTime;
    return true;
}

float CPUInfo::get_cpu_utilization() {
    unsigned long long idle1 = 0, total1 = 0;
    unsigned long long idle2 = 0, total2 = 0;

    if (!readCpuTicks(idle1, total1)) return 0.0f;
    this_thread::sleep_for(chrono::milliseconds(100)); // short sampling window
    if (!readCpuTicks(idle2, total2)) return 0.0f;

    unsigned long long idleDelta = idle2 - idle1;
    unsigned long long totalDelta = total2 - total1;
    if (totalDelta == 0) return 0.0f;

    float usage = 100.0f * (1.0f - (float)idleDelta / (float)totalDelta);
    if (usage < 0.0f) usage = 0.0f;
    if (usage > 100.0f) usage = 100.0f;
    return usage;
}

// ---------------------------------------------------------------------
// CPU speeds
// ---------------------------------------------------------------------

// "Base speed" = the CPU's rated/nominal frequency, not what it's boosting
// to right now. Try the cleanest source first, then fall back progressively.
string CPUInfo::get_cpu_base_speed() {
    // 1) Kernel-reported base frequency, when available (kHz)
    string base = trim(readFile("/sys/devices/system/cpu/cpu0/cpufreq/base_frequency"));
    if (!base.empty()) {
        return khzToGHzString(stol(base));
    }

    // 2) Many CPUs print their rated speed right in the model name,
    //    e.g. "Intel(R) Core(TM) i7-9750H CPU @ 2.60GHz"
    string model = get_cpu_info();
    smatch match;
    regex ghzPattern(R"(([0-9]+\.[0-9]+)\s*GHz)", regex::icase);
    if (regex_search(model, match, ghzPattern)) {
        return match[1].str() + " GHz";
    }

    // 3) Last resort: the highest frequency the governor is allowed to hit
    string maxFreq = trim(readFile("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq"));
    if (!maxFreq.empty()) {
        return khzToGHzString(stol(maxFreq));
    }

    return "Unknown";
}

// "Current speed" = what the CPU is actually clocked at right now,
// which changes constantly with turbo boost / power saving.
string CPUInfo::get_cpu_speed() {
    // 1) Live scaling driver value, most accurate
    string cur = trim(readFile("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq"));
    if (!cur.empty()) {
        return khzToGHzString(stol(cur));
    }

    // 2) Fallback: "cpu MHz" from /proc/cpuinfo (updates live on most kernels)
    ifstream file("/proc/cpuinfo");
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            if (line.find("cpu MHz") == 0) {
                size_t colon = line.find(':');
                if (colon != string::npos) {
                    double mhz = stod(trim(line.substr(colon + 1)));
                    stringstream ss;
                    ss.precision(2);
                    ss << fixed << (mhz / 1000.0) << " GHz";
                    return ss.str();
                }
            }
        }
    }

    return "Unknown";
}

// ---------------------------------------------------------------------
// CPU topology (sockets / cores / logical processors)
// ---------------------------------------------------------------------

// Number of physical CPU packages/sockets on the motherboard.
// Almost always 1 on a desktop/laptop; 2+ only on server boards.
int CPUInfo::get_cpu_sockets() {
    ifstream file("/proc/cpuinfo");
    if (!file.is_open()) return 1;

    set<string> physicalIds;
    string line;
    while (getline(file, line)) {
        if (line.find("physical id") == 0) {
            size_t colon = line.find(':');
            if (colon != string::npos) {
                physicalIds.insert(trim(line.substr(colon + 1)));
            }
        }
    }

    // Some CPUs/VMs don't report "physical id" at all; assume single socket.
    return physicalIds.empty() ? 1 : (int)physicalIds.size();
}

// Physical core count — counts unique (physical id, core id) pairs so
// hyperthreaded/SMT logical processors don't get double-counted.
int CPUInfo::get_cpu_cores() {
    ifstream file("/proc/cpuinfo");
    if (!file.is_open()) return 1;

    set<string> coreKeys;
    string physId, coreId;
    string line;

    while (getline(file, line)) {
        if (line.find("physical id") == 0) {
            size_t colon = line.find(':');
            if (colon != string::npos) physId = trim(line.substr(colon + 1));
        } else if (line.find("core id") == 0) {
            size_t colon = line.find(':');
            if (colon != string::npos) {
                coreId = trim(line.substr(colon + 1));
                coreKeys.insert(physId + ":" + coreId);
            }
        }
    }

    if (!coreKeys.empty()) return (int)coreKeys.size();

    // Fallback for CPUs/VMs that omit core id: assume no SMT, so
    // physical cores == logical processors.
    return get_cpu_logical_processors();
}

// Logical processors = what the OS scheduler sees, i.e. cores x threads-per-core
int CPUInfo::get_cpu_logical_processors() {
    unsigned int n = thread::hardware_concurrency();
    return n > 0 ? (int)n : 1;
}

// ---------------------------------------------------------------------
// Virtualization support
// ---------------------------------------------------------------------

// Linux doesn't have a single "virtualization: enabled/disabled" flag like
// Windows Task Manager shows. The closest honest answer: check whether the
// CPU advertises hardware virtualization extensions in /proc/cpuinfo flags.
// "vmx" = Intel VT-x, "svm" = AMD-V. Presence means the CPU (and usually the
// BIOS/UEFI setting) support it; it doesn't prove a hypervisor is actively
// using it right now.
string CPUInfo::get_cpu_virtualization() {
    string cpuinfo = readFile("/proc/cpuinfo");
    if (cpuinfo.empty()) return "Unknown";

    // "flags" line only needs to be checked once, values are identical
    // across cores on any normal system.
    if (cpuinfo.find(" vmx") != string::npos || cpuinfo.find("\tvmx") != string::npos) {
        return "Supported (Intel VT-x)";
    }
    if (cpuinfo.find(" svm") != string::npos || cpuinfo.find("\tsvm") != string::npos) {
        return "Supported (AMD-V)";
    }
    return "Not Supported";
}

// ---------------------------------------------------------------------
// Cache sizes
// ---------------------------------------------------------------------

// Walk /sys/devices/system/cpu/cpu0/cache/index*/ and sum up sizes that
// match the requested level. L1 has two caches per core (data + instruction)
// which is why level 1 sums two entries; L2/L3 are usually a single entry.
static string getCacheSizeForLevel(int level) {
    long totalKB = 0;
    bool found = false;

    for (int idx = 0; idx < 8; ++idx) { // 8 is more indices than any real CPU has
        string base = "/sys/devices/system/cpu/cpu0/cache/index" + to_string(idx) + "/";
        string levelStr = trim(readFile(base + "level"));
        if (levelStr.empty()) break; // ran out of cache indices to check

        if (stoi(levelStr) == level) {
            string sizeStr = trim(readFile(base + "size")); // e.g. "32K" or "1024K"
            if (!sizeStr.empty()) {
                // strip trailing 'K' and parse the number
                long kb = stol(sizeStr.substr(0, sizeStr.size() - 1));
                totalKB += kb;
                found = true;
            }
        }
    }

    return found ? formatKB(totalKB) : "Unknown";
}

string CPUInfo::get_cpu_l1_cache() { return getCacheSizeForLevel(1); }
string CPUInfo::get_cpu_l2_cache() { return getCacheSizeForLevel(2); }
string CPUInfo::get_cpu_l3_cache() { return getCacheSizeForLevel(3); }

// ---------------------------------------------------------------------
// System statistics
// ---------------------------------------------------------------------

// /proc/uptime's first number is seconds since boot (as a double, e.g. "123456.78")
string CPUInfo::get_system_uptime() {
    string content = readFile("/proc/uptime");
    if (content.empty()) return "Unknown";

    double totalSeconds = 0.0;
    istringstream ss(content);
    ss >> totalSeconds;

    long seconds = (long)totalSeconds;
    long days = seconds / 86400;
    long hours = (seconds % 86400) / 3600;
    long minutes = (seconds % 3600) / 60;
    long secs = seconds % 60;

    stringstream out;
    if (days > 0) out << days << "d ";
    out << hours << "h " << minutes << "m " << secs << "s";
    return out.str();
}

// Every running process shows up as a numeric folder under /proc,
// e.g. /proc/1234 for PID 1234. Counting those folders = process count.
int CPUInfo::get_process_count() {
    DIR* dir = opendir("/proc");
    if (!dir) return 0;

    int count = 0;
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (isAllDigits(entry->d_name)) count++;
    }
    closedir(dir);
    return count;
}

// Total threads across every process. /proc/[pid]/status has a "Threads:"
// line that already gives the thread count for that one process, so we
// just sum it across every PID folder.
int CPUInfo::get_thread_count() {
    DIR* dir = opendir("/proc");
    if (!dir) return 0;

    int totalThreads = 0;
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (!isAllDigits(entry->d_name)) continue;

        string statusPath = string("/proc/") + entry->d_name + "/status";
        ifstream file(statusPath);
        if (!file.is_open()) continue; // process may have exited mid-scan, that's fine

        string line;
        while (getline(file, line)) {
            if (line.find("Threads:") == 0) {
                size_t colon = line.find(':');
                if (colon != string::npos) {
                    totalThreads += stoi(trim(line.substr(colon + 1)));
                }
                break;
            }
        }
    }
    closedir(dir);
    return totalThreads;
}

// IMPORTANT CAVEAT: "handles" is a Windows concept (one number covering
// open files, registry keys, sockets, mutexes, etc. all at once). Linux
// has no single equivalent. The closest honest analog is system-wide open
// FILE DESCRIPTORS, exposed by /proc/sys/fs/file-nr as three numbers:
// "allocated  unused  max". We return the first (currently in use).
// This will read noticeably lower than Windows' "handles" number for a
// comparable workload, since it only counts files/sockets, not every
// kernel object type Windows lumps into "handles".
int CPUInfo::get_handle_count() {
    string content = readFile("/proc/sys/fs/file-nr");
    if (content.empty()) return 0;

    istringstream ss(content);
    long allocated = 0;
    ss >> allocated;
    return (int)allocated;
}