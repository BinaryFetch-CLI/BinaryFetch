#include "../Platform.h"
#include "../../GPUInfo.h"
#include <fstream>
#include <sstream>
#include <iomanip>

static std::string getVendorFromPciConf(const std::string& line) {
    if (line.find("NVIDIA") != std::string::npos || line.find("nvidia") != std::string::npos) return "NVIDIA";
    if (line.find("AMD") != std::string::npos || line.find("ATI") != std::string::npos || 
        line.find("Radeon") != std::string::npos) return "AMD";
    if (line.find("Intel") != std::string::npos) return "Intel";
    return "Unknown";
}

float GPUInfo::get_gpu_usage() {
    if (Platform::commandExists("nvidia-smi")) {
        std::string result = Platform::exec("nvidia-smi --query-gpu=utilization.gpu --format=csv,noheader,nounits 2>/dev/null");
        if (!result.empty()) {
            try { return std::stof(Platform::trim(result)); }
            catch (...) {}
        }
    }
    
    std::string result = Platform::exec("sysctl -n dev.drm.0.hwmon.temp 2>/dev/null");
    if (!result.empty()) {
        return -1.0f;
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
    
    std::string result = Platform::exec("sysctl -n hw.acpi.thermal.tz0.temperature 2>/dev/null");
    if (!result.empty()) {
        try {
            std::string temp = Platform::trim(result);
            size_t cPos = temp.find('C');
            if (cPos != std::string::npos) {
                temp = temp.substr(0, cPos);
            }
            return std::stof(temp);
        } catch (...) {}
    }
    
    return -1.0f;
}

int GPUInfo::get_gpu_core_count() {
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
    
    if (list.empty()) {
        std::string pciconf = Platform::exec("pciconf -lv 2>/dev/null | grep -B4 -E 'display|VGA|3D'");
        
        std::istringstream iss(pciconf);
        std::string line;
        gpu_data current;
        bool inGPU = false;
        
        while (std::getline(iss, line)) {
            if (line.find("display") != std::string::npos || 
                line.find("VGA") != std::string::npos ||
                line.find("3D") != std::string::npos) {
                if (inGPU && !current.gpu_name.empty()) {
                    list.push_back(current);
                }
                current = gpu_data();
                inGPU = true;
            }
            
            if (inGPU) {
                if (line.find("device") != std::string::npos && line.find("=") != std::string::npos) {
                    size_t eq = line.find("=");
                    if (eq != std::string::npos) {
                        std::string name = line.substr(eq + 1);
                        name.erase(std::remove(name.begin(), name.end(), '\''), name.end());
                        current.gpu_name = Platform::trim(name);
                        current.gpu_vendor = getVendorFromPciConf(name);
                    }
                }
                if (line.find("vendor") != std::string::npos && line.find("=") != std::string::npos) {
                    size_t eq = line.find("=");
                    if (eq != std::string::npos) {
                        std::string vendor = line.substr(eq + 1);
                        vendor.erase(std::remove(vendor.begin(), vendor.end(), '\''), vendor.end());
                        current.gpu_vendor = getVendorFromPciConf(vendor);
                    }
                }
            }
        }
        
        if (inGPU && !current.gpu_name.empty()) {
            current.gpu_memory = "Unknown";
            current.gpu_driver_version = "Unknown";
            current.gpu_usage = get_gpu_usage();
            current.gpu_temperature = get_gpu_temperature();
            current.gpu_frequency = -1.0f;
            current.gpu_core_count = 0;
            list.push_back(current);
        }
    }
    
    return list;
}
