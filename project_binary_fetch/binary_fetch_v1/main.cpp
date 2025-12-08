#include <iostream>
#include <iomanip>
#include <vector>
#include <functional>
#include <sstream>
#include <thread>
#include <future>
#include <memory>
#include <chrono>

// Full System Info Modules
#include "OSInfo.h"
#include "CPUInfo.h"
#include "MemoryInfo.h"
#include "GPUInfo.h"
#include "StorageInfo.h"
#include "NetworkInfo.h"
#include "DetailedGPUInfo.h"
#include "PerformanceInfo.h"
#include "UserInfo.h"
#include "SystemInfo.h"
#include "DisplayInfo.h"
#include "ExtraInfo.h"

// Compact Mode Modules
#include "CompactAudio.h"
#include "CompactOS.h"
#include "CompactCPU.h"
#include "CompactMemory.h"
#include "CompactScreen.h"
#include "CompactSystem.h"
#include "CompactGPU.h"
#include "CompactPerformance.h"
#include "CompactUser.h"
#include "CompactNetwork.h"
#include "compact_disk_info.h"

// UI Components
#include "AsciiArt.h"

using namespace std;

// ==================== COMPACT INFO FUNCTIONS ====================

void print_compact_header() {
    cout << endl;
    cout << "_>> BinaryFetch____________________________________________________" << endl;
}

void print_compact_os(CompactOS& c_os) {
    cout << "[OS]  -> " << c_os.getOSName()
        << c_os.getOSBuild()
        << " (" << c_os.getArchitecture() << ")"
        << " (uptime: " << c_os.getUptime() << ")" << endl;
}

void print_compact_cpu(CompactCPU& c_cpu) {
    cout << "[CPU] -> " << c_cpu.getCPUName() << " ("
        << c_cpu.getCPUCores() << "C/"
        << c_cpu.getCPUThreads() << "T)"
        << fixed << setprecision(2)
        << " @ " << c_cpu.getClockSpeed() << " GHz " << endl;
}

void print_compact_displays(CompactScreen& c_screen) {
    auto screens = c_screen.get_screens();
    int i = 1;
    for (const auto& s : screens) {
        cout << "[Display " << i++ << "] -> "
            << s.brand_name << " (" << s.resolution << ") @"
            << s.refresh_rate << "Hz\n";
    }
}

void print_compact_memory(CompactMemory& c_memory) {
    cout << "[Memory] -> " << "(total: " << c_memory.get_total_memory() << " GB)"
        << " (free: " << c_memory.get_free_memory() << " GB)"
        << " ( " << c_memory.get_used_memory_percent() << "% ) " << endl;
}

void print_compact_audio(CompactAudio& c_audio) {
    cout << "[Audio Input] -> " << c_audio.active_audio_input()
        << c_audio.active_audio_input_status() << endl;
    cout << "[Audio Output] -> " << c_audio.active_audio_output()
        << c_audio.active_audio_output_status() << endl;
}

void print_compact_system(CompactSystem& c_system) {
    cout << "[BIOS] -> " << c_system.getBIOSInfo() << endl;
    cout << "[Motherboard] -> " << c_system.getMotherboardInfo() << endl;
}

void print_compact_gpu(CompactGPU& c_gpu) {
    cout << "[GPU] -> " << c_gpu.getGPUName()
        << " (" << c_gpu.getGPUUsagePercent() << "%)"
        << " (" << c_gpu.getVRAMGB() << " GB)"
        << " (@" << c_gpu.getGPUFrequency() << ") " << endl;
}

void print_compact_performance(CompactPerformance& c_perf) {
    cout << "[Performance] -> "
        << "(CPU: " << c_perf.getCPUUsage() << "%) "
        << "(GPU: " << c_perf.getGPUUsage() << "%) "
        << "(RAM: " << c_perf.getRAMUsage() << "%) "
        << "(Disk: " << c_perf.getDiskUsage() << "%) " << endl;
}

void print_compact_user(CompactUser& c_user) {
    cout << "[User] -> @" << c_user.getUsername()
        << " -> (Domain: " << c_user.getDomain()
        << ") -> (Type: " << c_user.isAdmin() << ")" << endl;
}

void print_compact_network(CompactNetwork& c_net) {
    cout << "[network] -> " << "(Name: " << c_net.get_network_name()
        << ") (Type: " << c_net.get_network_type()
        << ") (ip: " << c_net.get_network_ip() << ") " << endl;
}

void print_compact_disk(DiskInfo& disk) {
    auto disks = disk.getAllDiskUsage();

    cout << "[Disk] -> ";
    for (const auto& d : disks) {
        cout << "(" << d.first[0] << ": "
            << fixed << setprecision(1)
            << d.second << "%) ";
    }
    cout << endl;

    auto caps = disk.getDiskCapacity();
    cout << "[Disk Cap] -> ";
    for (const auto& c : caps) {
        cout << "(" << c.first[0] << "-" << c.second << "GB)";
    }
    cout << endl;
}

// Combined function for all compact info
void print_all_compact_info(CompactOS& c_os, CompactCPU& c_cpu, CompactScreen& c_screen,
    CompactMemory& c_memory, CompactAudio& c_audio, CompactSystem& c_system,
    CompactGPU& c_gpu, CompactPerformance& c_perf, CompactUser& c_user,
    CompactNetwork& c_net, DiskInfo& disk) {
    print_compact_header();
    print_compact_os(c_os);
    print_compact_cpu(c_cpu);
    print_compact_displays(c_screen);
    print_compact_memory(c_memory);
    print_compact_audio(c_audio);
    print_compact_system(c_system);
    print_compact_gpu(c_gpu);
    print_compact_performance(c_perf);
    print_compact_user(c_user);
    print_compact_network(c_net);
    print_compact_disk(disk);
}

// ==================== DETAILED INFO FUNCTIONS ====================

void print_detailed_memory(MemoryInfo& ram) {
    cout << endl;
    cout << "---------------Memory Info--------------" << endl;
    cout << "(Total: " << ram.getTotal() << " GB) "
        << "(Free: " << ram.getFree() << " GB) "
        << "(Used: " << ram.getUsedPercentage() << "%)\n";

    const auto& modules = ram.getModules();
    for (size_t i = 0; i < modules.size(); ++i) {
        string cap = modules[i].capacity;
        int num = 0;
        try { num = stoi(cap); }
        catch (...) { num = 0; }

        ostringstream capOut;
        capOut << setw(2) << setfill('0') << num << "GB";

        cout << "Memory " << i << ": "
            << "(Used: " << ram.getUsedPercentage() << "%) "
            << capOut.str() << " "
            << modules[i].type << " "
            << modules[i].speed << "\n";
    }
    cout << endl;
}

void print_detailed_storage(StorageInfo& storage) {
    const auto& all_disks = storage.get_all_storage_info();
    if (all_disks.empty()) {
        cout << "--- Storage Info ---\nNo drives detected.\n\n";
        return;
    }

    auto fmt_storage = [](const string& s) {
        ostringstream oss;
        double v = stod(s);
        oss << fixed << setprecision(2)
            << setw(7) << right << setfill(' ')
            << v;
        return oss.str();
        };

    auto fmt_speed = [](const string& s) {
        ostringstream tmp;
        double v = stod(s);
        tmp << fixed << setprecision(2) << v;
        string val = tmp.str();
        int padding = 7 - val.size();
        if (padding < 0) padding = 0;
        return string(padding, ' ') + val;
        };

    cout << "------------------------ STORAGE SUMMARY --------------------------\n";
    for (const auto& d : all_disks) {
        cout << d.storage_type << " " << d.drive_letter
            << " [ (Used) " << fmt_storage(d.used_space)
            << " GiB / " << fmt_storage(d.total_space)
            << " GiB " << d.used_percentage
            << " - " << d.file_system << " "
            << (d.is_external ? "Ext]" : "Int]") << "\n";
    }

    cout << "\n---------------------- DISK PERFORMANCE & DETAILS ----------------------\n";
    for (const auto& d : all_disks) {
        cout << d.drive_letter << " [ Read: ("
            << fmt_speed(d.read_speed)
            << " MB/s) | Write: ("
            << fmt_speed(d.write_speed)
            << " MB/s) | " << d.serial_number
            << (d.is_external ? " Ext ]" : " Int ]") << "\n";
    }

    cout << "\n----------------- DISK PERFORMANCE & DETAILS (Predicted) ---------------\n";
    for (const auto& d : all_disks) {
        cout << d.drive_letter << " [ Read: ("
            << fmt_speed(d.predicted_read_speed)
            << " MB/s) | Write: ("
            << fmt_speed(d.predicted_write_speed)
            << " MB/s) | " << d.serial_number
            << (d.is_external ? " Ext ]" : " Int ]") << "\n";
    }
    cout << endl;
}

void print_detailed_network(CompactNetwork& c_net) {
    cout << "--- Network Info (Compact + Extra) ---\n";
    cout << "Network Name: " << c_net.get_network_name() << "\n";
    cout << "Network Type: " << c_net.get_network_type() << "\n";
    cout << "IP (compact): " << c_net.get_network_ip() << "\n";
    cout << endl;
}

void print_detailed_audio_power(ExtraInfo& extra) {
    cout << "--- Audio & Power Info ---\n";
    cout << "Audio Devices: " << extra.get_audio_devices() << "\n";
    cout << "Power Status: " << extra.get_power_status() << "\n";
    cout << endl;
}

void print_detailed_os(OSInfo& os) {
    cout << "--- OS Info ---\n";
    cout << "Name: " << os.GetOSName() << "\n";
    cout << "Version: " << os.GetOSVersion() << "\n";
    cout << "Architecture: " << os.GetOSArchitecture() << "\n";
    cout << "Kernel: " << os.get_os_kernel_info() << "\n";
    cout << "Uptime: " << os.get_os_uptime() << "\n";
    cout << "Install Date: " << os.get_os_install_date() << "\n";
    cout << "Serial: " << os.get_os_serial_number() << "\n";
    cout << endl;
}

void print_detailed_cpu(CPUInfo& cpu) {
    cout << "--- CPU Info ---\n";
    cout << "Brand: " << cpu.get_cpu_info() << "\n";
    cout << "Utilization: " << cpu.get_cpu_utilization() << "%\n";
    cout << "Speed: " << cpu.get_cpu_speed() << "\n";
    cout << "Base Speed: " << cpu.get_cpu_base_speed() << "\n";
    cout << "Cores: " << cpu.get_cpu_cores() << "\n";
    cout << "Logical Processors: " << cpu.get_cpu_logical_processors() << "\n";
    cout << "Sockets: " << cpu.get_cpu_sockets() << "\n";
    cout << "Virtualization: " << cpu.get_cpu_virtualization() << "\n";
    cout << "L1 Cache: " << cpu.get_cpu_l1_cache() << "\n";
    cout << "L2 Cache: " << cpu.get_cpu_l2_cache() << "\n";
    cout << "L3 Cache: " << cpu.get_cpu_l3_cache() << "\n";
    cout << endl;
}

void print_detailed_gpu(GPUInfo& obj_gpu, DetailedGPUInfo& detailed_gpu_info) {
    auto all_gpu_info = obj_gpu.get_all_gpu_info();
    if (all_gpu_info.empty()) {
        cout << "--- GPU Info ---\nNo GPU detected.\n\n";
        return;
    }

    cout << "--- GPU Info ---\n";
    for (size_t i = 0; i < all_gpu_info.size(); ++i) {
        auto& g = all_gpu_info[i];
        cout << "GPU " << i + 1 << ":\n";
        cout << "  Name: " << g.gpu_name << "\n";
        cout << "  Memory: " << g.gpu_memory << "\n";
        cout << "  Usage: " << g.gpu_usage << "%\n";
        cout << "  Vendor: " << g.gpu_vendor << "\n";
        cout << "  Driver Version: " << g.gpu_driver_version << "\n";
        cout << "  Temperature: " << g.gpu_temperature << " C\n";
        cout << "  Core Count: " << g.gpu_core_count << "\n";
        cout << endl;
    }

    auto primary = detailed_gpu_info.primary_gpu_info();
    cout << "Primary GPU Details:\n";
    cout << "  Name: " << primary.name << "\n";
    cout << "  VRAM: " << primary.vram_gb << " GiB\n";
    cout << "  Frequency: " << primary.frequency_ghz << " GHz\n\n";
}

void print_detailed_display(DisplayInfo& display) {
    cout << "--- Display Info ---\n";
    auto monitors = display.get_all_displays();
    if (monitors.empty()) {
        cout << "No monitors detected.\n\n";
        return;
    }

    for (size_t i = 0; i < monitors.size(); ++i) {
        auto& m = monitors[i];
        cout << "Monitor " << i + 1 << ":\n";
        cout << "  Brand: " << m.brand_name << "\n";
        cout << "  Resolution: " << m.resolution << "\n";
        cout << "  Refresh Rate: " << m.refresh_rate << " Hz\n";
        cout << endl;
    }
}

void print_detailed_system(SystemInfo& sys) {
    cout << "--- BIOS & Motherboard Info ---\n";
    cout << "Bios Vendor: " << sys.get_bios_vendor() << "\n";
    cout << "Bios Version: " << sys.get_bios_version() << "\n";
    cout << "Bios Date: " << sys.get_bios_date() << "\n";
    cout << "Motherboard Model: " << sys.get_motherboard_model() << "\n";
    cout << "Motherboard Manufacturer: " << sys.get_motherboard_manufacturer() << "\n";
    cout << endl;
}

void print_detailed_user(UserInfo& user) {
    cout << "--- User Info ---\n";
    cout << "Username: " << user.get_username() << "\n";
    cout << "Computer Name: " << user.get_computer_name() << "\n";
    cout << "Domain: " << user.get_domain_name() << "\n";
    cout << "Groups: " << user.get_user_groups() << "\n";
    cout << endl;
}

void print_detailed_performance(PerformanceInfo& perf) {
    cout << "--- Performance Info ---\n";
    cout << "System Uptime: " << perf.get_system_uptime() << "\n";
    cout << "CPU Usage: " << perf.get_cpu_usage_percent() << "%\n";
    cout << "RAM Usage: " << perf.get_ram_usage_percent() << "%\n";
    cout << "Disk Usage: " << perf.get_disk_usage_percent() << "%\n";
    cout << "GPU Usage: " << perf.get_gpu_usage_percent() << "%\n";
    cout << endl;
}

// ==================== MAIN ====================

int main() {
    // Initialize ASCII Art
    AsciiArt art;
    if (!art.loadFromFile("AsciiArt.txt")) {
        cout << "Note: ASCII art not loaded. Showing info only.\n\n";
    }

    // Initialize all info objects in parallel using async
    auto fut_os = async(launch::async, []() { return make_shared<OSInfo>(); });
    auto fut_cpu = async(launch::async, []() { return make_shared<CPUInfo>(); });
    auto fut_ram = async(launch::async, []() { return make_shared<MemoryInfo>(); });
    auto fut_gpu = async(launch::async, []() { return make_shared<GPUInfo>(); });
    auto fut_detailed_gpu = async(launch::async, []() { return make_shared<DetailedGPUInfo>(); });
    auto fut_storage = async(launch::async, []() { return make_shared<StorageInfo>(); });
    auto fut_net = async(launch::async, []() { return make_shared<NetworkInfo>(); });
    auto fut_user = async(launch::async, []() { return make_shared<UserInfo>(); });
    auto fut_perf = async(launch::async, []() { return make_shared<PerformanceInfo>(); });
    auto fut_display = async(launch::async, []() { return make_shared<DisplayInfo>(); });
    auto fut_extra = async(launch::async, []() { return make_shared<ExtraInfo>(); });
    auto fut_sys = async(launch::async, []() { return make_shared<SystemInfo>(); });

    // Compact modules
    auto fut_c_audio = async(launch::async, []() { return make_shared<CompactAudio>(); });
    auto fut_c_os = async(launch::async, []() { return make_shared<CompactOS>(); });
    auto fut_c_cpu = async(launch::async, []() { return make_shared<CompactCPU>(); });
    auto fut_c_screen = async(launch::async, []() { return make_shared<CompactScreen>(); });
    auto fut_c_memory = async(launch::async, []() { return make_shared<CompactMemory>(); });
    auto fut_c_system = async(launch::async, []() { return make_shared<CompactSystem>(); });
    auto fut_c_gpu = async(launch::async, []() { return make_shared<CompactGPU>(); });
    auto fut_c_perf = async(launch::async, []() { return make_shared<CompactPerformance>(); });
    auto fut_c_user = async(launch::async, []() { return make_shared<CompactUser>(); });
    auto fut_c_net = async(launch::async, []() { return make_shared<CompactNetwork>(); });
    auto fut_disk = async(launch::async, []() { return make_shared<DiskInfo>(); });

    // Wait for all futures to complete
    auto os = fut_os.get();
    auto cpu = fut_cpu.get();
    auto ram = fut_ram.get();
    auto obj_gpu = fut_gpu.get();
    auto detailed_gpu_info = fut_detailed_gpu.get();
    auto storage = fut_storage.get();
    auto net = fut_net.get();
    auto user = fut_user.get();
    auto perf = fut_perf.get();
    auto display = fut_display.get();
    auto extra = fut_extra.get();
    auto sys = fut_sys.get();

    auto c_audio = fut_c_audio.get();
    auto c_os = fut_c_os.get();
    auto c_cpu = fut_c_cpu.get();
    auto c_screen = fut_c_screen.get();
    auto c_memory = fut_c_memory.get();
    auto c_system = fut_c_system.get();
    auto c_gpu = fut_c_gpu.get();
    auto c_perf = fut_c_perf.get();
    auto c_user = fut_c_user.get();
    auto c_net = fut_c_net.get();
    auto disk = fut_disk.get();

    // Print with ASCII art
    art.printWithArt([&]() {
        // Compact info
        print_all_compact_info(*c_os, *c_cpu, *c_screen, *c_memory, *c_audio,
            *c_system, *c_gpu, *c_perf, *c_user, *c_net, *disk);

        // Detailed info
        print_detailed_memory(*ram);
        print_detailed_storage(*storage);
        print_detailed_network(*c_net);
        print_detailed_audio_power(*extra);
        print_detailed_os(*os);
        print_detailed_cpu(*cpu);
        print_detailed_gpu(*obj_gpu, *detailed_gpu_info);
        print_detailed_display(*display);
        print_detailed_system(*sys);
        print_detailed_user(*user);
        print_detailed_performance(*perf);
        });

    cout << endl;
    return 0;
}