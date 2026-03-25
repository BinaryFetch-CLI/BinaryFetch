#pragma once
#include <string>
using namespace std;
class CompactCPU {
public:
    string getCPUName();        
    string getCPUCores();      // returns "6" (core)
    string getCPUThreads();    // returns "12" (threds)
    double getClockSpeed();    // returns 4.40 (ghz)
    double getUsagePercent();  // returns 12.5 (usage in double)
};
