#include "CompactMemory.h"
#include "platform/Platform.h"
#include <sstream>

#if PLATFORM_LINUX

double CompactMemory::get_total_memory() {
    std::string content = Platform::readFile("/proc/meminfo");
    std::string val = Platform::parseValue(content, "MemTotal");
    if (!val.empty()) {
        long long kb = std::stoll(val);
        return static_cast<double>(kb) / (1024.0 * 1024.0);
    }
    return 0.0;
}

double CompactMemory::get_free_memory() {
    std::string content = Platform::readFile("/proc/meminfo");
    
    std::string available = Platform::parseValue(content, "MemAvailable");
    if (!available.empty()) {
        long long kb = std::stoll(available);
        return static_cast<double>(kb) / (1024.0 * 1024.0);
    }
    
    long long memFree = 0, buffers = 0, cached = 0;
    std::string val = Platform::parseValue(content, "MemFree");
    if (!val.empty()) memFree = std::stoll(val);
    val = Platform::parseValue(content, "Buffers");
    if (!val.empty()) buffers = std::stoll(val);
    val = Platform::parseValue(content, "Cached");
    if (!val.empty()) cached = std::stoll(val);
    
    long long freeKb = memFree + buffers + cached;
    return static_cast<double>(freeKb) / (1024.0 * 1024.0);
}

double CompactMemory::get_used_memory_percent() {
    std::string content = Platform::readFile("/proc/meminfo");
    
    long long memTotal = 0, memAvailable = 0;
    std::string val = Platform::parseValue(content, "MemTotal");
    if (!val.empty()) memTotal = std::stoll(val);
    
    val = Platform::parseValue(content, "MemAvailable");
    if (!val.empty()) {
        memAvailable = std::stoll(val);
    } else {
        long long memFree = 0, buffers = 0, cached = 0;
        val = Platform::parseValue(content, "MemFree");
        if (!val.empty()) memFree = std::stoll(val);
        val = Platform::parseValue(content, "Buffers");
        if (!val.empty()) buffers = std::stoll(val);
        val = Platform::parseValue(content, "Cached");
        if (!val.empty()) cached = std::stoll(val);
        memAvailable = memFree + buffers + cached;
    }
    
    if (memTotal == 0) return 0.0;
    long long used = memTotal - memAvailable;
    return static_cast<double>(used * 100) / static_cast<double>(memTotal);
}

int CompactMemory::memory_slot_used() {
    if (Platform::commandExists("dmidecode")) {
        std::string result = Platform::exec("sudo dmidecode -t memory 2>/dev/null | grep -c 'Size:.*MB\\|Size:.*GB' || echo 0");
        result = Platform::trim(result);
        if (!result.empty()) {
            try { return std::stoi(result); }
            catch (...) {}
        }
    }
    return 0;
}

int CompactMemory::memory_slot_available() {
    if (Platform::commandExists("dmidecode")) {
        std::string result = Platform::exec("sudo dmidecode -t memory 2>/dev/null | grep -c 'Size:' || echo 0");
        result = Platform::trim(result);
        if (!result.empty()) {
            try { return std::stoi(result); }
            catch (...) {}
        }
    }
    return 0;
}

#elif PLATFORM_FREEBSD

double CompactMemory::get_total_memory() {
    unsigned long physmem = Platform::sysctlULong("hw.physmem");
    return static_cast<double>(physmem) / (1024.0 * 1024.0 * 1024.0);
}

double CompactMemory::get_free_memory() {
    unsigned long pagesize = Platform::sysctlULong("hw.pagesize");
    if (pagesize == 0) pagesize = 4096;
    
    unsigned long free_count = Platform::sysctlULong("vm.stats.vm.v_free_count");
    unsigned long inactive = Platform::sysctlULong("vm.stats.vm.v_inactive_count");
    unsigned long cache = Platform::sysctlULong("vm.stats.vm.v_cache_count");
    
    unsigned long available = (free_count + inactive + cache) * pagesize;
    return static_cast<double>(available) / (1024.0 * 1024.0 * 1024.0);
}

double CompactMemory::get_used_memory_percent() {
    unsigned long physmem = Platform::sysctlULong("hw.physmem");
    unsigned long pagesize = Platform::sysctlULong("hw.pagesize");
    if (pagesize == 0) pagesize = 4096;
    
    unsigned long free_count = Platform::sysctlULong("vm.stats.vm.v_free_count");
    unsigned long inactive = Platform::sysctlULong("vm.stats.vm.v_inactive_count");
    unsigned long cache = Platform::sysctlULong("vm.stats.vm.v_cache_count");
    
    unsigned long available = (free_count + inactive + cache) * pagesize;
    
    if (physmem == 0) return 0.0;
    unsigned long used = physmem - available;
    return static_cast<double>(used * 100) / static_cast<double>(physmem);
}

int CompactMemory::memory_slot_used() {
    return 0;
}

int CompactMemory::memory_slot_available() {
    return 0;
}

#endif
