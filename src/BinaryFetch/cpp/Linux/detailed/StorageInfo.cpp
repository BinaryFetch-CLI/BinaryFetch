#define _GNU_SOURCE
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
#include <algorithm>
#include <thread>

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

// Returns just the last path component of a mount point, e.g.
//   "/"          -> "/"
//   "/boot/efi"  -> "efi"
//   "/home"      -> "home"
static string short_mount_name(const string& target) {
    if (target.empty()) return target;
    if (target == "/") return "/";

    string t = target;
    while (t.size() > 1 && t.back() == '/') t.pop_back(); // strip trailing slash if any

    size_t pos = t.find_last_of('/');
    if (pos != string::npos && pos + 1 < t.size()) {
        return t.substr(pos + 1);
    }
    return t;
}

// Left-justify `s` to `width` with trailing spaces so multiple entries
// line up in a fixed-width column.
static string pad_right(const string& s, size_t width) {
    string out = s;
    if (out.size() < width) out.append(width - out.size(), ' ');
    return out;
}

// Truncate a serial number to its first 4 characters.
static string truncate_serial(const string& sn) {
    return sn.substr(0, std::min<size_t>(4, sn.size()));
}

// Returns:
//   >= 0.0  -> measured speed in MB/s
//   -1.0    -> could not test (no write permission, or the I/O itself
//              failed), caller should report this as "N/A" rather than a
//              misleading 0.00
//
// Cache-bypass strategy (the Linux equivalent of what the Windows version
// does with FILE_FLAG_NO_BUFFERING): O_DIRECT is NOT used here, because it
// is unreliable across Linux filesystems -- it can open fine and then fail
// on the actual write() with EINVAL on overlayfs, some container mounts,
// and certain alignment cases, which silently produced the "0.00" results.
// Instead:
//   - write test: buffered write, then fsync() to force it to physical
//     media before stopping the clock -- measures real write throughput.
//   - read test: posix_fadvise(DONTNEED) evicts the file from the page
//     cache first, so the read has to come from the device, not RAM.
static double measure_disk_speed(const string& root_path, bool writeTest) {
    // If we can't even write to this mount, don't pretend we measured 0 MB/s --
    // that reads as "this disk is extremely slow" when it's actually just a
    // permissions issue (e.g. running as a non-root user against "/" or
    // "/boot/efi").
    if (access(root_path.c_str(), W_OK) != 0) return -1.0;

    const size_t BUF_SIZE = 64 * 1024 * 1024; // 64 MiB: large enough that open/seek overhead is negligible

    void* raw_buffer = nullptr;
    if (posix_memalign(&raw_buffer, 4096, BUF_SIZE) != 0) return -1.0;
    memset(raw_buffer, 0xAA, BUF_SIZE);
    char* buffer = static_cast<char*>(raw_buffer);

    string testFile = root_path;
    if (!testFile.empty() && testFile.back() != '/') testFile += '/';
    testFile += ".binaryfetch_speed_test.bin";

    double result = -1.0;

    if (writeTest) {
        int fd = open(testFile.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);

        if (fd >= 0) {
            auto start = chrono::high_resolution_clock::now();

            size_t totalWritten = 0;
            while (totalWritten < BUF_SIZE) {
                ssize_t w = write(fd, buffer + totalWritten, BUF_SIZE - totalWritten);
                if (w <= 0) break; // real error or short write we can't recover from
                totalWritten += static_cast<size_t>(w);
            }

            // Force the data all the way to physical media before we stop
            // timing -- otherwise we're just measuring "copied into RAM".
            fsync(fd);
            auto end = chrono::high_resolution_clock::now();
            close(fd);

            if (totalWritten == BUF_SIZE) {
                double seconds = chrono::duration<double>(end - start).count();
                if (seconds < 0.001) seconds = 0.001;
                result = (totalWritten / (1024.0 * 1024.0)) / seconds;
            } else {
                result = -1.0; // genuine I/O failure, not "the disk is 0 MB/s"
            }
        }
    } else {
        int fd = open(testFile.c_str(), O_RDONLY);

        if (fd >= 0) {
            // Evict the file from the page cache so this read actually hits
            // the device instead of returning memory-speed cached data.
            posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);

            auto start = chrono::high_resolution_clock::now();

            size_t totalRead = 0;
            while (totalRead < BUF_SIZE) {
                ssize_t r = read(fd, buffer + totalRead, BUF_SIZE - totalRead);
                if (r <= 0) break;
                totalRead += static_cast<size_t>(r);
            }

            auto end = chrono::high_resolution_clock::now();
            close(fd);
            unlink(testFile.c_str());

            if (totalRead == BUF_SIZE) {
                double seconds = chrono::duration<double>(end - start).count();
                if (seconds < 0.001) seconds = 0.001;
                result = (totalRead / (1024.0 * 1024.0)) / seconds;
            } else {
                result = -1.0;
            }
        }
    }

    free(raw_buffer);
    return result;
}

// Formats a measure_disk_speed() result for display: "N/A" for the
// permission-denied sentinel, otherwise a fixed 2-decimal MB/s string.
static string format_speed(double v) {
    if (v < 0.0) return "N/A";
    ostringstream ss;
    ss << fixed << setprecision(2) << v;
    return ss.str();
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

        // Not "real" user-facing drives -- boot/firmware partitions
        // (e.g. /boot, /boot/efi) rather than actual storage volumes.
        if (target == "/boot" || target.rfind("/boot/", 0) == 0) continue;

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

// `short_name` and `pad_width` are precomputed by the caller across the
// whole mount list so every drive_letter comes back the same length and
// columns stay aligned regardless of how long/short each mount path is.
static storage_data build_disk_data(const mount_entry& m, int disk_index,
                                     const string& short_name, size_t pad_width) {
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

    disk.drive_letter = pad_right(short_name, pad_width);
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

    // Same write-then-read-with-retry pattern as the Windows implementation:
    // measure write, small delay for the fs to settle, measure read: if
    // either genuinely failed (the -1.0 sentinel, not a permission issue --
    // access() already gated that above), retry once after a longer delay
    // before accepting the result.
    double w = measure_disk_speed(m.target, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    double r = measure_disk_speed(m.target, false);

    if (w < 0.0 || r < 0.0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        w = measure_disk_speed(m.target, true);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        r = measure_disk_speed(m.target, false);
    }

    disk.read_speed = format_speed(r);
    disk.write_speed = format_speed(w);

    disk.serial_number = truncate_serial(to_string(1000 + disk_index));

    ifstream serial("/sys/block/" + base + "/device/serial");
    string sn;
    if (serial >> sn && !sn.empty()) disk.serial_number = truncate_serial(sn);

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

// Computes the short display name for every mount and the max width across
// them, so callers can hand each one to build_disk_data() pre-aligned.
static void compute_short_names(const vector<mount_entry>& mounts,
                                 vector<string>& short_names_out, size_t& pad_width_out) {
    short_names_out.clear();
    pad_width_out = 0;
    short_names_out.reserve(mounts.size());

    for (const auto& m : mounts) {
        string name = short_mount_name(m.target);
        pad_width_out = std::max(pad_width_out, name.size());
        short_names_out.push_back(name);
    }
}

vector<storage_data> StorageInfo::get_all_storage_info() {
    vector<storage_data> all_disks;

    auto mounts = get_mount_entries();
    vector<string> short_names;
    size_t pad_width = 0;
    compute_short_names(mounts, short_names, pad_width);

    for (size_t i = 0; i < mounts.size(); ++i) {
        all_disks.push_back(build_disk_data(mounts[i], static_cast<int>(i), short_names[i], pad_width));
    }

    return all_disks;
}

void StorageInfo::process_storage_info(function<void(const storage_data&)> callback) {
    auto mounts = get_mount_entries();
    vector<string> short_names;
    size_t pad_width = 0;
    compute_short_names(mounts, short_names, pad_width);

    for (size_t i = 0; i < mounts.size(); ++i) {
        callback(build_disk_data(mounts[i], static_cast<int>(i), short_names[i], pad_width));
    }
}