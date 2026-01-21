#include "../Platform.h"
#include "../../CompactAudio.h"
#include <fstream>
#include <sstream>

static std::string cachedOutputDevice;
static std::string cachedInputDevice;
static bool devicesCached = false;

static void cacheAudioDevices() {
    if (devicesCached) return;
    
    std::string mixerOutput = Platform::exec("mixer -s 2>/dev/null");
    
    if (!mixerOutput.empty()) {
        std::istringstream iss(mixerOutput);
        std::string line;
        bool foundOutput = false;
        bool foundInput = false;
        
        while (std::getline(iss, line)) {
            if (line.find("vol") != std::string::npos && !foundOutput) {
                cachedOutputDevice = "OSS Audio Output";
                foundOutput = true;
            }
            if (line.find("mic") != std::string::npos && !foundInput) {
                cachedInputDevice = "OSS Microphone";
                foundInput = true;
            }
            if (line.find("rec") != std::string::npos && !foundInput) {
                cachedInputDevice = "OSS Recording Input";
                foundInput = true;
            }
        }
    }
    
    if (cachedOutputDevice.empty()) {
        std::string sndstat = Platform::readFile("/dev/sndstat");
        if (!sndstat.empty()) {
            std::istringstream iss(sndstat);
            std::string line;
            while (std::getline(iss, line)) {
                if (line.find("pcm") != std::string::npos && line.find("play") != std::string::npos) {
                    size_t bracket = line.find('<');
                    size_t bracket2 = line.find('>');
                    if (bracket != std::string::npos && bracket2 != std::string::npos) {
                        cachedOutputDevice = line.substr(bracket + 1, bracket2 - bracket - 1);
                        break;
                    }
                }
            }
        }
    }
    
    if (cachedInputDevice.empty()) {
        std::string sndstat = Platform::readFile("/dev/sndstat");
        if (!sndstat.empty()) {
            std::istringstream iss(sndstat);
            std::string line;
            while (std::getline(iss, line)) {
                if (line.find("pcm") != std::string::npos && line.find("rec") != std::string::npos) {
                    size_t bracket = line.find('<');
                    size_t bracket2 = line.find('>');
                    if (bracket != std::string::npos && bracket2 != std::string::npos) {
                        cachedInputDevice = line.substr(bracket + 1, bracket2 - bracket - 1);
                        break;
                    }
                }
            }
        }
    }
    
    if (cachedOutputDevice.empty()) cachedOutputDevice = "Unknown Audio Output";
    if (cachedInputDevice.empty()) cachedInputDevice = "Unknown Audio Input";
    
    devicesCached = true;
}

std::string CompactAudio::active_audio_output() {
    cacheAudioDevices();
    return cachedOutputDevice;
}

std::string CompactAudio::active_audio_output_status() {
    return "Active";
}

std::string CompactAudio::active_audio_input() {
    cacheAudioDevices();
    return cachedInputDevice;
}

std::string CompactAudio::active_audio_input_status() {
    return "Active";
}
