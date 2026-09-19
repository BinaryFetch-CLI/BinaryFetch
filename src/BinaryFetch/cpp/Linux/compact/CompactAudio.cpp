// CompactAudio.cpp
#include "CompactAudio.h"
#include <array>
#include <algorithm>
#include <cstdio>
#include <memory>
#include <sstream>

namespace {

// ---------------- generic helpers ----------------

string runCommand(const string& cmd) {
    std::array<char, 256> buffer;
    string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) return "";
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

string trim(const string& s) {
    size_t first = s.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    size_t last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

bool startsWith(const string& s, const string& prefix) {
    return s.rfind(prefix, 0) == 0;
}

bool commandExists(const string& bin) {
    string check = "command -v " + bin + " >/dev/null 2>&1 && echo yes";
    return trim(runCommand(check)) == "yes";
}

// ---------------- pactl-based helpers (used for names/volume, and as fallback) ----------------

string extractField(const string& blob, const string& deviceName, const string& fieldName) {
    std::istringstream stream(blob);
    string line;
    bool inTargetBlock = false;
    string result;

    while (std::getline(stream, line)) {
        string trimmed = trim(line);

        if (startsWith(trimmed, "Name: ")) {
            inTargetBlock = (trim(trimmed.substr(6)) == deviceName);
        }
        if (inTargetBlock && startsWith(trimmed, fieldName + ": ")) {
            result = trim(trimmed.substr(fieldName.size() + 2));
            break;
        }
    }
    return result;
}

string getDeviceIndex(const string& listCmd, const string& deviceName) {
    string output = runCommand(listCmd);
    std::istringstream stream(output);
    string line;
    while (std::getline(stream, line)) {
        std::istringstream lineStream(line);
        string index, name;
        std::getline(lineStream, index, '\t');
        std::getline(lineStream, name, '\t');
        if (name == deviceName) return index;
    }
    return "";
}

string getDefaultSink() {
    return trim(runCommand("pactl get-default-sink 2>/dev/null"));
}

string getDefaultSource() {
    return trim(runCommand("pactl get-default-source 2>/dev/null"));
}

bool isMeterOnlyClient(const string& appName) {
    static const std::array<string, 3> meterApps = {
        "PulseAudio Volume Control",
        "pavucontrol",
        "peak detect"
    };
    string lower = appName;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    for (const auto& meter : meterApps) {
        string m = meter;
        std::transform(m.begin(), m.end(), m.begin(), ::tolower);
        if (lower.find(m) != string::npos) return true;
    }
    return false;
}

// Fallback mic-active check via pactl source-outputs + Corked.
// Only used when pw-dump/jq aren't available (i.e. not PipeWire).
bool micActiveViaPactl(const string& sourceName) {
    string sourceIndex = getDeviceIndex("pactl list short sources 2>/dev/null", sourceName);
    if (sourceIndex.empty()) return false;

    string outputsInfo = runCommand("pactl list source-outputs 2>/dev/null");
    std::istringstream stream(outputsInfo);
    string line, currentSource, appName;
    bool corked = true;

    auto flushBlock = [&]() -> bool {
        return (currentSource == sourceIndex && !corked && !isMeterOnlyClient(appName));
    };

    while (std::getline(stream, line)) {
        string trimmed = trim(line);
        if (startsWith(trimmed, "Source Output #")) {
            if (flushBlock()) return true;
            currentSource.clear();
            corked = true;
            appName.clear();
        }
        if (startsWith(trimmed, "Source: ")) currentSource = trim(trimmed.substr(8));
        if (startsWith(trimmed, "Corked: ")) corked = (trim(trimmed.substr(8)) == "yes");
        if (startsWith(trimmed, "application.name = ")) {
            string v = trim(trimmed.substr(20));
            if (v.size() >= 2 && v.front() == '"' && v.back() == '"') v = v.substr(1, v.size() - 2);
            appName = v;
        }
    }
    return flushBlock();
}

// ---------------- PipeWire link-graph based helper (accurate path) ----------------

// True if the PipeWire node named `sourceName` currently has at least one
// ACTIVE link carrying audio out of it -- i.e. something is really capturing
// right now, not just holding an open-but-idle stream (which is what makes
// pactl's "Corked" flag unreliable under pipewire-pulse).
bool micHasActiveLinkPipewire(const string& sourceName) {
    string idCmd =
        "pw-dump 2>/dev/null | jq -r '.[] | "
        "select(.info.props[\"node.name\"]==\"" + sourceName + "\") | .id' 2>/dev/null";
    string nodeId = trim(runCommand(idCmd));
    if (nodeId.empty()) return false;

    string linkCmd =
        "pw-dump 2>/dev/null | jq -r '.[] | "
        "select(.type==\"PipeWire:Interface:Link\") | "
        "select(.info.props[\"link.output.node\"]==" + nodeId + ") | "
        "select(.info.state==\"active\") | .id' 2>/dev/null";
    string activeLinks = trim(runCommand(linkCmd));
    return !activeLinks.empty();
}

} // namespace

// ---------------- OUTPUT ----------------

string CompactAudio::active_audio_output() {
    string sink = getDefaultSink();
    if (sink.empty()) return "No speaker found";

    string sinksInfo = runCommand("pactl list sinks 2>/dev/null");
    string desc = extractField(sinksInfo, sink, "Description");
    return desc.empty() ? sink : desc;
}

// "(Active)" if a default sink exists, isn't muted, and volume > 0%.
// Otherwise "(Inactive)" -- kept to the header's two documented values.
string CompactAudio::active_audio_output_status() {
    string sink = getDefaultSink();
    if (sink.empty()) return "Inactive";

    string sinksInfo = runCommand("pactl list sinks 2>/dev/null");

    string mute = extractField(sinksInfo, sink, "Mute");
    if (mute == "yes") return "Inactive";

    string volumeLine = extractField(sinksInfo, sink, "Volume");
    size_t pct = volumeLine.find('%');
    if (pct != string::npos) {
        size_t start = volumeLine.rfind('/', pct - 2);
        string numStr = trim(volumeLine.substr(start + 1, pct - start - 1));
        int percent = 0;
        try { percent = std::stoi(numStr); } catch (...) {}
        return percent > 0 ? "Active" : "Inactive";
    }
    return "(Inactive)";
}

// ---------------- INPUT ----------------

string CompactAudio::active_audio_input() {
    string source = getDefaultSource();
    if (source.empty()) return "No microphone found";

    string sourcesInfo = runCommand("pactl list sources 2>/dev/null");
    string desc = extractField(sourcesInfo, source, "Description");
    return desc.empty() ? source : desc;
}

// "(Active)" only when audio is genuinely flowing out of the mic right now.
// Primary path: PipeWire's link graph (pw-dump + jq) -- accurate even under
// pipewire-pulse, where pactl's "Corked" flag is not trustworthy.
// Fallback path: pactl source-outputs + Corked, for non-PipeWire systems.
string CompactAudio::active_audio_input_status() {
    string source = getDefaultSource();
    if (source.empty()) return "Inactive";

    if (commandExists("pw-dump") && commandExists("jq")) {
        return micHasActiveLinkPipewire(source) ? "Active" : "Inactive";
    }

    return micActiveViaPactl(source) ? "Active" : "Inactive";
}