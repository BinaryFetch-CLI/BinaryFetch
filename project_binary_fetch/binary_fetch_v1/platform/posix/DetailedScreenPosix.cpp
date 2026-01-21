#include "../Platform.h"
#include "../../DetailedScreen.h"
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <cmath>
#include <cstdio>

using namespace std;
using namespace Platform;

DetailedScreen::DetailedScreen() {
    refresh();
}

string DetailedScreen::scaleMultiplier(int scalePercent) {
    if (scalePercent <= 100) return "1x";
    if (scalePercent <= 125) return "1.25x";
    if (scalePercent <= 150) return "1.5x";
    if (scalePercent <= 175) return "1.75x";
    if (scalePercent <= 200) return "2x";
    if (scalePercent <= 250) return "2.5x";
    if (scalePercent <= 300) return "3x";
    return to_string(scalePercent / 100) + "x";
}

int DetailedScreen::computeUpscaleFactor(int currentWidth, int nativeWidth) {
    if (nativeWidth <= 0) return 1;
    return (currentWidth + nativeWidth - 1) / nativeWidth;
}

float DetailedScreen::calculatePPI(int width, int height, float diagonalInches) {
    if (diagonalInches <= 0) return 0.0f;
    float diagonalPixels = sqrt(static_cast<float>(width * width + height * height));
    return diagonalPixels / diagonalInches;
}

float DetailedScreen::calculateDiagonal(float widthMM, float heightMM) {
    return sqrt(widthMM * widthMM + heightMM * heightMM);
}

float DetailedScreen::calculateScreenSizeInches(float widthMM, float heightMM) {
    float diagonalMM = calculateDiagonal(widthMM, heightMM);
    return diagonalMM / 25.4f;
}

bool DetailedScreen::isNvidiaPresent() {
    return commandExists("nvidia-smi") && !exec("nvidia-smi -L 2>/dev/null").empty();
}

bool DetailedScreen::isAMDPresent() {
    string output = exec("lspci 2>/dev/null | grep -i 'AMD\\|ATI\\|Radeon' 2>/dev/null");
    return !output.empty();
}

string DetailedScreen::getGPUVendor() {
    if (isNvidiaPresent()) return "NVIDIA";
    if (isAMDPresent()) return "AMD";
    
    string output = exec("lspci 2>/dev/null | grep -i 'Intel.*Graphics' 2>/dev/null");
    if (!output.empty()) return "Intel";
    
    return "Unknown";
}

bool DetailedScreen::populateFromXrandr() {
    if (!commandExists("xrandr")) {
        return false;
    }
    
    string output = exec("xrandr --query 2>/dev/null");
    if (output.empty()) return false;
    
    istringstream iss(output);
    string line;
    DetailedScreenInfo currentScreen;
    bool inScreen = false;
    
    while (getline(iss, line)) {
        if (line.find(" connected") != string::npos) {
            if (inScreen && !currentScreen.name.empty()) {
                screens.push_back(currentScreen);
            }
            
            currentScreen = DetailedScreenInfo();
            inScreen = true;
            
            size_t spacePos = line.find(' ');
            if (spacePos != string::npos) {
                currentScreen.deviceName = line.substr(0, spacePos);
                currentScreen.name = currentScreen.deviceName;
            }
            
            currentScreen.isPrimary = (line.find("primary") != string::npos);
            
            size_t resStart = string::npos;
            for (size_t i = 0; i < line.size(); i++) {
                if (isdigit(line[i]) && (i == 0 || !isdigit(line[i-1]))) {
                    size_t xPos = line.find('x', i);
                    if (xPos != string::npos && xPos < i + 6) {
                        resStart = i;
                        break;
                    }
                }
            }
            
            if (resStart != string::npos) {
                size_t xPos = line.find('x', resStart);
                size_t plusPos = line.find('+', resStart);
                
                if (xPos != string::npos) {
                    try {
                        currentScreen.current_width = stoi(line.substr(resStart, xPos - resStart));
                        
                        size_t heightEnd = (plusPos != string::npos) ? plusPos : line.find(' ', xPos);
                        if (heightEnd != string::npos) {
                            currentScreen.current_height = stoi(line.substr(xPos + 1, heightEnd - xPos - 1));
                        }
                    } catch (...) {}
                }
                
                if (plusPos != string::npos) {
                    size_t secondPlus = line.find('+', plusPos + 1);
                    if (secondPlus != string::npos) {
                        try {
                            currentScreen.pos_x = stoi(line.substr(plusPos + 1, secondPlus - plusPos - 1));
                            size_t endPos = line.find(' ', secondPlus);
                            currentScreen.pos_y = stoi(line.substr(secondPlus + 1, endPos - secondPlus - 1));
                        } catch (...) {}
                    }
                }
            }
            
            size_t mmPos = line.find("mm x ");
            if (mmPos != string::npos) {
                size_t widthStart = mmPos;
                while (widthStart > 0 && (isdigit(line[widthStart-1]) || line[widthStart-1] == ' ')) {
                    widthStart--;
                }
                
                try {
                    currentScreen.width_mm = stof(line.substr(widthStart, mmPos - widthStart));
                    
                    size_t heightStart = mmPos + 5;
                    size_t heightEnd = line.find("mm", heightStart);
                    if (heightEnd != string::npos) {
                        currentScreen.height_mm = stof(line.substr(heightStart, heightEnd - heightStart));
                    }
                    
                    currentScreen.diagonal_inches = calculateScreenSizeInches(currentScreen.width_mm, currentScreen.height_mm);
                    currentScreen.ppi = calculatePPI(currentScreen.current_width, currentScreen.current_height, currentScreen.diagonal_inches);
                } catch (...) {}
            }
            
        } else if (inScreen && line.find("*") != string::npos && line.find("x") != string::npos) {
            size_t starPos = line.find('*');
            if (starPos != string::npos && starPos > 0) {
                size_t rateStart = starPos - 1;
                while (rateStart > 0 && (isdigit(line[rateStart-1]) || line[rateStart-1] == '.')) {
                    rateStart--;
                }
                
                try {
                    float refreshFloat = stof(line.substr(rateStart, starPos - rateStart));
                    currentScreen.refresh_rate = static_cast<int>(round(refreshFloat));
                } catch (...) {
                    currentScreen.refresh_rate = 60;
                }
            }
            
            if (currentScreen.native_width == 0) {
                size_t xPos = line.find('x');
                if (xPos != string::npos) {
                    size_t resStart = xPos;
                    while (resStart > 0 && isdigit(line[resStart-1])) {
                        resStart--;
                    }
                    
                    try {
                        currentScreen.native_width = stoi(line.substr(resStart, xPos - resStart));
                        
                        size_t heightEnd = xPos + 1;
                        while (heightEnd < line.size() && isdigit(line[heightEnd])) {
                            heightEnd++;
                        }
                        currentScreen.native_height = stoi(line.substr(xPos + 1, heightEnd - xPos - 1));
                    } catch (...) {}
                }
            }
        }
    }
    
    if (inScreen && !currentScreen.name.empty()) {
        screens.push_back(currentScreen);
    }
    
    return !screens.empty();
}

bool DetailedScreen::populateFromDRM() {
    const string drmPath = "/sys/class/drm/";
    
    DIR* dir = opendir(drmPath.c_str());
    if (!dir) return false;
    
    struct dirent* entry;
    
    while ((entry = readdir(dir)) != nullptr) {
        string name = entry->d_name;
        
        if (name.find("card") != 0 || name.find('-') == string::npos) continue;
        
        string connectorPath = drmPath + name + "/";
        
        string status = trim(readFile(connectorPath + "status"));
        if (status != "connected") continue;
        
        DetailedScreenInfo screen;
        screen.deviceName = name;
        screen.name = name;
        
        string modes = readFile(connectorPath + "modes");
        if (!modes.empty()) {
            istringstream iss(modes);
            string mode;
            if (getline(iss, mode)) {
                size_t xPos = mode.find('x');
                if (xPos != string::npos) {
                    try {
                        screen.native_width = stoi(mode.substr(0, xPos));
                        screen.native_height = stoi(mode.substr(xPos + 1));
                        screen.current_width = screen.native_width;
                        screen.current_height = screen.native_height;
                    } catch (...) {}
                }
            }
        }
        
        screens.push_back(screen);
    }
    
    closedir(dir);
    return !screens.empty();
}

bool DetailedScreen::refresh() {
    screens.clear();
    
    if (populateFromXrandr()) {
        return true;
    }
    
    if (populateFromDRM()) {
        return true;
    }
    
    DetailedScreenInfo placeholder;
    placeholder.name = "Unknown Display";
    placeholder.deviceName = "Unknown";
    placeholder.isPrimary = true;
    placeholder.current_width = 1920;
    placeholder.current_height = 1080;
    placeholder.native_width = 1920;
    placeholder.native_height = 1080;
    placeholder.refresh_rate = 60;
    placeholder.scale_percent = 100;
    screens.push_back(placeholder);
    
    return true;
}
