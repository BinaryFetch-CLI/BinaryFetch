#include "CompactGPU.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <glob.h>

// Helper to read a single line from a file
static std::string readLine(const std::string& path) {
    std::ifstream file(path);
    std::string line;
    if (file && std::getline(file, line)) {
        return line;
    }
    return "";
}

// Get the GPU manufacturer and device code/name
std::string CompactGPU::getGPUName() {
    std::string vendor = readLine("/sys/class/drm/card0/device/vendor");
    
    if (!vendor.empty()) {
        if (vendor.find("0x10de") != std::string::npos) return "NVIDIA Corporation GPU";
        if (vendor.find("0x1002") != std::string::npos) return "Advanced Micro Devices, Inc. [AMD/ATI]";
        if (vendor.find("0x8086") != std::string::npos) return "Intel Corporation Graphics";
    }

    // Try reading device name from uevent
    std::ifstream file("/sys/class/drm/card0/device/uevent");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.find("PCI_ID=") == 0) {
                return "PCI GPU (" + line.substr(7) + ")";
            }
        }
    }
    
    return "Unknown GPU";
}

// Get GPU Dedicated VRAM in GB
double CompactGPU::getVRAMGB() {
    std::string vramPath = "/sys/class/drm/card0/device/mem_info_vram_total";
    std::ifstream file(vramPath);
    if (file.is_open()) {
        double bytes = 0;
        if (file >> bytes) {
            return bytes / (1024.0 * 1024.0 * 1024.0);
        }
    }
    return 0.0;
}

// Get active GPU Usage Percent
int CompactGPU::getGPUUsagePercent() {
    std::ifstream file("/sys/class/drm/card0/device/gpu_busy_percent");
    if (file.is_open()) {
        int usage = 0;
        if (file >> usage) return usage;
    }
    return -1;
}

// Get GPU Frequency description
std::string CompactGPU::getGPUFrequency() {
    // Check AMD frequency
    std::ifstream file("/sys/class/drm/card0/device/pp_dpm_sclk");
    if (file.is_open()) {
        std::string line, lastNonEmpty;
        while (std::getline(file, line)) {
            if (line.find('*') != std::string::npos) {
                return line; // e.g. "3: 1200Mhz *"
            }
            if (!line.empty()) lastNonEmpty = line;
        }
        if (!lastNonEmpty.empty()) return lastNonEmpty;
    }

    // Check Intel frequency
    std::ifstream intelFile("/sys/class/drm/card0/device/gt_cur_freq_mhz");
    if (intelFile.is_open()) {
        int freq = 0;
        if (intelFile >> freq) {
            return std::to_string(freq) + " MHz";
        }
    }

    return "Unknown MHz";
}

// Get GPU Temperature in Celsius
double CompactGPU::getGPUTemperature() {
    // Traverse standard hwmon directories under GPU device
    glob_t g;
    if (glob("/sys/class/drm/card0/device/hwmon/hwmon*/temp1_input", 0, nullptr, &g) == 0) {
        if (g.gl_pathc > 0) {
            std::ifstream file(g.gl_pathv[0]);
            double millidegrees = 0;
            if (file >> millidegrees) {
                globfree(&g);
                return millidegrees / 1000.0;
            }
        }
        globfree(&g);
    }
    return 0.0;
}
