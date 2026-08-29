#include "MemoryInfo.h"
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>
#include <array>
#include <memory>
#include <algorithm>
#include <cctype>
#include <unistd.h>
#include <vector>
#include <iostream>
using namespace std;

MemoryInfo::MemoryInfo() {
    fetchSystemMemory();
    fetchModulesInfo();
}

// ---------- helpers ----------
static string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static bool runCommand(const string& cmd, string& output, int& exitCode) {
    output.clear();
    array<char, 256> buffer;

    FILE* pipe = popen((cmd + " 2>&1").c_str(), "r"); // merge stderr so we can see real errors
    if (!pipe) {
        exitCode = -1;
        return false;
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        output += buffer.data();
    }

    int status = pclose(pipe);
    exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    return true;
}

static bool isRoot() {
    return geteuid() == 0;
}

// ---------- total/free memory ----------
void MemoryInfo::fetchSystemMemory() {
    totalGB = 0;
    freeGB = 0;

    ifstream meminfo("/proc/meminfo");
    if (!meminfo.is_open()) {
        cerr << "[MemoryInfo] ERROR: could not open /proc/meminfo\n";
        return;
    }

    unsigned long long memTotalKB = 0;
    unsigned long long memAvailableKB = 0;
    bool haveTotal = false;
    bool haveAvailable = false;

    string line;
    while (getline(meminfo, line)) {
        if (line.rfind("MemTotal:", 0) == 0) {
            istringstream iss(line.substr(string("MemTotal:").size()));
            iss >> memTotalKB;
            haveTotal = true;
        }
        else if (line.rfind("MemAvailable:", 0) == 0) {
            istringstream iss(line.substr(string("MemAvailable:").size()));
            iss >> memAvailableKB;
            haveAvailable = true;
        }
        if (haveTotal && haveAvailable) break;
    }

    if (haveTotal) {
        const unsigned long long kbPerGB = 1024ULL * 1024ULL;
        totalGB = static_cast<int>((memTotalKB + kbPerGB - 1) / kbPerGB);
    }
    if (haveAvailable) {
        const unsigned long long kbPerGB = 1024ULL * 1024ULL;
        freeGB = static_cast<int>(memAvailableKB / kbPerGB);
    }
}

// ---------- Method 1: dmidecode (text parsing) ----------
static bool fetchModulesViaDmidecode(vector<MemoryModule>& outModules) {
    static const char* candidates[] = {
        "dmidecode --type 17",
        "/usr/sbin/dmidecode --type 17",
        "/sbin/dmidecode --type 17"
    };

    string output;
    int exitCode = -1;
    bool ran = false;

    for (const char* cmd : candidates) {
        if (runCommand(cmd, output, exitCode)) {
            ran = true;
            if (exitCode == 0 && !output.empty()) break; // success, stop trying
        }
    }

    if (!ran) {
        cerr << "[MemoryInfo] dmidecode: popen() failed entirely (shell unavailable?)\n";
        return false;
    }

    if (exitCode != 0) {
        cerr << "[MemoryInfo] dmidecode exited with code " << exitCode
             << ". Output was:\n" << output << "\n";
        if (output.find("Permission denied") != string::npos || !isRoot()) {
            cerr << "[MemoryInfo] Likely cause: not running as root. Try: sudo ./yourprogram\n";
        }
        else if (output.find("not found") != string::npos) {
            cerr << "[MemoryInfo] Likely cause: dmidecode is not installed. Try: sudo apt install dmidecode\n";
        }
        return false;
    }

    istringstream stream(output);
    string line;
    bool inBlock = false;
    bool have = false;
    MemoryModule current;

    auto flush = [&]() {
        if (!have) return;
        if (!current.capacity.empty() && current.capacity != "No Module Installed") {
            outModules.push_back(current);
        }
        current = MemoryModule();
        have = false;
    };

    while (getline(stream, line)) {
        string trimmed = trim(line);

        if (trimmed.rfind("Memory Device", 0) == 0 && trimmed.find("Array") == string::npos) {
            flush();
            inBlock = true;
            have = true;
            continue;
        }
        if (trimmed.empty()) {
            if (inBlock) { flush(); inBlock = false; }
            continue;
        }
        if (!inBlock) continue;

        size_t colon = trimmed.find(':');
        if (colon == string::npos) continue;

        string key = trim(trimmed.substr(0, colon));
        string value = trim(trimmed.substr(colon + 1));

        if (key == "Size") {
            current.capacity = value;
            if (value != "No Module Installed") {
                istringstream vs(value);
                double amount = 0;
                string unit;
                vs >> amount >> unit;
                for (auto& c : unit) c = static_cast<char>(toupper(c));
                if (unit == "MB") {
                    int gb = static_cast<int>((amount + 1023) / 1024);
                    current.capacity = to_string(gb) + "GB";
                } else if (unit == "GB") {
                    current.capacity = to_string(static_cast<int>(amount)) + "GB";
                }
            }
        }
        else if (key == "Type" && current.type.empty()) {
            current.type = value;
        }
        else if (key == "Speed") {
            current.speed = value;
        }
    }
    flush();

    if (outModules.empty()) {
        cerr << "[MemoryInfo] dmidecode ran successfully but reported no populated memory slots.\n";
    }
    return !outModules.empty();
}

// ---------- Method 2: raw SMBIOS table parsing (no external binary needed) ----------
static string decodeSmbiosMemoryType(uint8_t code) {
    switch (code) {
        case 0x11: return "DDR";
        case 0x12: return "DDR2";
        case 0x13: return "DDR2 FB-DIMM";
        case 0x18: return "DDR3";
        case 0x1A: return "DDR4";
        case 0x1B: return "LPDDR";
        case 0x1C: return "LPDDR2";
        case 0x1D: return "LPDDR3";
        case 0x1E: return "LPDDR4";
        case 0x22: return "DDR5";
        case 0x23: return "LPDDR5";
        default:   return "Unknown";
    }
}

static bool fetchModulesViaSysfsDMI(vector<MemoryModule>& outModules) {
    const char* path = "/sys/firmware/dmi/tables/DMI";
    ifstream file(path, ios::binary);
    if (!file.is_open()) {
        cerr << "[MemoryInfo] Could not open " << path
             << " (need root, or kernel does not expose it on this system)\n";
        return false;
    }

    vector<unsigned char> data(
        (istreambuf_iterator<char>(file)),
        istreambuf_iterator<char>()
    );
    file.close();

    if (data.empty()) {
        cerr << "[MemoryInfo] " << path << " was empty (likely no read permission)\n";
        return false;
    }

    size_t offset = 0;
    while (offset + 4 <= data.size()) {
        uint8_t type = data[offset];
        uint8_t length = data[offset + 1];

        if (type == 127) break; // End-of-Table marker
        if (length < 4 || offset + length > data.size()) break; // malformed, bail out safely

        size_t structStart = offset;
        size_t stringsStart = offset + length;

        // Collect the string table that follows the formatted section
        vector<string> strings;
        size_t pos = stringsStart;
        while (pos < data.size()) {
            if (data[pos] == 0) {
                if (pos == stringsStart) {
                    // empty string table entirely (no strings at all)
                    pos += 1;
                    break;
                }
                pos += 1;
                if (pos < data.size() && data[pos - 1] == 0 &&
                    (strings.empty() || true)) {
                    // check for double-null terminator
                }
                if (pos < data.size() && data[pos] == 0) {
                    pos += 1;
                    break;
                }
                continue;
            }
            string s;
            while (pos < data.size() && data[pos] != 0) {
                s += static_cast<char>(data[pos]);
                pos++;
            }
            strings.push_back(s);
            pos++; // skip null terminator
            if (pos < data.size() && data[pos] == 0) {
                pos++;
                break;
            }
        }

        if (type == 17 && length >= 0x15) {
            MemoryModule module;

            uint16_t sizeField = data[structStart + 0x0C] | (data[structStart + 0x0D] << 8);

            if (sizeField == 0) {
                // slot empty, skip
                offset = pos;
                continue;
            }

            unsigned long long sizeMB = 0;
            bool sizeKnown = true;

            if (sizeField == 0xFFFF) {
                sizeKnown = false;
            }
            else if (sizeField == 0x7FFF && length >= 0x20) {
                uint32_t ext = data[structStart + 0x1C]
                             | (data[structStart + 0x1D] << 8)
                             | (data[structStart + 0x1E] << 16)
                             | (data[structStart + 0x1F] << 24);
                sizeMB = ext;
            }
            else {
                bool isKB = (sizeField & 0x8000) != 0;
                unsigned long long raw = sizeField & 0x7FFF;
                sizeMB = isKB ? (raw / 1024) : raw;
            }

            if (!sizeKnown) {
                module.capacity = "Unknown";
            } else {
                unsigned long long gb = (sizeMB + 1023) / 1024; // round up to nearest GB
                module.capacity = to_string(gb) + "GB";
            }

            uint8_t memType = data[structStart + 0x12];
            module.type = decodeSmbiosMemoryType(memType);

            if (length >= 0x17) {
                uint16_t speedField = data[structStart + 0x15] | (data[structStart + 0x16] << 8);
                if (speedField == 0 || speedField == 0xFFFF) {
                    module.speed = "Unknown MT/s";
                } else {
                    module.speed = to_string(speedField) + " MT/s";
                }
            } else {
                module.speed = "Unknown MT/s";
            }

            outModules.push_back(module);
        }

        offset = pos;
    }

    if (outModules.empty()) {
        cerr << "[MemoryInfo] Parsed SMBIOS table but found no populated memory slots.\n";
    }
    return !outModules.empty();
}

// ---------- combined entry point ----------
void MemoryInfo::fetchModulesInfo() {
    modules.clear();

    if (!isRoot()) {
        cerr << "[MemoryInfo] WARNING: not running as root. "
                "Memory module details require root (SMBIOS/DMI tables are protected). "
                "Re-run with sudo.\n";
    }

    if (fetchModulesViaDmidecode(modules)) {
        return;
    }

    cerr << "[MemoryInfo] Falling back to direct SMBIOS table parsing...\n";

    if (fetchModulesViaSysfsDMI(modules)) {
        return;
    }

    cerr << "[MemoryInfo] Both methods failed. No module data available. "
            "Make sure you are running as root (sudo) and that either dmidecode "
            "is installed or /sys/firmware/dmi/tables/DMI is readable.\n";
}

// ---------- accessors ----------
int MemoryInfo::getTotal() const { return totalGB; }
int MemoryInfo::getFree() const { return freeGB; }
int MemoryInfo::getUsedPercentage() const {
    if (totalGB == 0) return 0;
    double percentage = (static_cast<double>(totalGB - freeGB) / totalGB) * 100;
    if (percentage > 100.0) percentage = 100.0;
    if (percentage < 0.0) percentage = 0.0;
    return static_cast<int>(percentage);
}
const vector<MemoryModule>& MemoryInfo::getModules() const { return modules; }