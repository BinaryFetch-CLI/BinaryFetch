#include "../Platform.h"
#include "../../ExtraInfo.h"
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <cstdio>

using namespace std;
using namespace Platform;

static vector<AudioDevice> parse_pulseaudio_sinks() {
    vector<AudioDevice> devices;
    
    string output = exec("pactl list sinks 2>/dev/null");
    if (output.empty()) return devices;
    
    istringstream iss(output);
    string line;
    AudioDevice current;
    bool inSink = false;
    string defaultSink = exec("pactl get-default-sink 2>/dev/null");
    defaultSink = trim(defaultSink);
    
    while (getline(iss, line)) {
        if (line.find("Sink #") != string::npos) {
            if (inSink && !current.name.empty()) {
                devices.push_back(current);
            }
            current = AudioDevice();
            current.isOutput = true;
            inSink = true;
        } else if (inSink) {
            if (line.find("Name:") != string::npos) {
                size_t pos = line.find("Name:");
                string name = trim(line.substr(pos + 5));
                current.isActive = (name == defaultSink);
            } else if (line.find("Description:") != string::npos) {
                size_t pos = line.find("Description:");
                current.name = trim(line.substr(pos + 12));
            }
        }
    }
    
    if (inSink && !current.name.empty()) {
        devices.push_back(current);
    }
    
    return devices;
}

static vector<AudioDevice> parse_pulseaudio_sources() {
    vector<AudioDevice> devices;
    
    string output = exec("pactl list sources 2>/dev/null");
    if (output.empty()) return devices;
    
    istringstream iss(output);
    string line;
    AudioDevice current;
    bool inSource = false;
    string defaultSource = exec("pactl get-default-source 2>/dev/null");
    defaultSource = trim(defaultSource);
    
    while (getline(iss, line)) {
        if (line.find("Source #") != string::npos) {
            if (inSource && !current.name.empty()) {
                if (current.name.find(".monitor") == string::npos &&
                    current.name.find("Monitor") == string::npos) {
                    devices.push_back(current);
                }
            }
            current = AudioDevice();
            current.isOutput = false;
            inSource = true;
        } else if (inSource) {
            if (line.find("Name:") != string::npos) {
                size_t pos = line.find("Name:");
                string name = trim(line.substr(pos + 5));
                current.isActive = (name == defaultSource);
            } else if (line.find("Description:") != string::npos) {
                size_t pos = line.find("Description:");
                current.name = trim(line.substr(pos + 12));
            }
        }
    }
    
    if (inSource && !current.name.empty()) {
        if (current.name.find(".monitor") == string::npos &&
            current.name.find("Monitor") == string::npos) {
            devices.push_back(current);
        }
    }
    
    return devices;
}

static vector<AudioDevice> parse_alsa_outputs() {
    vector<AudioDevice> devices;
    
    string output = exec("aplay -l 2>/dev/null");
    if (output.empty()) return devices;
    
    istringstream iss(output);
    string line;
    
    while (getline(iss, line)) {
        if (line.find("card") == 0 && line.find("device") != string::npos) {
            AudioDevice dev;
            dev.isOutput = true;
            dev.isActive = (devices.empty());
            
            size_t start = line.find('[');
            size_t end = line.rfind(']');
            if (start != string::npos && end != string::npos && end > start) {
                dev.name = line.substr(start + 1, end - start - 1);
            } else {
                dev.name = line;
            }
            
            devices.push_back(dev);
        }
    }
    
    return devices;
}

static vector<AudioDevice> parse_alsa_inputs() {
    vector<AudioDevice> devices;
    
    string output = exec("arecord -l 2>/dev/null");
    if (output.empty()) return devices;
    
    istringstream iss(output);
    string line;
    
    while (getline(iss, line)) {
        if (line.find("card") == 0 && line.find("device") != string::npos) {
            AudioDevice dev;
            dev.isOutput = false;
            dev.isActive = (devices.empty());
            
            size_t start = line.find('[');
            size_t end = line.rfind(']');
            if (start != string::npos && end != string::npos && end > start) {
                dev.name = line.substr(start + 1, end - start - 1);
            } else {
                dev.name = line;
            }
            
            devices.push_back(dev);
        }
    }
    
    return devices;
}

vector<AudioDevice> ExtraInfo::get_output_devices() {
    vector<AudioDevice> devices = parse_pulseaudio_sinks();
    
    if (devices.empty()) {
        devices = parse_alsa_outputs();
    }
    
    if (devices.empty()) {
        AudioDevice dev;
        dev.name = "Default Audio Output";
        dev.isActive = true;
        dev.isOutput = true;
        devices.push_back(dev);
    }
    
    return devices;
}

vector<AudioDevice> ExtraInfo::get_input_devices() {
    vector<AudioDevice> devices = parse_pulseaudio_sources();
    
    if (devices.empty()) {
        devices = parse_alsa_inputs();
    }
    
    if (devices.empty()) {
        AudioDevice dev;
        dev.name = "Default Audio Input";
        dev.isActive = true;
        dev.isOutput = false;
        devices.push_back(dev);
    }
    
    return devices;
}

PowerStatus ExtraInfo::get_power_status() {
    PowerStatus status;
    status.hasBattery = false;
    status.batteryPercent = 0;
    status.isACOnline = true;
    status.isCharging = false;
    
    const string powerSupplyPath = "/sys/class/power_supply/";
    
    DIR* dir = opendir(powerSupplyPath.c_str());
    if (!dir) {
        return status;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        string name = entry->d_name;
        if (name == "." || name == "..") continue;
        
        string devicePath = powerSupplyPath + name + "/";
        string type = trim(readFile(devicePath + "type"));
        
        if (type == "Battery") {
            status.hasBattery = true;
            
            string capacityStr = trim(readFile(devicePath + "capacity"));
            if (!capacityStr.empty()) {
                try {
                    status.batteryPercent = stoi(capacityStr);
                } catch (...) {
                    status.batteryPercent = 0;
                }
            }
            
            string batteryStatus = trim(readFile(devicePath + "status"));
            status.isCharging = (batteryStatus == "Charging");
            status.isACOnline = (batteryStatus == "Charging" || batteryStatus == "Full" || batteryStatus == "Not charging");
            
        } else if (type == "Mains") {
            string online = trim(readFile(devicePath + "online"));
            if (online == "1") {
                status.isACOnline = true;
            }
        }
    }
    
    closedir(dir);
    return status;
}
