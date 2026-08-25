#pragma once
#include <string>
#include <vector>
using namespace std;

// Structure to hold audio device information
struct AudioDevice {
    string name;        // Device friendly name
    bool isActive;      // Whether this is the default/active device
    bool isOutput;      // True for output devices, false for input
};

// Structure to hold power status information
struct PowerStatus {
    bool hasBattery;        // Whether system has a battery
    int batteryPercent;     // Battery charge percentage (0-100)
    bool isACOnline;        // Whether AC power is connected
    bool isCharging;        // Whether battery is currently charging
};

class ExtraInfo {
public:
    vector<AudioDevice> get_output_devices();  // Get all output devices (speakers/headphones)
    vector<AudioDevice> get_input_devices();   // Get all input devices (microphones)
    PowerStatus get_power_status();            // Get power status information
};