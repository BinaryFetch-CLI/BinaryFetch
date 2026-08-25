#include "CompactScreen.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <cstring>
#include <glob.h>

// Constructor
CompactScreen::CompactScreen() {
    refresh();
}

// Check if an NVIDIA card is active on the system
bool CompactScreen::isNvidiaPresent() {
    std::ifstream file("/sys/class/drm/card0/device/vendor");
    if (file.is_open()) {
        std::string vendor;
        if (file >> vendor) {
            return vendor.find("0x10de") != std::string::npos;
        }
    }
    return false;
}

// Check if an AMD card is active on the system
bool CompactScreen::isAMDPresent() {
    std::ifstream file("/sys/class/drm/card0/device/vendor");
    if (file.is_open()) {
        std::string vendor;
        if (file >> vendor) {
            return vendor.find("0x1002") != std::string::npos;
        }
    }
    return false;
}

// Helper to convert scale percent to string multiplier (e.g. 150 -> "1.5x")
std::string CompactScreen::scaleMultiplier(int scalePercent) {
    float mul = scalePercent / 100.0f;
    char buf[32];
    if (std::abs(mul - std::round(mul)) < 0.001f) {
        std::snprintf(buf, sizeof(buf), "%.0fx", mul);
    } else {
        std::snprintf(buf, sizeof(buf), "%.2fx", mul);
        std::string s(buf);
        size_t dot = s.find('.');
        if (dot != std::string::npos) {
            while (!s.empty() && s.back() == '0') s.pop_back();
            if (!s.empty() && s.back() == '.') s.pop_back();
            s += 'x';
            return s;
        }
    }
    return std::string(buf);
}

// Compute upscaling factor
int CompactScreen::computeUpscaleFactor(int currentWidth, int nativeWidth) {
    if (nativeWidth <= 0 || currentWidth <= 0) return 1;
    float ratio = static_cast<float>(currentWidth) / static_cast<float>(nativeWidth);
    if (ratio < 1.25f) return 1;
    return static_cast<int>(std::round(ratio));
}

// Parse EDID data to extract monitor name and native resolution
CompactScreen::EDIDInfo CompactScreen::parseEDID(const unsigned char* edid, size_t size) {
    EDIDInfo info = { "", 0, 0, false };
    if (!edid || size < 128) return info;

    // Validate EDID header
    if (edid[0] != 0x00 || edid[1] != 0xFF || edid[2] != 0xFF || edid[3] != 0xFF ||
        edid[4] != 0xFF || edid[5] != 0xFF || edid[6] != 0xFF || edid[7] != 0x00) {
        return info;
    }

    // Extract native resolution from first detailed timing descriptor (bytes 54-71)
    if (size >= 72) {
        unsigned short hActive = ((edid[58] >> 4) << 8) | edid[56];
        unsigned short vActive = ((edid[61] >> 4) << 8) | edid[59];
        if (hActive > 0 && vActive > 0) {
            info.nativeWidth = hActive;
            info.nativeHeight = vActive;
            info.valid = true;
        }
    }

    // Extract monitor name from descriptor blocks (0xFC descriptor type)
    for (int i = 54; i < 126; i += 18) {
        if (i + 17 >= size) break;
        if (edid[i] == 0x00 && edid[i + 1] == 0x00 && edid[i + 2] == 0x00 && edid[i + 3] == 0xFC) {
            std::string name;
            for (int j = 5; j < 18; ++j) {
                if (edid[i + j] == 0x0A || edid[i + j] == 0x00) break;
                if (edid[i + j] >= 0x20 && edid[i + j] <= 0x7E) {
                    name += static_cast<char>(edid[i + j]);
                }
            }
            while (!name.empty() && name.back() == ' ') name.pop_back();
            if (!name.empty()) {
                info.friendlyName = name;
            }
            break;
        }
    }
    return info;
}

// Refresh connector lists and query information
bool CompactScreen::refresh() {
    screens.clear();
    
    glob_t g;
    // Look for all connectors (e.g. card0-HDMI-A-1, card0-DP-1)
    if (glob("/sys/class/drm/card*-*", 0, nullptr, &g) == 0) {
        for (size_t i = 0; i < g.gl_pathc; ++i) {
            std::string connPath = g.gl_pathv[i];
            
            // Check if connected
            std::ifstream statusFile(connPath + "/status");
            std::string status;
            if (statusFile >> status && status == "connected") {
                
                // Get display name (connector type and ID)
                size_t lastSlash = connPath.find_last_of('/');
                std::string connName = (lastSlash != std::string::npos) ? connPath.substr(lastSlash + 1) : connPath;
                
                // Read current mode / resolution
                int currentW = 0, currentH = 0, refreshRate = 60;
                std::ifstream modesFile(connPath + "/modes");
                std::string modeLine;
                if (modesFile && std::getline(modesFile, modeLine)) {
                    // modeLine is e.g. "1920x1080"
                    size_t xPos = modeLine.find('x');
                    if (xPos != std::string::npos) {
                        currentW = std::stoi(modeLine.substr(0, xPos));
                        currentH = std::stoi(modeLine.substr(xPos + 1));
                    }
                }

                // Try reading EDID
                int nativeW = currentW;
                int nativeH = currentH;
                std::string friendlyName = connName;
                
                std::ifstream edidFile(connPath + "/edid", std::ios::binary);
                if (edidFile) {
                    unsigned char edidData[1024];
                    edidFile.read(reinterpret_cast<char*>(edidData), sizeof(edidData));
                    size_t bytesRead = edidFile.gcount();
                    
                    EDIDInfo edidInfo = parseEDID(edidData, bytesRead);
                    if (!edidInfo.friendlyName.empty()) {
                        friendlyName = edidInfo.friendlyName;
                    }
                    if (edidInfo.valid) {
                        nativeW = edidInfo.nativeWidth;
                        nativeH = edidInfo.nativeHeight;
                    }
                }

                // Scaling factor
                int scalePercent = 100;
                // Simple environment check
                char* scaleEnv = std::getenv("GDK_SCALE");
                if (scaleEnv) {
                    try {
                        scalePercent = std::stoi(scaleEnv) * 100;
                    } catch (...) {}
                }

                int upscaleFactor = computeUpscaleFactor(currentW, nativeW);
                std::string upscaleStr = "1x";
                if (upscaleFactor > 1) {
                    upscaleStr = std::to_string(upscaleFactor) + "x";
                }

                ScreenInfo sinfo;
                sinfo.name = friendlyName;
                sinfo.native_width = nativeW;
                sinfo.native_height = nativeH;
                sinfo.current_width = currentW;
                sinfo.current_height = currentH;
                sinfo.refresh_rate = refreshRate;
                sinfo.scale_percent = scalePercent;
                sinfo.scale_mul = scaleMultiplier(scalePercent);
                sinfo.upscale = upscaleStr;

                screens.push_back(sinfo);
            }
        }
    }
    globfree(&g);
    return !screens.empty();
}

// Populate methods (stubs required by compiler or Windows declarations, not used on Linux)
bool CompactScreen::populateFromDXGI() { return false; }
bool CompactScreen::enrichWithNVAPI() { return false; }
bool CompactScreen::enrichWithADL() { return false; }
std::string CompactScreen::getFriendlyNameFromEDID(const std::wstring&) { return ""; }
