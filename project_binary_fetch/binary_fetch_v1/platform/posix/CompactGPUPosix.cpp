#include "../../CompactGPU.h"
#include "../Platform.h"
#include <sstream>
#include <iomanip>
#include <fstream>
#include <dirent.h>
#include <map>

#if PLATFORM_POSIX

static std::map<std::string, std::string> pciIdToName = {
    // AMD Radeon RX 500 series
    {"1002:67df", "AMD Radeon RX 580"},
    {"1002:67ef", "AMD Radeon RX 560"},
    {"1002:67ff", "AMD Radeon RX 560X"},
    {"1002:699f", "AMD Radeon RX 550"},
    // AMD Radeon RX 5000 series (RDNA)
    {"1002:731f", "AMD Radeon RX 5700 XT"},
    {"1002:7340", "AMD Radeon RX 5700"},
    {"1002:7341", "AMD Radeon RX 5600 XT"},
    // AMD Radeon RX 6000 series (RDNA 2)
    {"1002:73bf", "AMD Radeon RX 6900 XT"},
    {"1002:73af", "AMD Radeon RX 6800 XT"},
    {"1002:73a5", "AMD Radeon RX 6800"},
    {"1002:73df", "AMD Radeon RX 6700 XT"},
    {"1002:73ff", "AMD Radeon RX 6600 XT"},
    {"1002:73e3", "AMD Radeon RX 6600"},
    // AMD Radeon RX 7000 series (RDNA 3)
    {"1002:744c", "AMD Radeon RX 7900 XTX"},
    {"1002:7448", "AMD Radeon RX 7900 XT"},
    {"1002:7480", "AMD Radeon RX 7600"},
    {"1002:7483", "AMD Radeon RX 7600 XT"},
    // NVIDIA RTX 40 series
    {"10de:2684", "NVIDIA GeForce RTX 4090"},
    {"10de:2702", "NVIDIA GeForce RTX 4080 SUPER"},
    {"10de:2704", "NVIDIA GeForce RTX 4080"},
    {"10de:2782", "NVIDIA GeForce RTX 4070 Ti SUPER"},
    {"10de:2783", "NVIDIA GeForce RTX 4070 Ti"},
    {"10de:2786", "NVIDIA GeForce RTX 4070 SUPER"},
    {"10de:2788", "NVIDIA GeForce RTX 4070"},
    {"10de:27a0", "NVIDIA GeForce RTX 4060 Ti"},
    {"10de:27b0", "NVIDIA GeForce RTX 4060"},
    // NVIDIA RTX 30 series
    {"10de:2204", "NVIDIA GeForce RTX 3090"},
    {"10de:2203", "NVIDIA GeForce RTX 3090 Ti"},
    {"10de:2206", "NVIDIA GeForce RTX 3080"},
    {"10de:2208", "NVIDIA GeForce RTX 3080 Ti"},
    {"10de:2216", "NVIDIA GeForce RTX 3070"},
    {"10de:2414", "NVIDIA GeForce RTX 3070 Ti"},
    {"10de:2484", "NVIDIA GeForce RTX 3060"},
    {"10de:2486", "NVIDIA GeForce RTX 3060 Ti"},
    // NVIDIA RTX 20 series
    {"10de:1e04", "NVIDIA GeForce RTX 2080 Ti"},
    {"10de:1e07", "NVIDIA GeForce RTX 2080 SUPER"},
    {"10de:1e82", "NVIDIA GeForce RTX 2080"},
    {"10de:1f07", "NVIDIA GeForce RTX 2070 SUPER"},
    {"10de:1f02", "NVIDIA GeForce RTX 2070"},
    {"10de:1f08", "NVIDIA GeForce RTX 2060 SUPER"},
    {"10de:1f47", "NVIDIA GeForce RTX 2060"},
    // NVIDIA GTX 16 series
    {"10de:2182", "NVIDIA GeForce GTX 1660 Ti"},
    {"10de:2184", "NVIDIA GeForce GTX 1660 SUPER"},
    {"10de:2187", "NVIDIA GeForce GTX 1650 SUPER"},
    {"10de:1f82", "NVIDIA GeForce GTX 1650"},
    // Intel Arc
    {"8086:56a0", "Intel Arc A770"},
    {"8086:56a1", "Intel Arc A750"},
    {"8086:56a5", "Intel Arc A580"},
    {"8086:5690", "Intel Arc A380"},
    // Intel Integrated
    {"8086:9a49", "Intel Iris Xe Graphics"},
    {"8086:a7a0", "Intel Raptor Lake-P GT2"},
    {"8086:46a6", "Intel Alder Lake-P GT2"},
};

static std::string getVendorName(const std::string& vendorId) {
    if (vendorId == "1002" || vendorId == "0x1002") return "AMD";
    if (vendorId == "10de" || vendorId == "0x10de") return "NVIDIA";
    if (vendorId == "8086" || vendorId == "0x8086") return "Intel";
    return "Unknown";
}

static std::string getGPUNameFromSysfs() {
    DIR* dir = opendir("/sys/class/drm");
    if (!dir) return "";
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name.find("card") == 0 && name.find("-") == std::string::npos) {
            std::string devicePath = "/sys/class/drm/" + name + "/device/";
            std::string uevent = Platform::readFile(devicePath + "uevent");
            
            std::string pciId;
            std::istringstream iss(uevent);
            std::string line;
            while (std::getline(iss, line)) {
                if (line.find("PCI_ID=") == 0) {
                    pciId = line.substr(7);
                    for (char& c : pciId) c = std::tolower(c);
                    break;
                }
            }
            
            if (!pciId.empty()) {
                closedir(dir);
                
                auto it = pciIdToName.find(pciId);
                if (it != pciIdToName.end()) {
                    return it->second;
                }
                
                std::string vendor = getVendorName(pciId.substr(0, 4));
                return vendor + " GPU (" + pciId + ")";
            }
        }
    }
    closedir(dir);
    return "";
}

static std::string getGPUNameFromLspci() {
    std::string output = Platform::exec("lspci 2>/dev/null");
    if (output.empty()) return "";
    
    std::istringstream iss(output);
    std::string line;
    
    while (std::getline(iss, line)) {
        bool isVGA = line.find("0300") != std::string::npos || 
                     line.find("0302") != std::string::npos ||
                     line.find("0380") != std::string::npos ||
                     line.find("VGA") != std::string::npos ||
                     line.find("3D") != std::string::npos ||
                     line.find("Display") != std::string::npos;
        
        if (isVGA) {
            size_t colonPos = line.rfind(':');
            if (colonPos != std::string::npos && colonPos > 5) {
                std::string pciId = Platform::trim(line.substr(colonPos - 9, 9));
                for (char& c : pciId) c = std::tolower(c);
                
                auto it = pciIdToName.find(pciId);
                if (it != pciIdToName.end()) {
                    return it->second;
                }
                
                std::string vendor = getVendorName(pciId.substr(0, 4));
                return vendor + " GPU";
            }
        }
    }
    
    return "";
}

std::string CompactGPU::getGPUName() {
    if (Platform::commandExists("nvidia-smi")) {
        std::string result = Platform::exec("nvidia-smi --query-gpu=name --format=csv,noheader 2>/dev/null | head -1");
        result = Platform::trim(result);
        if (!result.empty()) return result;
    }
    
    std::string name = getGPUNameFromSysfs();
    if (!name.empty()) return name;
    
    name = getGPUNameFromLspci();
    if (!name.empty()) return name;
    
    return "Unknown GPU";
}

int CompactGPU::getGPUUsagePercent() {
    if (Platform::commandExists("nvidia-smi")) {
        std::string result = Platform::exec("nvidia-smi --query-gpu=utilization.gpu --format=csv,noheader,nounits 2>/dev/null | head -1");
        result = Platform::trim(result);
        if (!result.empty()) {
            try { return std::stoi(result); }
            catch (...) {}
        }
    }
    
    DIR* dir = opendir("/sys/class/drm");
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name.find("card") == 0 && name.find("-") == std::string::npos) {
                std::string busyPath = "/sys/class/drm/" + name + "/device/gpu_busy_percent";
                std::string busy = Platform::readFileLine(busyPath);
                if (!busy.empty()) {
                    closedir(dir);
                    try { return std::stoi(Platform::trim(busy)); }
                    catch (...) {}
                }
            }
        }
        closedir(dir);
    }
    
    return 0;
}

double CompactGPU::getVRAMGB() {
    if (Platform::commandExists("nvidia-smi")) {
        std::string result = Platform::exec("nvidia-smi --query-gpu=memory.total --format=csv,noheader,nounits 2>/dev/null | head -1");
        result = Platform::trim(result);
        if (!result.empty()) {
            try {
                float memMB = std::stof(result);
                return memMB / 1024.0;
            } catch (...) {}
        }
    }
    
    DIR* dir = opendir("/sys/class/drm");
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name.find("card") == 0 && name.find("-") == std::string::npos) {
                std::string memPath = "/sys/class/drm/" + name + "/device/mem_info_vram_total";
                std::string memInfo = Platform::readFileLine(memPath);
                if (!memInfo.empty()) {
                    closedir(dir);
                    try {
                        unsigned long long bytes = std::stoull(memInfo);
                        return bytes / (1024.0 * 1024.0 * 1024.0);
                    } catch (...) {}
                }
            }
        }
        closedir(dir);
    }
    
    return 0.0;
}

std::string CompactGPU::getGPUFrequency() {
    if (Platform::commandExists("nvidia-smi")) {
        std::string result = Platform::exec("nvidia-smi --query-gpu=clocks.gr --format=csv,noheader,nounits 2>/dev/null | head -1");
        result = Platform::trim(result);
        if (!result.empty()) {
            return result + " MHz";
        }
    }
    
    DIR* dir = opendir("/sys/class/drm");
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name.find("card") == 0 && name.find("-") == std::string::npos) {
                std::string freqPath = "/sys/class/drm/" + name + "/device/pp_dpm_sclk";
                std::string freq = Platform::readFile(freqPath);
                if (!freq.empty()) {
                    std::istringstream iss(freq);
                    std::string line;
                    while (std::getline(iss, line)) {
                        if (line.find('*') != std::string::npos) {
                            size_t colonPos = line.find(':');
                            size_t mhzPos = line.find("Mhz");
                            if (mhzPos == std::string::npos) mhzPos = line.find("MHz");
                            if (colonPos != std::string::npos && mhzPos != std::string::npos) {
                                std::string clockStr = Platform::trim(line.substr(colonPos + 1, mhzPos - colonPos - 1));
                                closedir(dir);
                                return clockStr + " MHz";
                            }
                        }
                    }
                }
            }
        }
        closedir(dir);
    }
    
    return "N/A";
}

#endif
