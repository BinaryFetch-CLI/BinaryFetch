#include "../Platform.h"
#include "../../GPUInfo.h"
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <cstring>
#include <iomanip>

static std::string findGPUInDRM() {
    std::vector<std::string> gpus;
    DIR* dir = opendir("/sys/class/drm");
    if (!dir) return "";
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name.find("card") == 0 && name.find("-") == std::string::npos) {
            std::string vendorPath = "/sys/class/drm/" + name + "/device/vendor";
            std::string vendor = Platform::readFileLine(vendorPath);
            if (!vendor.empty()) {
                gpus.push_back(name);
            }
        }
    }
    closedir(dir);
    return gpus.empty() ? "" : gpus[0];
}

float GPUInfo::get_gpu_usage() {
    if (Platform::commandExists("nvidia-smi")) {
        std::string result = Platform::exec("nvidia-smi --query-gpu=utilization.gpu --format=csv,noheader,nounits 2>/dev/null");
        if (!result.empty()) {
            try { return std::stof(Platform::trim(result)); }
            catch (...) {}
        }
    }
    
    std::string card = findGPUInDRM();
    if (!card.empty()) {
        std::string busyPath = "/sys/class/drm/" + card + "/device/gpu_busy_percent";
        std::string busy = Platform::readFileLine(busyPath);
        if (!busy.empty()) {
            try { return std::stof(busy); }
            catch (...) {}
        }
    }
    
    return -1.0f;
}

float GPUInfo::get_gpu_temperature() {
    if (Platform::commandExists("nvidia-smi")) {
        std::string result = Platform::exec("nvidia-smi --query-gpu=temperature.gpu --format=csv,noheader,nounits 2>/dev/null");
        if (!result.empty()) {
            try { return std::stof(Platform::trim(result)); }
            catch (...) {}
        }
    }
    
    std::string card = findGPUInDRM();
    if (!card.empty()) {
        DIR* hwmonDir = opendir(("/sys/class/drm/" + card + "/device/hwmon").c_str());
        if (hwmonDir) {
            struct dirent* entry;
            while ((entry = readdir(hwmonDir)) != nullptr) {
                if (std::string(entry->d_name).find("hwmon") == 0) {
                    std::string tempPath = "/sys/class/drm/" + card + "/device/hwmon/" + entry->d_name + "/temp1_input";
                    std::string temp = Platform::readFileLine(tempPath);
                    if (!temp.empty()) {
                        closedir(hwmonDir);
                        try { return std::stof(temp) / 1000.0f; }
                        catch (...) { return -1.0f; }
                    }
                }
            }
            closedir(hwmonDir);
        }
    }
    
    return -1.0f;
}

int GPUInfo::get_gpu_core_count() {
    if (Platform::commandExists("nvidia-smi")) {
        std::string result = Platform::exec("nvidia-smi --query-gpu=gpu_name --format=csv,noheader 2>/dev/null");
        return 0;
    }
    return 0;
}

std::vector<gpu_data> GPUInfo::get_all_gpu_info() {
    std::vector<gpu_data> list;
    
    if (Platform::commandExists("nvidia-smi")) {
        std::string result = Platform::exec("nvidia-smi --query-gpu=name,memory.total,driver_version,utilization.gpu,temperature.gpu,clocks.gr --format=csv,noheader,nounits 2>/dev/null");
        
        std::istringstream iss(result);
        std::string line;
        while (std::getline(iss, line)) {
            if (line.empty()) continue;
            
            auto parts = Platform::split(line, ',');
            if (parts.size() >= 6) {
                gpu_data d;
                d.gpu_name = Platform::trim(parts[0]);
                
                float memMB = std::stof(Platform::trim(parts[1]));
                std::ostringstream memStream;
                memStream << std::fixed << std::setprecision(1) << (memMB / 1024.0f) << " GB";
                d.gpu_memory = memStream.str();
                
                d.gpu_driver_version = Platform::trim(parts[2]);
                d.gpu_vendor = "NVIDIA";
                d.gpu_usage = std::stof(Platform::trim(parts[3]));
                d.gpu_temperature = std::stof(Platform::trim(parts[4]));
                d.gpu_frequency = std::stof(Platform::trim(parts[5]));
                d.gpu_core_count = 0;
                
                list.push_back(d);
            }
        }
    }
    
    std::string lspci = Platform::exec("lspci -nn 2>/dev/null | grep -i 'vga\\|3d\\|display'");
    if (!lspci.empty() && list.empty()) {
        std::istringstream iss(lspci);
        std::string line;
        while (std::getline(iss, line)) {
            gpu_data d;
            
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos && colonPos + 1 < line.length()) {
                d.gpu_name = Platform::trim(line.substr(colonPos + 1));
                size_t bracketPos = d.gpu_name.find('[');
                if (bracketPos != std::string::npos) {
                    d.gpu_name = Platform::trim(d.gpu_name.substr(0, bracketPos));
                }
            }
            
            if (line.find("10de") != std::string::npos) d.gpu_vendor = "NVIDIA";
            else if (line.find("1002") != std::string::npos) d.gpu_vendor = "AMD";
            else if (line.find("8086") != std::string::npos) d.gpu_vendor = "Intel";
            else d.gpu_vendor = "Unknown";
            
            d.gpu_memory = "Unknown";
            d.gpu_driver_version = "Unknown";
            d.gpu_usage = get_gpu_usage();
            d.gpu_temperature = get_gpu_temperature();
            d.gpu_frequency = -1.0f;
            d.gpu_core_count = 0;
            
            if (!d.gpu_name.empty()) {
                list.push_back(d);
            }
        }
    }
    
    return list;
}
