#include "compact_disk_info.h"
#include <windows.h>

DiskInfo::DiskInfo() {
    // Constructor (empty)
}

// Calculate used percentage for a single disk (integer)
int DiskInfo::calculateUsedPercentage(const std::string& driveLetter) {
    ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes;

    if (GetDiskFreeSpaceExA(driveLetter.c_str(),
        &freeBytesAvailable,
        &totalNumberOfBytes,
        &totalNumberOfFreeBytes)) {

        if (totalNumberOfBytes.QuadPart > 0) {
            return static_cast<int>(
                ((totalNumberOfBytes.QuadPart - totalNumberOfFreeBytes.QuadPart) * 100)
                / totalNumberOfBytes.QuadPart
                );
        }
    }

    return 0;
}

// Get all disk usage (%)
std::vector<std::pair<std::string, int>> DiskInfo::getAllDiskUsage() {
    std::vector<std::pair<std::string, int>> diskList;

    for (char drive = 'A'; drive <= 'Z'; ++drive) {
        std::string driveLetter = std::string(1, drive) + ":\\";

        if (GetDriveTypeA(driveLetter.c_str()) == DRIVE_FIXED ||
            GetDriveTypeA(driveLetter.c_str()) == DRIVE_REMOVABLE) {

            int used = calculateUsedPercentage(driveLetter);
            diskList.push_back({ driveLetter, used });
        }
    }

    return diskList;
}

// Helper: calculate capacity in GB
int DiskInfo::calculateCapacityGB(const std::string& driveLetter) {
    ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes;

    if (GetDiskFreeSpaceExA(driveLetter.c_str(),
        &freeBytesAvailable,
        &totalNumberOfBytes,
        &totalNumberOfFreeBytes)) {
        // Convert bytes to GB
        return static_cast<int>(totalNumberOfBytes.QuadPart / (1024 * 1024 * 1024));
    }

    return 0;
}

// Get all disk capacities in GB
std::vector<std::pair<std::string, int>> DiskInfo::getDiskCapacity() {
    std::vector<std::pair<std::string, int>> diskCapList;

    for (char drive = 'A'; drive <= 'Z'; ++drive) {
        std::string driveLetter = std::string(1, drive) + ":\\";

        if (GetDriveTypeA(driveLetter.c_str()) == DRIVE_FIXED ||
            GetDriveTypeA(driveLetter.c_str()) == DRIVE_REMOVABLE) {

            int capacity = calculateCapacityGB(driveLetter);
            diskCapList.push_back({ driveLetter, capacity });
        }
    }

    return diskCapList;
}
