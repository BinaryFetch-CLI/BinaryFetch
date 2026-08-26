#include "StorageInfo.h"
#include <sys/statvfs.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <fstream>
#include <set>

using namespace std;

static bool is_pseudo_fs(const string& fstype) {
    static const set<string> pseudo = {
        "proc", "sysfs", "devtmpfs", "tmpfs", "devpts", "cgroup", "cgroup2",
        "pstore", "securityfs", "debugfs", "tracefs", "configfs", "fusectl",
        "mqueue", "hugetlbfs", "autofs", "rpc_pipefs", "binfmt_misc",
        "overlay", "squashfs", "efivarfs", "fuse.gvfsd-fuse", "fuse.portal",
        "fuse.snapfuse", "bpf"
    };
    return pseudo.count(fstype) > 0;
}

static string base_device_name(string dev) {
    size_t pos = dev.find_last_of('/');
    if (pos != string::npos) dev = dev.substr(pos + 1);

    if (dev.rfind("nvme", 0) == 0 || dev.rfind("mmcblk", 0) == 0) {
        for (size_t i = dev.size(); i-- > 0;) {
            if (dev[i] == 'p' && i + 1 < dev.size() && isdigit((unsigned char)dev[i + 1])) {
                dev = dev.substr(0, i);
                break;
            }
        }
    } else {
        while (!dev.empty() && isdigit((unsigned char)dev.back())) dev.pop_back();
    }
    return dev;
}

static double measure_disk_speed(const string& root_path, bool writeTest) {
    const size_t BUF_SIZE = 32 * 1024 * 1024;

    void* raw_buffer = nullptr;
    if (posix_memalign(&raw_buffer, 4096, BUF_SIZE) != 0) return 0.0;
    memset(raw_buffer, 0xAA, BUF_SIZE);
    char* buffer = static_cast<char*>(raw_buffer);

    string testFile = root_path;
    if (!testFile.empty() && testFile.back() != '/') testFile += '/';
    testFile += ".binaryfetch_speed_test.bin";

    double result = 0.0;

    if (writeTest) {
        int fd = open(testFile.c_str(), O_CREAT | O_WRONLY | O_TRUNC | O_DIRECT, 0644);
        if (fd < 0) fd = open(testFile.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);

        if (fd >= 0) {
            auto start = chrono::high_resolution_clock::now();
            ssize_t written = write(fd, buffer, BUF_SIZE);
            fsync(fd);
            auto end = chrono::high_resolution_clock::now();
            close(fd);

            if (written > 0) {
                double seconds = chrono::duration<double>(end - start).count();
                if (seconds < 0.001) seconds = 0.001;
                result = (written / (1024.0 * 1024.0)) / seconds;
            }
        }
    } else {
        int fd = open(testFile.c_str(), O_RDONLY | O_DIRECT);
        if (fd < 0) fd = open(testFile.c_str(), O_RDONLY);

        if (fd >= 0) {
            auto start = chrono::high_resolution_clock::now();
            ssize_t r = read(fd, buffer, BUF_SIZE);
            auto end = chrono::high_resolution_clock::now();
            close(fd);
            unlink(testFile.c_str());

            if (r > 0) {
                double seconds = chrono::duration<double>(end - start).count();
                if (seconds < 0.001) seconds = 0.001;
                result = (r / (1024.0 * 1024.0)) / seconds;
            }
        }
    }

    free(raw_buffer);
    return result;
}

struct mount_entry {
    string source;
    string target;
    string fstype;
};

static vector<mount_entry> get_mount_entries() {
    vector<mount_entry> entries;
    ifstream mounts("/proc/mounts");
    string line;

    while (getline(mounts, line)) {
        istringstream iss(line);
        string source, target, fstype;
        iss >> source >> target >> fstype;

        if (source.rfind("/dev/", 0) != 0) continue;
        if (is_pseudo_fs(fstype)) continue;

        entries.push_back({ source, target, fstype });
    }

    return entries;
}

static string detect_storage_type(const string& root_path, bool is_external) {
    if (is_external) return "USB";

    string best_source;
    size_t best_len = 0;

    for (const auto& m : get_mount_entries()) {
        if (root_path.compare(0, m.target.size(), m.target) == 0 && m.target.size() > best_len) {
            best_len = m.target.size();
            best_source = m.source;
        }
    }

    if (best_source.empty()) return "Unknown";

    string base = base_device_name(best_source);

    ifstream removable("/sys/block/" + base + "/removable");
    int rem = 0;
    if (removable >> rem && rem == 1) return "USB";

    ifstream rota("/sys/block/" + base + "/queue/rotational");
    int val = -1;
    if (rota >> val) return val == 0 ? "SSD" : "HDD";

    return "Unknown";
}

string StorageInfo::get_storage_type(const string&, const string& root_path, bool is_external) {
    return detect_storage_type(root_path, is_external);
}

static storage_data build_disk_data(const mount_entry& m, int disk_index) {
    storage_data disk;

    struct statvfs vfs{};
    if (statvfs(m.target.c_str(), &vfs) != 0) return disk;

    double total_gib = (double)(vfs.f_blocks * vfs.f_frsize) / (1024.0 * 1024.0 * 1024.0);
    double free_gib = (double)(vfs.f_bfree * vfs.f_frsize) / (1024.0 * 1024.0 * 1024.0);
    double used_gib = total_gib - free_gib;
    double used_percent = (total_gib > 0) ? (used_gib / total_gib) * 100.0 : 0.0;

    string base = base_device_name(m.source);

    bool is_external = false;
    ifstream removable("/sys/block/" + base + "/removable");
    int rem = 0;
    if (removable >> rem && rem == 1) is_external = true;

    ostringstream used_str, total_str;
    used_str << fixed << setprecision(2) << used_gib;
    total_str << fixed << setprecision(2) << total_gib;

    disk.drive_letter = "Disk (" + m.target + ")";
    disk.used_space = used_str.str();
    disk.total_space = total_str.str();
    disk.used_percentage = static_cast<int>(used_percent);
    disk.file_system = m.fstype;
    disk.is_external = is_external;

    try {
        disk.storage_type = detect_storage_type(m.target, is_external);
    } catch (...) {
        disk.storage_type = "Unknown";
    }

    double w = measure_disk_speed(m.target, true);
    double r = measure_disk_speed(m.target, false);

    ostringstream ss;
    ss << fixed << setprecision(2) << (r > 0 ? r : 0.0);
    disk.read_speed = ss.str();
    ss.str("");
    ss.clear();
    ss << fixed << setprecision(2) << (w > 0 ? w : 0.0);
    disk.write_speed = ss.str();

    disk.serial_number = to_string(1000 + disk_index);

    ifstream serial("/sys/block/" + base + "/device/serial");
    string sn;
    if (serial >> sn && !sn.empty()) disk.serial_number = sn;

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

    return disk;
}

vector<storage_data> StorageInfo::get_all_storage_info() {
    vector<storage_data> all_disks;
    int disk_index = 0;

    for (const auto& m : get_mount_entries()) {
        all_disks.push_back(build_disk_data(m, disk_index));
        disk_index++;
    }

    return all_disks;
}

void StorageInfo::process_storage_info(function<void(const storage_data&)> callback) {
    int disk_index = 0;

    for (const auto& m : get_mount_entries()) {
        callback(build_disk_data(m, disk_index));
        disk_index++;
    }
}