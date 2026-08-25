#include "CompactAudio.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// Helper to query sound card descriptions from ALSA sysfs
static std::vector<std::string> getAudioDevices() {
    std::vector<std::string> devices;
    std::ifstream file("/proc/asound/cards");
    if (!file.is_open()) return devices;

    std::string line;
    while (std::getline(file, line)) {
        // Line format example: " 0 [PCH            ]: HDA-Intel - HDA Intel PCH"
        size_t colon = line.find("]: ");
        if (colon != std::string::npos) {
            std::string desc = line.substr(colon + 3);
            if (!desc.empty()) {
                // Trim trailing/leading spaces
                size_t first = desc.find_first_not_of(" \t");
                size_t last = desc.find_last_not_of(" \t");
                if (first != std::string::npos && last != std::string::npos) {
                    devices.push_back(desc.substr(first, (last - first + 1)));
                } else {
                    devices.push_back(desc);
                }
            }
        }
    }
    return devices;
}

// Get primary audio output device description
std::string CompactAudio::active_audio_output() {
    auto devices = getAudioDevices();
    if (!devices.empty()) {
        return devices[0];
    }
    return "No speaker found";
}

// Get output status
std::string CompactAudio::active_audio_output_status() {
    auto devices = getAudioDevices();
    return devices.empty() ? "Inactive" : "Active";
}

// Get primary audio input device description
std::string CompactAudio::active_audio_input() {
    auto devices = getAudioDevices();
    if (!devices.empty()) {
        return devices[0]; // On standard ALSA, input is usually handled by the same card
    }
    return "No microphone found";
}

// Get input status
std::string CompactAudio::active_audio_input_status() {
    auto devices = getAudioDevices();
    return devices.empty() ? "Inactive" : "Active";
}
