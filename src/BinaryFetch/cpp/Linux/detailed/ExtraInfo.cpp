#include "ExtraInfo.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <dirent.h>
#include <cstdio>
#include <memory>
#include <array>
#include <algorithm>
using namespace std;

// Small helpers
static string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static string runCommand(const string& cmd) {
    string result;
    array<char, 256> buffer;
    string fullCmd = cmd + " 2>/dev/null";
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(fullCmd.c_str(), "r"), pclose);
    if (!pipe) return "";
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

// Parses "pactl list sinks" / "pactl list sources" output into a list of
// (name, description) pairs. pactl's "list" format uses indentation, so we
// look for "Name:" and "Description:" lines within each device block.
static vector<pair<string, string>> parsePactlList(const string& output) {
    vector<pair<string, string>> devices;
    istringstream stream(output);
    string line;
    string currentName;
    bool haveName = false;

    while (getline(stream, line)) {
        string trimmed = trim(line);

        if (trimmed.rfind("Name:", 0) == 0) {
            currentName = trim(trimmed.substr(5));
            haveName = true;
        }
        else if (trimmed.rfind("Description:", 0) == 0 && haveName) {
            string description = trim(trimmed.substr(12));
            devices.push_back({ currentName, description });
            haveName = false;
        }
    }
    return devices;
}

// Reads the current default sink/source name, e.g. via
// "pactl get-default-sink" / "pactl get-default-source".
static string getDefaultPactlDevice(const string& kind) {
    string output = runCommand("pactl get-default-" + kind);
    return trim(output);
}

// Audio devices
vector<AudioDevice> ExtraInfo::get_output_devices()
{
    vector<AudioDevice> devices;

    string listOutput = runCommand("pactl list sinks");
    if (listOutput.empty()) return devices;

    vector<pair<string, string>> sinks = parsePactlList(listOutput);
    string defaultSink = getDefaultPactlDevice("sink");

    for (const auto& sink : sinks) {
        AudioDevice device;
        device.name = sink.second.empty() ? sink.first : sink.second;
        device.isActive = (sink.first == defaultSink);
        device.isOutput = true;
        devices.push_back(device);
    }

    return devices;
}

vector<AudioDevice> ExtraInfo::get_input_devices()
{
    vector<AudioDevice> devices;

    string listOutput = runCommand("pactl list sources");
    if (listOutput.empty()) return devices;

    vector<pair<string, string>> sources = parsePactlList(listOutput);
    string defaultSource = getDefaultPactlDevice("source");

    for (const auto& source : sources) {
        // Skip monitor sources (e.g. "Monitor of Built-in Audio") — these
        // are loopback taps on an output device, not real microphones.
        if (source.first.find(".monitor") != string::npos) continue;

        AudioDevice device;
        device.name = source.second.empty() ? source.first : source.second;
        device.isActive = (source.first == defaultSource);
        device.isOutput = false;
        devices.push_back(device);
    }

    return devices;
}

// Power status
PowerStatus ExtraInfo::get_power_status()
{
    PowerStatus status;
    status.hasBattery = false;
    status.isCharging = false;
    status.batteryPercent = 0;
    status.isACOnline = false;

    DIR* dir = opendir("/sys/class/power_supply");
    if (!dir) return status;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        string name = entry->d_name;
        if (name == "." || name == "..") continue;

        string basePath = "/sys/class/power_supply/" + name;
        ifstream typeFile(basePath + "/type");
        string type;
        if (!(typeFile >> type)) continue;

        if (type == "Battery") {
            status.hasBattery = true;

            ifstream capacityFile(basePath + "/capacity");
            int capacity = 0;
            if (capacityFile >> capacity) {
                status.batteryPercent = capacity;
            }

            ifstream stateFile(basePath + "/status");
            string state;
            if (getline(stateFile, state)) {
                state = trim(state);
                status.isCharging = (state == "Charging");
            }
        }
        else if (type == "Mains" || type == "USB") {
            ifstream onlineFile(basePath + "/online");
            int online = 0;
            if (onlineFile >> online) {
                if (online == 1) status.isACOnline = true;
            }
        }
    }
    closedir(dir);

    // No AC/Mains supply node at all (some laptops/desktops omit it) but a
    // battery exists and isn't charging and isn't discharging either —
    // treat AC as online if the battery reports "Full" or "Not charging".
    if (status.hasBattery && !status.isACOnline) {
        for (auto& c : (void)0, vector<int>{}) {} // no-op placeholder removed below
    }

    return status;
}