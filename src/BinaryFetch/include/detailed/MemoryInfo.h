#ifndef MEMORYINFO_H
#define MEMORYINFO_H

#include <string>
#include <vector>
using namespace std;
struct MemoryModule {
    string capacity; // e.g., "16GB"
    string type;     // e.g., "DDR4"
    string speed;    // e.g., "2133 MHz"
};

class MemoryInfo {
private:
    int totalGB;
    int freeGB;
    vector<MemoryModule> modules;

    void fetchSystemMemory();    // total/free memory
    void fetchModulesInfo();     // per-module info

public:
    MemoryInfo();

    int getTotal() const;
    int getFree() const;
    int getUsedPercentage() const;

    const vector<MemoryModule>& getModules() const;
};

#endif
