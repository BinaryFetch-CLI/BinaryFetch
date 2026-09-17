#include "compact_disk_info.h"
#include <sys/statvfs.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>

DiskInfo::DiskInfo() {
    // Empty constructor
}

// Calculate disk usage percentage for a given mount point path
int DiskInfo::calculateUsedPercentage(const std::string& path) {
    struct statvfs stat;
    if (statvfs(path.c_str(), &stat) == 0) {
        double total = static_cast<double>(stat.f_blocks) * stat.f_frsize;
        double free = static_cast<double>(stat.f_bfree) * stat.f_frsize;
        if (total > 0.0) {
            return static_cast<int>(((total - free) * 100.0) / total);
        }
    }
    return 0;
}

// Filesystem types we consider "real" physical disks
static bool isPhysicalFsType(const std::string& fsType) {
    static const std::unordered_set<std::string> allowed = {
        "ext2", "ext3", "ext4", "xfs", "btrfs", "ntfs", "ntfs3",
        "exfat", "f2fs", "reiserfs", "jfs", "zfs"
    };
    return allowed.count(fsType) > 0;
}

// Helper to scan `/proc/mounts` and find physical disk mount points
static std::vector<std::string> getPhysicalMountPoints() {
    std::vector<std::string> mountPoints;
    std::unordered_set<std::string> seenMountPoints; // dedupe

    std::ifstream file("/proc/mounts");
    if (!file.is_open()) return mountPoints;

    std::string device, mountPoint, fsType, options;
    int dump, pass;
    while (file >> device >> mountPoint >> fsType >> options >> dump >> pass) {
        // Must be a real block device
        if (device.rfind("/dev/", 0) != 0) continue;

        // Exclude loop devices, virtual/pseudo mounts
        if (device.find("/dev/loop") != std::string::npos) continue;

        // Exclude anything not a "real" disk filesystem
        // (this also drops vfat/EFI partitions, squashfs, overlay, tmpfs, etc.)
        if (!isPhysicalFsType(fsType)) continue;

        // Skip duplicates — /proc/mounts can list the same mount point
        // more than once (bind mounts, remounts, etc.)
        if (seenMountPoints.count(mountPoint)) continue;
        seenMountPoints.insert(mountPoint);

        mountPoints.push_back(mountPoint);
    }
    return mountPoints;
}

// Get all physical disk usage percentages
std::vector<std::pair<std::string, int>> DiskInfo::getAllDiskUsage() {
    std::vector<std::pair<std::string, int>> usages;
    auto mounts = getPhysicalMountPoints();
    for (const auto& mnt : mounts) {
        usages.push_back({mnt, calculateUsedPercentage(mnt)});
    }
    return usages;
}

// Calculate capacity in GB for a mount point path
int DiskInfo::calculateCapacityGB(const std::string& path) {
    struct statvfs stat;
    if (statvfs(path.c_str(), &stat) == 0) {
        double totalBytes = static_cast<double>(stat.f_blocks) * stat.f_frsize;
        return static_cast<int>(totalBytes / (1024.0 * 1024.0 * 1024.0));
    }
    return 0;
}

// Get all physical disk capacities
std::vector<std::pair<std::string, int>> DiskInfo::getDiskCapacity() {
    std::vector<std::pair<std::string, int>> capacities;
    auto mounts = getPhysicalMountPoints();
    for (const auto& mnt : mounts) {
        capacities.push_back({mnt, calculateCapacityGB(mnt)});
    }
    return capacities;
}