#include "../Platform.h"
#include "../../DisplayInfo.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <dirent.h>

std::string DisplayInfo::WideToUtf8(const wchar_t*) {
    return "";
}

std::string DisplayInfo::scaleMultiplier(int scalePercent) {
    float mul = scalePercent / 100.0f;
    char buf[32];
    if (fabsf(mul - roundf(mul)) < 0.001f) {
        snprintf(buf, sizeof(buf), "%.0fx", mul);
    } else {
        snprintf(buf, sizeof(buf), "%.2fx", mul);
    }
    return std::string(buf);
}

int DisplayInfo::computeUpscaleFactor(int currentWidth, int nativeWidth) {
    if (nativeWidth <= 0 || currentWidth <= 0) return 1;
    float ratio = static_cast<float>(currentWidth) / static_cast<float>(nativeWidth);
    if (ratio < 1.25f) return 1;
    return static_cast<int>(std::round(ratio));
}

std::string DisplayInfo::computeAspectRatio(int w, int h) {
    if (w <= 0 || h <= 0) return "Unknown";
    int a = w, b = h;
    while (b != 0) { int t = b; b = a % b; a = t; }
    return std::to_string(w/a) + ":" + std::to_string(h/a);
}

bool DisplayInfo::isNvidiaPresent() {
    return Platform::fileExists("/sys/module/nvidia/version");
}

bool DisplayInfo::isAMDPresent() {
    return Platform::fileExists("/sys/module/amdgpu/version");
}

DisplayInfo::EDIDInfo DisplayInfo::parseEDID(const unsigned char* edid, size_t size) {
    EDIDInfo info = {"", 0, 0, false};
    if (!edid || size < 128) return info;
    if (edid[0] != 0x00 || edid[1] != 0xFF || edid[7] != 0x00) return info;
    
    if (size >= 72) {
        unsigned short hActive = ((edid[58] >> 4) << 8) | edid[56];
        unsigned short vActive = ((edid[61] >> 4) << 8) | edid[59];
        if (hActive > 0 && vActive > 0) {
            info.nativeWidth = hActive;
            info.nativeHeight = vActive;
            info.valid = true;
        }
    }
    
    for (int i = 54; i < 126; i += 18) {
        if (i + 17 >= static_cast<int>(size)) break;
        if (edid[i] == 0x00 && edid[i + 1] == 0x00 && edid[i + 3] == 0xFC) {
            std::string name;
            for (int j = 5; j < 18; ++j) {
                if (edid[i + j] == 0x0A || edid[i + j] == 0x00) break;
                if (edid[i + j] >= 0x20 && edid[i + j] <= 0x7E) {
                    name += static_cast<char>(edid[i + j]);
                }
            }
            while (!name.empty() && name.back() == ' ') name.pop_back();
            if (!name.empty()) info.friendlyName = name;
            break;
        }
    }
    return info;
}

std::string DisplayInfo::getFriendlyNameFromEDID(const std::wstring&) {
    DIR* dir = opendir("/sys/class/drm");
    if (!dir) return "Generic Monitor";
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name.find("card") == 0 && name.find("-") != std::string::npos) {
            std::string edidPath = "/sys/class/drm/" + name + "/edid";
            std::ifstream file(edidPath, std::ios::binary);
            if (file.is_open()) {
                std::vector<unsigned char> data((std::istreambuf_iterator<char>(file)),
                                                  std::istreambuf_iterator<char>());
                file.close();
                
                if (data.size() >= 128) {
                    EDIDInfo info = parseEDID(data.data(), data.size());
                    if (!info.friendlyName.empty()) {
                        closedir(dir);
                        return info.friendlyName;
                    }
                }
            }
        }
    }
    closedir(dir);
    return "Generic Monitor";
}

DisplayInfo::DisplayInfo() {
    refresh();
}

bool DisplayInfo::refresh() {
    screens.clear();
    return populateFromDXGI();
}

const std::vector<DisplayInfo::ScreenInfo>& DisplayInfo::getScreens() const {
    return screens;
}

bool DisplayInfo::populateFromDXGI() {
    std::string displayEnv = Platform::getEnv("DISPLAY");
    std::string waylandEnv = Platform::getEnv("WAYLAND_DISPLAY");
    
    if (displayEnv.empty() && waylandEnv.empty()) {
        return true;
    }
    
    std::string xrandr = Platform::exec("xrandr --current 2>/dev/null");
    if (xrandr.empty()) return true;
    
    std::istringstream iss(xrandr);
    std::string line;
    ScreenInfo current;
    bool inMonitor = false;
    
    while (std::getline(iss, line)) {
        if (line.find(" connected") != std::string::npos) {
            if (inMonitor && current.current_width > 0) {
                screens.push_back(current);
            }
            
            current = ScreenInfo();
            inMonitor = true;
            
            size_t spacePos = line.find(' ');
            if (spacePos != std::string::npos) {
                current.name = line.substr(0, spacePos);
            }
            
            size_t resStart = line.find_first_of("0123456789");
            if (resStart != std::string::npos) {
                int w = 0, h = 0, offX = 0, offY = 0;
                if (sscanf(line.c_str() + resStart, "%dx%d+%d+%d", &w, &h, &offX, &offY) >= 2) {
                    current.current_width = w;
                    current.current_height = h;
                    current.native_width = w;
                    current.native_height = h;
                    current.native_resolution = std::to_string(w) + "x" + std::to_string(h);
                    current.aspect_ratio = computeAspectRatio(w, h);
                }
            }
        }
        else if (inMonitor && line.find("*") != std::string::npos) {
            size_t hzStart = line.find_last_of("0123456789.");
            if (hzStart != std::string::npos) {
                size_t hzEnd = line.find("*");
                if (hzEnd != std::string::npos) {
                    while (hzStart > 0 && (std::isdigit(line[hzStart-1]) || line[hzStart-1] == '.')) {
                        hzStart--;
                    }
                    std::string hzStr = line.substr(hzStart, hzEnd - hzStart);
                    try {
                        current.refresh_rate = static_cast<int>(std::stof(Platform::trim(hzStr)));
                    } catch (...) {
                        current.refresh_rate = 60;
                    }
                }
            }
        }
    }
    
    if (inMonitor && current.current_width > 0) {
        screens.push_back(current);
    }
    
    std::string dpiStr = Platform::exec("xrdb -query 2>/dev/null | grep -i dpi | head -1 | awk '{print $2}'");
    int dpi = 96;
    if (!dpiStr.empty()) {
        try { dpi = std::stoi(Platform::trim(dpiStr)); }
        catch (...) { dpi = 96; }
    }
    
    for (auto& screen : screens) {
        screen.scale_percent = static_cast<int>(std::round((dpi / 96.0f) * 100.0f));
        screen.scale_mul = scaleMultiplier(screen.scale_percent);
        screen.upscale = "1x";
        screen.dsr_enabled = false;
        screen.dsr_type = "None";
    }
    
    return !screens.empty();
}

bool DisplayInfo::enrichWithNVAPI() { return true; }
bool DisplayInfo::enrichWithADL() { return true; }
