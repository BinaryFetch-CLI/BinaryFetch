#pragma once
#include <vector>
#include <string>

class DiskInfo {
public:
    DiskInfo();  // Constructor

    // Get usage of all disks as integers (%)
    std::vector<std::pair<std::string, int>> getAllDiskUsage();

    // Get capacity of all disks as integers (GB)
    std::vector<std::pair<std::string, int>> getDiskCapacity();

private:
    // Helper: calculate used percentage
    int calculateUsedPercentage(const std::string& driveLetter);

    // Helper: calculate total capacity in GB
    int calculateCapacityGB(const std::string& driveLetter);
};
