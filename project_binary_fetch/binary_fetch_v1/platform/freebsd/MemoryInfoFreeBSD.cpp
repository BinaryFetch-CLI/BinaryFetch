#include "../Platform.h"
#include "../../MemoryInfo.h"
#include <fstream>
#include <sstream>
#include <sys/sysctl.h>

MemoryInfo::MemoryInfo() {
    fetchSystemMemory();
    fetchModulesInfo();
}

void MemoryInfo::fetchSystemMemory() {
    unsigned long physmem = Platform::sysctlULong("hw.physmem");
    
    unsigned long pagesize = Platform::sysctlULong("hw.pagesize");
    if (pagesize == 0) pagesize = 4096;
    
    unsigned long free_count = Platform::sysctlULong("vm.stats.vm.v_free_count");
    unsigned long inactive = Platform::sysctlULong("vm.stats.vm.v_inactive_count");
    unsigned long cache = Platform::sysctlULong("vm.stats.vm.v_cache_count");
    
    unsigned long available = (free_count + inactive + cache) * pagesize;
    
    totalGB = static_cast<int>((physmem + 1024UL * 1024UL * 1024UL - 1) / (1024UL * 1024UL * 1024UL));
    freeGB = static_cast<int>(available / (1024UL * 1024UL * 1024UL));
}

void MemoryInfo::fetchModulesInfo() {
    modules.clear();
    
    std::string output = Platform::exec("dmidecode -t memory 2>/dev/null | grep -E 'Size:|Speed:|Type:' | head -20");
    
    if (output.empty()) {
        MemoryModule mod;
        mod.capacity = std::to_string(totalGB) + "GB";
        mod.type = "Unknown";
        mod.speed = "Unknown";
        modules.push_back(mod);
        return;
    }
    
    std::istringstream iss(output);
    std::string line;
    MemoryModule current;
    
    while (std::getline(iss, line)) {
        line = Platform::trim(line);
        
        if (line.find("Size:") == 0) {
            if (!current.capacity.empty() && current.capacity.find("No Module") == std::string::npos) {
                modules.push_back(current);
            }
            current = MemoryModule();
            
            std::string size = line.substr(5);
            size = Platform::trim(size);
            if (size.find("No Module") == std::string::npos) {
                current.capacity = size;
            }
        }
        else if (line.find("Type:") == 0 && current.type.empty()) {
            std::string type = Platform::trim(line.substr(5));
            if (type != "Unknown" && type != "Other") {
                current.type = type;
            }
        }
        else if (line.find("Speed:") == 0 && current.speed.empty()) {
            std::string speed = Platform::trim(line.substr(6));
            if (speed != "Unknown") {
                current.speed = speed;
            }
        }
    }
    
    if (!current.capacity.empty() && current.capacity.find("No Module") == std::string::npos) {
        modules.push_back(current);
    }
    
    if (modules.empty()) {
        MemoryModule mod;
        mod.capacity = std::to_string(totalGB) + "GB";
        mod.type = "Unknown";
        mod.speed = "Unknown";
        modules.push_back(mod);
    }
}

int MemoryInfo::getTotal() const { return totalGB; }
int MemoryInfo::getFree() const { return freeGB; }

int MemoryInfo::getUsedPercentage() const {
    if (totalGB == 0) return 0;
    double percentage = (static_cast<double>(totalGB - freeGB) / totalGB) * 100;
    if (percentage > 100.0) percentage = 100.0;
    if (percentage < 0.0) percentage = 0.0;
    return static_cast<int>(percentage);
}

const std::vector<MemoryModule>& MemoryInfo::getModules() const { return modules; }
