#include "compact_disk_info.h"
#include "platform/Platform.h"
#include <sys/statvfs.h>
#include <fstream>
#include <sstream>
#include <set>

#if PLATFORM_LINUX

DiskInfo::DiskInfo() {}

std::vector<std::pair<std::string, int>> DiskInfo::getAllDiskUsage() {
    std::vector<std::pair<std::string, int>> result;
    std::set<std::string> seen;
    
    std::ifstream mounts("/proc/mounts");
    std::string line;
    
    while (std::getline(mounts, line)) {
        std::istringstream iss(line);
        std::string device, mountpoint, fstype;
        iss >> device >> mountpoint >> fstype;
        
        bool isRealFs = (device.find("/dev/") == 0) || 
                        (fstype == "zfs") || 
                        (fstype == "btrfs" && mountpoint != "/") ||
                        (fstype == "ext4" || fstype == "ext3" || fstype == "xfs");
        
        if (!isRealFs) continue;
        if (fstype == "squashfs" || fstype == "tmpfs" || fstype == "devtmpfs") continue;
        if (mountpoint.find("/sys") == 0 || mountpoint.find("/proc") == 0) continue;
        if (mountpoint == "/nix/store") continue;
        if (seen.count(mountpoint)) continue;
        seen.insert(mountpoint);
        
        int usage = calculateUsedPercentage(mountpoint);
        
        std::string label = mountpoint;
        if (mountpoint == "/") label = "/";
        else if (mountpoint.rfind('/') != std::string::npos) {
            label = mountpoint.substr(mountpoint.rfind('/') + 1);
        }
        
        result.push_back({label, usage});
    }
    
    return result;
}

std::vector<std::pair<std::string, int>> DiskInfo::getDiskCapacity() {
    std::vector<std::pair<std::string, int>> result;
    std::set<std::string> seen;
    
    std::ifstream mounts("/proc/mounts");
    std::string line;
    
    while (std::getline(mounts, line)) {
        std::istringstream iss(line);
        std::string device, mountpoint, fstype;
        iss >> device >> mountpoint >> fstype;
        
        bool isRealFs = (device.find("/dev/") == 0) || 
                        (fstype == "zfs") || 
                        (fstype == "btrfs" && mountpoint != "/") ||
                        (fstype == "ext4" || fstype == "ext3" || fstype == "xfs");
        
        if (!isRealFs) continue;
        if (fstype == "squashfs" || fstype == "tmpfs" || fstype == "devtmpfs") continue;
        if (mountpoint.find("/sys") == 0 || mountpoint.find("/proc") == 0) continue;
        if (mountpoint == "/nix/store") continue;
        if (seen.count(mountpoint)) continue;
        seen.insert(mountpoint);
        
        int capacity = calculateCapacityGB(mountpoint);
        
        std::string label = mountpoint;
        if (mountpoint == "/") label = "/";
        else if (mountpoint.rfind('/') != std::string::npos) {
            label = mountpoint.substr(mountpoint.rfind('/') + 1);
        }
        
        result.push_back({label, capacity});
    }
    
    return result;
}

int DiskInfo::calculateUsedPercentage(const std::string& path) {
    struct statvfs stat;
    if (statvfs(path.c_str(), &stat) != 0) return 0;
    
    unsigned long long total = stat.f_blocks * stat.f_frsize;
    unsigned long long available = stat.f_bavail * stat.f_frsize;
    
    if (total == 0) return 0;
    return static_cast<int>(((total - available) * 100) / total);
}

int DiskInfo::calculateCapacityGB(const std::string& path) {
    struct statvfs stat;
    if (statvfs(path.c_str(), &stat) != 0) return 0;
    
    unsigned long long total = stat.f_blocks * stat.f_frsize;
    return static_cast<int>(total / (1024ULL * 1024ULL * 1024ULL));
}

#elif PLATFORM_FREEBSD

#include <sys/mount.h>

DiskInfo::DiskInfo() {}

std::vector<std::pair<std::string, int>> DiskInfo::getAllDiskUsage() {
    std::vector<std::pair<std::string, int>> result;
    
    struct statfs* mounts;
    int count = getmntinfo(&mounts, MNT_NOWAIT);
    
    for (int i = 0; i < count; i++) {
        std::string fstype = mounts[i].f_fstypename;
        std::string device = mounts[i].f_mntfromname;
        std::string mountpoint = mounts[i].f_mntonname;
        
        if (device.find("/dev/") != 0) continue;
        if (fstype == "devfs" || fstype == "nullfs" || fstype == "tmpfs") continue;
        
        int usage = calculateUsedPercentage(mountpoint);
        
        std::string label = mountpoint;
        if (mountpoint == "/") label = "/";
        else if (mountpoint.rfind('/') != std::string::npos) {
            label = mountpoint.substr(mountpoint.rfind('/') + 1);
        }
        
        result.push_back({label, usage});
    }
    
    return result;
}

std::vector<std::pair<std::string, int>> DiskInfo::getDiskCapacity() {
    std::vector<std::pair<std::string, int>> result;
    
    struct statfs* mounts;
    int count = getmntinfo(&mounts, MNT_NOWAIT);
    
    for (int i = 0; i < count; i++) {
        std::string fstype = mounts[i].f_fstypename;
        std::string device = mounts[i].f_mntfromname;
        std::string mountpoint = mounts[i].f_mntonname;
        
        if (device.find("/dev/") != 0) continue;
        if (fstype == "devfs" || fstype == "nullfs" || fstype == "tmpfs") continue;
        
        int capacity = calculateCapacityGB(mountpoint);
        
        std::string label = mountpoint;
        if (mountpoint == "/") label = "/";
        else if (mountpoint.rfind('/') != std::string::npos) {
            label = mountpoint.substr(mountpoint.rfind('/') + 1);
        }
        
        result.push_back({label, capacity});
    }
    
    return result;
}

int DiskInfo::calculateUsedPercentage(const std::string& path) {
    struct statvfs stat;
    if (statvfs(path.c_str(), &stat) != 0) return 0;
    
    unsigned long long total = stat.f_blocks * stat.f_frsize;
    unsigned long long available = stat.f_bavail * stat.f_frsize;
    
    if (total == 0) return 0;
    return static_cast<int>(((total - available) * 100) / total);
}

int DiskInfo::calculateCapacityGB(const std::string& path) {
    struct statvfs stat;
    if (statvfs(path.c_str(), &stat) != 0) return 0;
    
    unsigned long long total = stat.f_blocks * stat.f_frsize;
    return static_cast<int>(total / (1024ULL * 1024ULL * 1024ULL));
}

#endif
