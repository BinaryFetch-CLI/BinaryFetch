#include "../../CompactScreen.h"
#include "../Platform.h"
#include <sstream>
#include <fstream>
#include <cstring>
#include <dirent.h>
#include <cmath>

#if PLATFORM_POSIX

CompactScreen::CompactScreen() {
    refresh();
}

bool CompactScreen::refresh() {
    screens.clear();
    return populateFromXrandr() || populateFromDRM();
}

bool CompactScreen::isNvidiaPresent() {
    return Platform::commandExists("nvidia-smi");
}

bool CompactScreen::isAMDPresent() {
    std::string vendor = Platform::readFileLine("/sys/class/drm/card0/device/vendor");
    return vendor.find("1002") != std::string::npos;
}

std::string CompactScreen::scaleMultiplier(int scalePercent) {
    if (scalePercent <= 100) return "1x";
    if (scalePercent <= 125) return "1.25x";
    if (scalePercent <= 150) return "1.5x";
    if (scalePercent <= 175) return "1.75x";
    if (scalePercent <= 200) return "2x";
    return std::to_string(scalePercent / 100) + "x";
}

int CompactScreen::computeUpscaleFactor(int currentWidth, int nativeWidth) {
    if (nativeWidth <= 0 || currentWidth <= nativeWidth) return 1;
    return currentWidth / nativeWidth;
}

bool CompactScreen::populateFromXrandr() {
    if (!Platform::commandExists("xrandr")) return false;
    
    std::string output = Platform::exec("xrandr --query 2>/dev/null");
    if (output.empty()) return false;
    
    std::istringstream iss(output);
    std::string line;
    ScreenInfo current;
    bool hasScreen = false;
    
    while (std::getline(iss, line)) {
        if (line.find(" connected") != std::string::npos) {
            if (hasScreen && current.current_width > 0) {
                screens.push_back(current);
            }
            current = ScreenInfo();
            hasScreen = true;
            
            size_t pos = line.find(" connected");
            if (pos != std::string::npos) {
                current.name = Platform::trim(line.substr(0, pos));
            }
            
            int w = 0, h = 0, offX = 0, offY = 0;
            size_t resStart = line.find(" connected");
            if (resStart != std::string::npos) {
                std::string rest = line.substr(resStart + 10);
                if (rest.find("primary") == 0) {
                    rest = rest.substr(7);
                }
                rest = Platform::trim(rest);
                
                if (sscanf(rest.c_str(), "%dx%d+%d+%d", &w, &h, &offX, &offY) >= 2) {
                    current.current_width = w;
                    current.current_height = h;
                    current.native_width = w;
                    current.native_height = h;
                }
            }
            
            current.scale_percent = 100;
            current.scale_mul = "1x";
            current.upscale = "Off";
            current.refresh_rate = 60;
        }
        else if (hasScreen && line.find("*") != std::string::npos) {
            std::string trimmed = Platform::trim(line);
            size_t starPos = trimmed.find('*');
            if (starPos != std::string::npos && starPos > 0) {
                size_t hzEnd = starPos;
                size_t hzStart = hzEnd;
                while (hzStart > 0 && (std::isdigit(trimmed[hzStart-1]) || trimmed[hzStart-1] == '.')) {
                    hzStart--;
                }
                
                if (hzStart < hzEnd) {
                    std::string hzStr = trimmed.substr(hzStart, hzEnd - hzStart);
                    try {
                        current.refresh_rate = static_cast<int>(std::round(std::stof(hzStr)));
                    } catch (...) {
                        current.refresh_rate = 60;
                    }
                }
            }
        }
    }
    
    if (hasScreen && current.current_width > 0) {
        screens.push_back(current);
    }
    
    return !screens.empty();
}

bool CompactScreen::populateFromDRM() {
    DIR* dir = opendir("/sys/class/drm");
    if (!dir) return false;
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name.find("card") == 0 && name.find("-") != std::string::npos) {
            std::string statusPath = "/sys/class/drm/" + name + "/status";
            std::string status = Platform::readFileLine(statusPath);
            if (Platform::trim(status) == "connected") {
                ScreenInfo info;
                info.name = name;
                
                std::string modesPath = "/sys/class/drm/" + name + "/modes";
                std::string modes = Platform::readFileLine(modesPath);
                if (!modes.empty()) {
                    int w = 0, h = 0;
                    if (sscanf(modes.c_str(), "%dx%d", &w, &h) == 2) {
                        info.current_width = w;
                        info.current_height = h;
                        info.native_width = w;
                        info.native_height = h;
                    }
                }
                
                info.refresh_rate = 60;
                info.scale_percent = 100;
                info.scale_mul = "1x";
                info.upscale = "Off";
                
                screens.push_back(info);
            }
        }
    }
    closedir(dir);
    
    return !screens.empty();
}

CompactScreen::EDIDInfo CompactScreen::parseEDID(const unsigned char* edid, size_t size) {
    EDIDInfo info = {"Unknown", 0, 0, false};
    if (!edid || size < 128) return info;
    if (edid[0] != 0x00 || edid[1] != 0xFF || edid[2] != 0xFF) return info;
    
    for (int i = 54; i <= 108; i += 18) {
        if (edid[i] == 0 && edid[i+1] == 0 && edid[i+3] == 0xFC) {
            char name[14] = {0};
            memcpy(name, &edid[i+5], 13);
            for (int j = 0; j < 13; j++) {
                if (name[j] == '\n' || name[j] == '\r') name[j] = '\0';
            }
            info.friendlyName = Platform::trim(std::string(name));
            break;
        }
    }
    
    if (edid[56] && edid[59]) {
        info.nativeWidth = ((edid[58] & 0xF0) << 4) | edid[56];
        info.nativeHeight = ((edid[61] & 0xF0) << 4) | edid[59];
    }
    
    info.valid = true;
    return info;
}

#endif
