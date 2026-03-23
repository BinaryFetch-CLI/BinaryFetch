#pragma once
#include <vector>
#include <string>
using namespace std;

class DiskInfo {
public:
    DiskInfo();  // Constructor

    // Get usage of all disks as integers (%)
    vector<pair<string, int>> getAllDiskUsage();

    // Get capacity of all disks as integers (GB)
    vector<pair<string, int>> getDiskCapacity();

private:
    // Helper: calculate used percentage
    int calculateUsedPercentage(const string& driveLetter);

    // Helper: calculate total capacity in GB
    int calculateCapacityGB(const string& driveLetter);
};
