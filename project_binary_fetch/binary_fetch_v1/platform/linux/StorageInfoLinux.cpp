#include "../Platform.h"
#include "../../StorageInfo.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <sys/statvfs.h>
#include <mntent.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <chrono>
#include <set>

static std::set<std::string> pseudoFS = {"proc", "sysfs", "devtmpfs", "tmpfs", "securityfs", 
    "cgroup", "cgroup2", "pstore", "debugfs", "hugetlbfs", "mqueue", "fusectl", 
    "configfs", "devpts", "ramfs", "binfmt_misc", "autofs", "tracefs", "overlay", "squashfs"};

static std::string getBlockDevice(const std::string& mountPath) {
    FILE* mtab = setmntent("/proc/mounts", "r");
    if (!mtab) return "";
    
    struct mntent* ent;
    while ((ent = getmntent(mtab)) != nullptr) {
        if (std::string(ent->mnt_dir) == mountPath) {
            std::string dev = ent->mnt_fsname;
            endmntent(mtab);
            
            if (dev.find("/dev/") == 0) {
                std::string base = dev.substr(5);
                while (!base.empty() && (std::isdigit(base.back()) || base.back() == 'p')) {
                    base.pop_back();
                }
                return base;
            }
            return "";
        }
    }
    endmntent(mtab);
    return "";
}

std::string StorageInfo::get_storage_type(const std::string&, const std::string& root_path, bool) {
    std::string blockDev = getBlockDevice(root_path);
    if (blockDev.empty()) return "Unknown";
    
    std::string rotPath = "/sys/block/" + blockDev + "/queue/rotational";
    std::string rot = Platform::readFileLine(rotPath);
    
    if (!rot.empty()) {
        if (Platform::trim(rot) == "0") return "SSD";
        if (Platform::trim(rot) == "1") return "HDD";
    }
    
    if (blockDev.find("nvme") == 0) return "SSD";
    if (blockDev.find("mmcblk") == 0) return "SSD";
    
    std::string removePath = "/sys/block/" + blockDev + "/removable";
    std::string removable = Platform::readFileLine(removePath);
    if (!removable.empty() && Platform::trim(removable) == "1") return "USB";
    
    return "Unknown";
}

static double measureSpeed(const std::string& path, bool write) {
    const size_t BUF_SIZE = 16 * 1024 * 1024;
    std::vector<char> buffer(BUF_SIZE, 'X');
    
    std::string testFile = path + "/.binaryfetch_speed_test";
    
    if (write) {
        auto start = std::chrono::high_resolution_clock::now();
        int fd = open(testFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, 0644);
        if (fd < 0) return 0.0;
        
        ssize_t written = ::write(fd, buffer.data(), BUF_SIZE);
        fsync(fd);
        close(fd);
        
        auto end = std::chrono::high_resolution_clock::now();
        
        if (written <= 0) {
            unlink(testFile.c_str());
            return 0.0;
        }
        
        double seconds = std::chrono::duration<double>(end - start).count();
        if (seconds < 0.001) seconds = 0.001;
        
        return (written / (1024.0 * 1024.0)) / seconds;
    } else {
        int fd = open(testFile.c_str(), O_RDONLY);
        if (fd < 0) return 0.0;
        
        auto start = std::chrono::high_resolution_clock::now();
        ssize_t bytesRead = read(fd, buffer.data(), BUF_SIZE);
        close(fd);
        unlink(testFile.c_str());
        
        auto end = std::chrono::high_resolution_clock::now();
        
        if (bytesRead <= 0) return 0.0;
        
        double seconds = std::chrono::duration<double>(end - start).count();
        if (seconds < 0.001) seconds = 0.001;
        
        return (bytesRead / (1024.0 * 1024.0)) / seconds;
    }
}

std::vector<storage_data> StorageInfo::get_all_storage_info() {
    std::vector<storage_data> all_disks;
    
    FILE* mtab = setmntent("/proc/mounts", "r");
    if (!mtab) return all_disks;
    
    std::set<std::string> seen;
    struct mntent* ent;
    
    while ((ent = getmntent(mtab)) != nullptr) {
        std::string fstype = ent->mnt_type;
        std::string device = ent->mnt_fsname;
        std::string mountpoint = ent->mnt_dir;
        
        if (pseudoFS.count(fstype) || device.find("/dev/") != 0) continue;
        if (seen.count(device)) continue;
        seen.insert(device);
        
        struct statvfs stat;
        if (statvfs(mountpoint.c_str(), &stat) != 0) continue;
        
        unsigned long long total = stat.f_blocks * stat.f_frsize;
        unsigned long long free = stat.f_bfree * stat.f_frsize;
        unsigned long long used = total - free;
        
        if (total < 100 * 1024 * 1024) continue;
        
        double totalGiB = total / (1024.0 * 1024.0 * 1024.0);
        double usedGiB = used / (1024.0 * 1024.0 * 1024.0);
        double usedPercent = (total > 0) ? (used * 100.0 / total) : 0.0;
        
        storage_data disk;
        disk.drive_letter = "Disk (" + mountpoint + ")";
        
        std::ostringstream usedStr, totalStr, percentStr;
        usedStr << std::fixed << std::setprecision(2) << usedGiB;
        totalStr << std::fixed << std::setprecision(2) << totalGiB;
        percentStr << "(" << static_cast<int>(usedPercent) << "%)";
        
        disk.used_space = usedStr.str();
        disk.total_space = totalStr.str();
        disk.used_percentage = percentStr.str();
        disk.file_system = fstype;
        disk.storage_type = get_storage_type("", mountpoint, false);
        disk.is_external = (disk.storage_type == "USB");
        
        double w = measureSpeed(mountpoint, true);
        usleep(100000);
        double r = measureSpeed(mountpoint, false);
        
        std::ostringstream readStr, writeStr;
        readStr << std::fixed << std::setprecision(2) << r;
        writeStr << std::fixed << std::setprecision(2) << w;
        disk.read_speed = readStr.str();
        disk.write_speed = writeStr.str();
        
        if (disk.storage_type == "USB") {
            disk.predicted_read_speed = "100";
            disk.predicted_write_speed = "80";
        } else if (disk.storage_type == "SSD") {
            disk.predicted_read_speed = "500";
            disk.predicted_write_speed = "450";
        } else if (disk.storage_type == "HDD") {
            disk.predicted_read_speed = "140";
            disk.predicted_write_speed = "120";
        } else {
            disk.predicted_read_speed = "---";
            disk.predicted_write_speed = "---";
        }
        
        disk.serial_number = "N/A";
        
        all_disks.push_back(disk);
    }
    
    endmntent(mtab);
    return all_disks;
}

void StorageInfo::process_storage_info(std::function<void(const storage_data&)> callback) {
    auto disks = get_all_storage_info();
    for (const auto& disk : disks) {
        callback(disk);
    }
}
