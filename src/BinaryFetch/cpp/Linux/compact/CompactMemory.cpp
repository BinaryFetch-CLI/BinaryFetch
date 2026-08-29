#include "CompactMemory.h"
#include <fstream>
#include <sstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <array>
#include <iostream>

// Read one value (in kB) from /proc/meminfo, e.g. "MemTotal", "MemFree"...
static double getMeminfoValue(const std::string& key) {
    std::ifstream file("/proc/meminfo");
    if (!file.is_open()) return 0.0;

    std::string line;
    while (std::getline(file, line)) {
        if (line.find(key + ":") == 0) {
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                double valKB = 0.0;
                std::stringstream ss(line.substr(colon + 1));
                if (ss >> valKB) {
                    return valKB * 1024.0; // kB -> bytes
                }
            }
        }
    }
    return 0.0;
}

// Strip leading/trailing whitespace (dmidecode indents fields with tabs)
static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Run a command, capturing stdout separately from stderr so we can tell
// "worked but empty" apart from "failed and here's why".
static std::string runCommand(const std::string& cmd, std::string* errOut = nullptr) {
    std::string result;
    std::array<char, 256> buffer;

    std::string tmpErrFile = "/tmp/compactmemory_dmidecode_err.XXXXXX";
    // Redirect stderr to a temp file so we can inspect it for diagnostics.
    std::string fullCmd = cmd + " 2>/tmp/compactmemory_dmidecode_err.log";

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(fullCmd.c_str(), "r"), pclose);
    if (!pipe) return "";

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    if (errOut) {
        std::ifstream errFile("/tmp/compactmemory_dmidecode_err.log");
        if (errFile.is_open()) {
            std::stringstream ss;
            ss << errFile.rdbuf();
            *errOut = ss.str();
        }
    }
    std::remove("/tmp/compactmemory_dmidecode_err.log");

    return result;
}

// Try, in order:
//   1) plain dmidecode   -> works if already root
//   2) sudo -n dmidecode -> works if a NOPASSWD sudoers rule was set up (recommended, see chat)
//   3) pkexec dmidecode  -> works on a normal desktop with a polkit agent (one-time GUI prompt)
// Prints a diagnostic to stderr explaining exactly what happened, since this is the
// #1 point of confusion (silently doing nothing looks identical to "no RAM info exists").
static std::string getDmidecodeOutput() {
    struct Attempt { const char* label; const char* cmd; };
    static const Attempt attempts[] = {
        { "plain dmidecode",     "dmidecode -t memory" },
        { "sudo -n dmidecode",   "sudo -n dmidecode -t memory" },
        { "pkexec dmidecode",    "pkexec dmidecode -t memory" },
    };

    for (const auto& a : attempts) {
        std::string err;
        std::string output = runCommand(a.cmd, &err);

        // IMPORTANT: don't just check "non-empty". Even when dmidecode can't
        // read the DMI table (no permission), it still prints a couple of
        // harmless banner lines to stdout ("# dmidecode 3.7", "Getting SMBIOS
        // data from sysfs.") before the permission error. That makes output
        // non-empty despite containing zero real data, which previously
        // caused this attempt to be wrongly accepted as a success and skip
        // the sudo/pkexec fallbacks entirely. Require real content instead.
        if (output.find("Memory Device") != std::string::npos) {
            return output; // genuine success, stop here
        }

        // Report why this attempt failed, so it's not a silent 0/0.
        std::cerr << "[CompactMemory] " << a.label << " didn't return real memory data.";
        if (!err.empty()) {
            std::cerr << " stderr: " << trim(err);
        } else if (!output.empty()) {
            std::cerr << " stdout was: " << trim(output);
        }
        std::cerr << "\n";
    }

    std::cerr << "[CompactMemory] All methods failed. Likely causes:\n"
                 "  - dmidecode not installed (sudo apt/dnf/pacman install dmidecode)\n"
                 "  - no root access and no passwordless sudo/polkit configured\n"
                 "  Fix: sudo visudo, add a NOPASSWD rule for dmidecode (see chat for exact command)\n";
    return "";
}

// Parse `dmidecode -t memory` output into slot counts.
// ok=false means we couldn't read DMI data at all (see stderr for why).
static void countMemorySlots(int& total, int& used, bool& ok) {
    total = 0;
    used = 0;
    ok = false;

    std::string output = getDmidecodeOutput();
    if (output.empty()) return;

    ok = true;
    std::istringstream stream(output);
    std::string rawLine;
    bool inDevice = false;

    while (std::getline(stream, rawLine)) {
        std::string line = trim(rawLine);

        // A physical slot section header is the line "Memory Device" *exactly*.
        // (Don't match "Memory Device Mapped Address", a different DMI type.)
        if (line == "Memory Device") {
            inDevice = true;
            total++;
            continue;
        }

        // Inside a slot section, "Size:" tells us if it's populated.
        // Must be the START of the trimmed line so "Range Size:" doesn't count.
        if (inDevice && line.rfind("Size:", 0) == 0) {
            if (line.find("No Module Installed") == std::string::npos) {
                used++;
            }
            inDevice = false;
        }
    }

    if (total == 0) {
        std::cerr << "[CompactMemory] dmidecode ran, but no \"Memory Device\" entries were "
                     "found in its output. Your dmidecode version/output format may differ.\n";
    }
}

// Get total physical RAM in GB
double CompactMemory::get_total_memory() {
    double bytes = getMeminfoValue("MemTotal");
    return bytes / (1024.0 * 1024.0 * 1024.0);
}

// Get free/available physical RAM in GB
double CompactMemory::get_free_memory() {
    double availableBytes = getMeminfoValue("MemAvailable");
    if (availableBytes == 0.0) {
        double free = getMeminfoValue("MemFree");
        double buffers = getMeminfoValue("Buffers");
        double cached = getMeminfoValue("Cached");
        availableBytes = free + buffers + cached;
    }
    return availableBytes / (1024.0 * 1024.0 * 1024.0);
}

// Get percent of memory used
double CompactMemory::get_used_memory_percent() {
    double total = getMeminfoValue("MemTotal");
    double available = getMeminfoValue("MemAvailable");
    if (available == 0.0) {
        double free = getMeminfoValue("MemFree");
        double buffers = getMeminfoValue("Buffers");
        double cached = getMeminfoValue("Cached");
        available = free + buffers + cached;
    }
    if (total <= 0.0) return 0.0;
    return 100.0 * (total - available) / total;
}

// How many RAM slots are filled. Returns -1 if DMI data couldn't be read
// (check stderr output for the exact reason).
int CompactMemory::memory_slot_used() {
    int total = 0, used = 0;
    bool ok = false;
    countMemorySlots(total, used, ok);
    return ok ? used : -1;
}

// Total physical RAM slots on the motherboard. Returns -1 if DMI data
// couldn't be read (check stderr output for the exact reason).
int CompactMemory::memory_slot_available() {
    int total = 0, used = 0;
    bool ok = false;
    countMemorySlots(total, used, ok);
    return ok ? total : -1;
}