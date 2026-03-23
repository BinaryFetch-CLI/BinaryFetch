#pragma once
#include <string>

class CompactCPU {
public:
    std::string getCPUName();
    std::string getCPUCores();      // returns "6" (core)
    std::string getCPUThreads();    // returns "12" (threds)
    double getClockSpeed();         // returns 4.40 (ghz)
    double getUsagePercent();       // returns 12.5 (usage in double)
};
