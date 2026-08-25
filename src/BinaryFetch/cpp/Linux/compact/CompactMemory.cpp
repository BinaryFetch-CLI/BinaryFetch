#include "CompactMemory.h"
#include <fstream>
#include <sstream>
#include <string>

// Helper to parse values in kB from /proc/meminfo
static double getMeminfoValue(const std::string& key) {
    std::ifstream file("/proc/meminfo");
    if (!file.is_open()) return 0.0;

    std::string line;
    while (std::getline(file, line)) {
        if (line.find(key + ":") == 0) {
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                double valKB = 0.0;
                std::stringstream ss(line.substr(colon + 1));
                if (ss >> valKB) {
                    return valKB * 1024.0; // convert kB to bytes
                }
            }
        }
    }
    return 0.0;
}

// Get total physical RAM in GB
double CompactMemory::get_total_memory() {
    double bytes = getMeminfoValue("MemTotal");
    return bytes / (1024.0 * 1024.0 * 1024.0);
}

// Get free/available physical RAM in GB
double CompactMemory::get_free_memory() {
    double availableBytes = getMeminfoValue("MemAvailable");
    if (availableBytes == 0.0) {
        // Fallback for older kernels
        double free = getMeminfoValue("MemFree");
        double buffers = getMeminfoValue("Buffers");
        double cached = getMeminfoValue("Cached");
        availableBytes = free + buffers + cached;
    }
    return availableBytes / (1024.0 * 1024.0 * 1024.0);
}

// Get percent of memory used
double CompactMemory::get_used_memory_percent() {
    double total = getMeminfoValue("MemTotal");
    double available = getMeminfoValue("MemAvailable");
    if (available == 0.0) {
         double free = getMeminfoValue("MemFree");
         double buffers = getMeminfoValue("Buffers");
         double cached = getMeminfoValue("Cached");
         available = free + buffers + cached;
    }
    if (total <= 0.0) return 0.0;
    return 100.0 * (total - available) / total;
}

// Get RAM slots used (requires root or specific sysfs access, fallback to 0/1 on error)
int CompactMemory::memory_slot_used() {
    // Standard Linux doesn't expose DMI slots without root (dmidecode).
    // Return a sensible default or 0/unknown.
    return 0;
}

// Get total RAM slots available
int CompactMemory::memory_slot_available() {
    return 0;
}
