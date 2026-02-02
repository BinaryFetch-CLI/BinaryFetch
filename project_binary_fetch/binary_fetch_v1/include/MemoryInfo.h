#ifndef MEMORYINFO_H
#define MEMORYINFO_H

#include <string>
#include <vector>

struct MemoryModule {
    std::string capacity; // e.g., "16GB"
    std::string type;     // e.g., "DDR4"
    std::string speed;    // e.g., "2133 MHz"
};

class MemoryInfo {
private:
    int totalGB;
    int freeGB;
    std::vector<MemoryModule> modules;

    void fetchSystemMemory();    // total/free memory
    void fetchModulesInfo();     // per-module info

public:
    MemoryInfo();

    int getTotal() const;
    int getFree() const;
    int getUsedPercentage() const;

    const std::vector<MemoryModule>& getModules() const;
};

#endif
