#include "../../CompactAudio.h"
#include "../Platform.h"
#include <sstream>
#include <vector>
#include <algorithm>

#if PLATFORM_LINUX

static std::string cachedOutputDevice;
static std::string cachedInputDevice;
static bool devicesCached = false;

static std::string shortenDeviceName(const std::string& name) {
    std::string result = name;
    
    std::vector<std::string> removePatterns = {
        "Family 17h (Models 00h-0fh) ",
        "Family 17h ",
        "(Models 00h-0fh) ",
        "High Definition Audio Controller ",
        "HD Audio Controller ",
        "Audio Controller ",
        "Controller ",
        "(High Definition Audio Device)",
        "(High Definition Audio)",
        "High Definition Audio Device",
    };
    
    for (const auto& pattern : removePatterns) {
        size_t pos;
        while ((pos = result.find(pattern)) != std::string::npos) {
            result.erase(pos, pattern.length());
        }
    }
    
    while (!result.empty() && result[0] == ' ') result.erase(0, 1);
    while (!result.empty() && result.back() == ' ') result.pop_back();
    
    while (result.find("  ") != std::string::npos) {
        size_t pos = result.find("  ");
        result.erase(pos, 1);
    }
    
    if (result.empty()) result = name;
    
    return result;
}

static void cacheAudioDevices() {
    if (devicesCached) return;
    
    if (Platform::commandExists("wpctl")) {
        std::string status = Platform::exec("wpctl status 2>/dev/null");
        if (!status.empty()) {
            std::istringstream iss(status);
            std::string line;
            bool inSinks = false;
            bool inSources = false;
            
            while (std::getline(iss, line)) {
                if (line.find("Sinks:") != std::string::npos) {
                    inSinks = true;
                    inSources = false;
                    continue;
                }
                if (line.find("Sources:") != std::string::npos) {
                    inSinks = false;
                    inSources = true;
                    continue;
                }
                if (line.find("Filters:") != std::string::npos || 
                    line.find("Streams:") != std::string::npos) {
                    inSinks = false;
                    inSources = false;
                    continue;
                }
                
                if ((inSinks || inSources) && line.find("*") != std::string::npos) {
                    size_t dotPos = line.find('.');
                    size_t bracketPos = line.find('[');
                    if (dotPos != std::string::npos) {
                        size_t nameEnd = (bracketPos != std::string::npos) ? bracketPos : line.length();
                        std::string name = Platform::trim(line.substr(dotPos + 1, nameEnd - dotPos - 1));
                        
                        if (inSinks && cachedOutputDevice.empty()) {
                            cachedOutputDevice = name;
                        } else if (inSources && cachedInputDevice.empty()) {
                            cachedInputDevice = name;
                        }
                    }
                }
            }
        }
    }
    
    if ((cachedOutputDevice.empty() || cachedInputDevice.empty()) && Platform::commandExists("pactl")) {
        if (cachedOutputDevice.empty()) {
            std::string sinkName = Platform::trim(Platform::exec("pactl get-default-sink 2>/dev/null"));
            if (!sinkName.empty()) {
                std::string info = Platform::exec("pactl list sinks 2>/dev/null | grep -A5 'Name: " + sinkName + "' | grep 'Description:' | head -1");
                if (!info.empty()) {
                    size_t pos = info.find(':');
                    if (pos != std::string::npos) {
                        cachedOutputDevice = Platform::trim(info.substr(pos + 1));
                    }
                }
                if (cachedOutputDevice.empty()) {
                    cachedOutputDevice = sinkName;
                }
            }
        }
        
        if (cachedInputDevice.empty()) {
            std::string sourceName = Platform::trim(Platform::exec("pactl get-default-source 2>/dev/null"));
            if (!sourceName.empty() && sourceName.find("monitor") == std::string::npos) {
                std::string info = Platform::exec("pactl list sources 2>/dev/null | grep -A5 'Name: " + sourceName + "' | grep 'Description:' | head -1");
                if (!info.empty()) {
                    size_t pos = info.find(':');
                    if (pos != std::string::npos) {
                        cachedInputDevice = Platform::trim(info.substr(pos + 1));
                    }
                }
                if (cachedInputDevice.empty()) {
                    cachedInputDevice = sourceName;
                }
            }
        }
    }
    
    if ((cachedOutputDevice.empty() || cachedInputDevice.empty()) && Platform::commandExists("aplay")) {
        if (cachedOutputDevice.empty()) {
            std::string output = Platform::exec("aplay -l 2>/dev/null | grep 'card' | head -1");
            if (!output.empty()) {
                size_t bracket = output.find('[');
                size_t bracket2 = output.find(']');
                if (bracket != std::string::npos && bracket2 != std::string::npos && bracket2 > bracket) {
                    cachedOutputDevice = output.substr(bracket + 1, bracket2 - bracket - 1);
                }
            }
        }
        
        if (cachedInputDevice.empty()) {
            std::string output = Platform::exec("arecord -l 2>/dev/null | grep 'card' | head -1");
            if (!output.empty()) {
                size_t bracket = output.find('[');
                size_t bracket2 = output.find(']');
                if (bracket != std::string::npos && bracket2 != std::string::npos && bracket2 > bracket) {
                    cachedInputDevice = output.substr(bracket + 1, bracket2 - bracket - 1);
                }
            }
        }
    }
    
    if (cachedOutputDevice.empty()) cachedOutputDevice = "Default Audio Output";
    if (cachedInputDevice.empty()) cachedInputDevice = "Default Audio Input";
    
    devicesCached = true;
}

std::string CompactAudio::active_audio_output() {
    cacheAudioDevices();
    return shortenDeviceName(cachedOutputDevice);
}

std::string CompactAudio::active_audio_output_status() {
    cacheAudioDevices();
    return cachedOutputDevice.empty() || cachedOutputDevice == "Default Audio Output" ? "(Unknown)" : "(Active)";
}

std::string CompactAudio::active_audio_input() {
    cacheAudioDevices();
    return shortenDeviceName(cachedInputDevice);
}

std::string CompactAudio::active_audio_input_status() {
    cacheAudioDevices();
    return cachedInputDevice.empty() || cachedInputDevice == "Default Audio Input" ? "(Unknown)" : "(Active)";
}

#elif PLATFORM_FREEBSD

std::string CompactAudio::active_audio_output() {
    std::string sndstat = Platform::readFile("/dev/sndstat");
    if (!sndstat.empty()) {
        std::istringstream iss(sndstat);
        std::string line;
        while (std::getline(iss, line)) {
            if (line.find("default") != std::string::npos || 
                line.find("pcm0") != std::string::npos) {
                size_t angleStart = line.find('<');
                size_t angleEnd = line.find('>');
                if (angleStart != std::string::npos && angleEnd != std::string::npos) {
                    return line.substr(angleStart + 1, angleEnd - angleStart - 1);
                }
            }
        }
    }
    
    if (Platform::commandExists("mixer")) {
        std::string result = Platform::exec("mixer -S 2>/dev/null | head -1");
        if (!result.empty()) {
            return Platform::trim(result);
        }
    }
    
    return "Default Audio Output";
}

std::string CompactAudio::active_audio_output_status() {
    if (Platform::fileExists("/dev/dsp") || Platform::fileExists("/dev/dsp0")) {
        return "(Active)";
    }
    return "(Unknown)";
}

std::string CompactAudio::active_audio_input() {
    std::string sndstat = Platform::readFile("/dev/sndstat");
    if (!sndstat.empty()) {
        std::istringstream iss(sndstat);
        std::string line;
        while (std::getline(iss, line)) {
            if (line.find("rec") != std::string::npos ||
                line.find("input") != std::string::npos) {
                size_t angleStart = line.find('<');
                size_t angleEnd = line.find('>');
                if (angleStart != std::string::npos && angleEnd != std::string::npos) {
                    return line.substr(angleStart + 1, angleEnd - angleStart - 1);
                }
            }
        }
    }
    
    return "Default Audio Input";
}

std::string CompactAudio::active_audio_input_status() {
    if (Platform::fileExists("/dev/dsp") || Platform::fileExists("/dev/dsp0")) {
        return "(Active)";
    }
    return "(Unknown)";
}

#endif
