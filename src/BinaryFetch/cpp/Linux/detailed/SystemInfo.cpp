#include "SystemInfo.h"
#include <fstream>
#include <sstream>
#include <array>
#include <memory>
#include <cstdio>
#include <unistd.h>
#include <iostream>
using namespace std;

SystemInfo::SystemInfo() {}
SystemInfo::~SystemInfo() {}

//  helpers 
static string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
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

//  kept the original signature/name exactly as declared in the header 
// On Windows this read a registry subkey+value. On Linux there is no registry,
// so we repurpose it as a DMI/SMBIOS field reader: "subkey" is unused (kept only
// to match the header without editing it), "valueName" is the DMI field name
// under /sys/class/dmi/id/.
string SystemInfo::read_registry_value(const std::string& subkey, const std::string& valueName) {
    (void)subkey; // unused on Linux - no registry hive concept exists here

    string path = "/sys/class/dmi/id/" + valueName;
    ifstream file(path);

    if (!file.is_open()) {
        cerr << "[SystemInfo] Could not open " << path;
        if (!isRoot()) {
            cerr << " (some DMI fields require root - try sudo)";
        }
        cerr << "\n";
        return "Unknown";
    }

    string value;
    getline(file, value);
    value = trim(value);

    if (value.empty()) {
        return "Unknown";
    }
    return value;
}

//  BIOS info 
string SystemInfo::get_bios_vendor() {
    string value = read_registry_value("", "bios_vendor");
    if (value != "Unknown") return value;

    string output;
    if (runCommand("dmidecode -s bios-vendor", output)) {
        string trimmed = trim(output);
        if (!trimmed.empty()) return trimmed;
    }
    return "Unknown";
}

string SystemInfo::get_bios_version() {
    string value = read_registry_value("", "bios_version");
    if (value != "Unknown") return value;

    string output;
    if (runCommand("dmidecode -s bios-version", output)) {
        string trimmed = trim(output);
        if (!trimmed.empty()) return trimmed;
    }
    return "Unknown";
}

string SystemInfo::get_bios_date() {
    string value = read_registry_value("", "bios_date");
    if (value != "Unknown") return value;

    string output;
    if (runCommand("dmidecode -s bios-release-date", output)) {
        string trimmed = trim(output);
        if (!trimmed.empty()) return trimmed;
    }
    return "Unknown";
}

//  Motherboard info 
string SystemInfo::get_motherboard_model() {
    string value = read_registry_value("", "board_name");
    if (value != "Unknown") return value;

    string output;
    if (runCommand("dmidecode -s baseboard-product-name", output)) {
        string trimmed = trim(output);
        if (!trimmed.empty()) return trimmed;
    }
    return "Unknown";
}

string SystemInfo::get_motherboard_manufacturer() {
    string value = read_registry_value("", "board_vendor");
    if (value != "Unknown") return value;

    string output;
    if (runCommand("dmidecode -s baseboard-manufacturer", output)) {
        string trimmed = trim(output);
        if (!trimmed.empty()) return trimmed;
    }
    return "Unknown";
}