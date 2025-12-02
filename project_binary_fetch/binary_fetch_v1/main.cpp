
#include <iostream>
#include <iomanip>   // interblink
#include <vector>
#include <functional>
#include <sstream>


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

#include "AsciiArt.h"

using namespace std;

int main() {

    // Initialize ASCII Art
    AsciiArt art;
    if (!art.loadFromFile("AsciiArt.txt")) {
        cout << "Note: ASCII art not loaded. Showing info only.\n\n";
    }

    // CentralControl removed - no config.json loaded here
    OSInfo os;
    CPUInfo cpu;
    MemoryInfo ram;
    GPUInfo obj_gpu;
    DetailedGPUInfo detailed_gpu_info;
    StorageInfo storage;
    NetworkInfo net;
    UserInfo user;
    PerformanceInfo perf;
    DisplayInfo display;
    ExtraInfo extra;
    SystemInfo sys;

    CompactAudio c_audio;
    CompactOS c_os;
    CompactCPU c_cpu;
    CompactScreen c_screen;
    CompactMemory c_memory;
    CompactSystem c_system;
    CompactGPU c_gpu;
    CompactPerformance c_perf;
    CompactUser c_user;
    CompactNetwork c_net;
    DiskInfo disk;

    // Use ASCII art with your existing compact output
    //--------------ascii art layout------------------------------------
    art.printWithArt([&]() {

        cout << endl;
        cout << "_>> BinaryFetch____________________________________________________" << endl;

        // ---------------- Compact Summary Section ----------------
        cout << "[OS]  -> " << c_os.getOSName()
            << c_os.getOSBuild()
            << " (" << c_os.getArchitecture() << ")"
            << " (uptime: " << c_os.getUptime() << ")" << endl;

        cout << "[CPU] -> " << c_cpu.getCPUName() << " ("
            << c_cpu.getCPUCores() << "C/"
            << c_cpu.getCPUThreads() << "T)"
            << std::fixed << std::setprecision(2)
            << " @ " << c_cpu.getClockSpeed() << " GHz " << endl;

        auto screens = c_screen.get_screens();
        int i = 1;
        for (const auto& s : screens) {
            cout << "[Display " << i++ << "] -> "
                << s.brand_name << " (" << s.resolution << ") @"
                << s.refresh_rate << "Hz\n";
        }

        cout << "[Memory] -> " << "(total: " << c_memory.get_total_memory() << " GB)"
            << " (free: " << c_memory.get_free_memory() << " GB)"
            << " ( " << c_memory.get_used_memory_percent() << "% ) " << endl;

        cout << "[Audio Input] -> " << c_audio.active_audio_input()
            << c_audio.active_audio_input_status() << endl;

        cout << "[Audio Output] -> " << c_audio.active_audio_output()
            << c_audio.active_audio_output_status() << endl;

        cout << "[BIOS] -> " << c_system.getBIOSInfo() << endl;
        cout << "[Motherboard] -> " << c_system.getMotherboardInfo() << endl;

        cout << "[GPU] -> " << c_gpu.getGPUName()
            << " (" << c_gpu.getGPUUsagePercent() << "%)"
            << " (" << c_gpu.getVRAMGB() << " GB)"
            << " (@" << c_gpu.getGPUFrequency() << ") " << endl;

        cout << "[Performance] -> "
            << "(CPU: " << c_perf.getCPUUsage() << "%) "
            << "(GPU: " << c_perf.getGPUUsage() << "%) "
            << "(RAM: " << c_perf.getRAMUsage() << "%) "
            << "(Disk: " << c_perf.getDiskUsage() << "%) " << endl;

        cout << "[User] -> @" << c_user.getUsername()
            << " -> (Domain: " << c_user.getDomain()
            << ") -> (Type: " << c_user.isAdmin() << ")" << endl;

        cout << "[network] -> " << "(Name: " << c_net.get_network_name()
            << ") (Type: " << c_net.get_network_type()
            << ") (ip: " << c_net.get_network_ip() << ") " << endl;

        auto disks = disk.getAllDiskUsage();

        // print disk usage in percentage---------------------------------
        std::cout << "[Disk] -> ";
        for (const auto& d : disks) {
            std::cout << "(" << d.first[0] << ": "
                << std::fixed << std::setprecision(1)
                << d.second << "%) ";
        }
        std::cout << std::endl;
        // print disk capacity in GB---------------------------------------
        auto caps = disk.getDiskCapacity();
        std::cout << "[Disk Cap] -> ";
        for (const auto& c : caps) {
            std::cout << "(" << c.first[0] << "-" << c.second << "GB)";
        }
        std::cout << std::endl;



        // ---------------- FULL DETAILED INFO (Option A order) ----------------
        cout << endl;
        cout << "---------------Memory Info--------------" << endl;
        std::cout << "(Total: " << ram.getTotal() << " GB) "
            << "(Free: " << ram.getFree() << " GB) "
            << "(Used: " << ram.getUsedPercentage() << "%)\n";

        const auto& modules = ram.getModules();
        for (size_t i = 0; i < modules.size(); ++i) {

            // --- Zero-pad capacity ---
            std::string cap = modules[i].capacity;   // e.g. "8GB"
            int num = 0;

            try { num = std::stoi(cap); }  // Extract "8"

            catch (...) { num = 0; }

            std::ostringstream capOut;
            capOut << std::setw(2) << std::setfill('0') << num << "GB";

            // --- Print ---
            std::cout << "Memory " << i << ": "
                << "(Used: " << ram.getUsedPercentage() << "%) "
                << capOut.str() << " "
                << modules[i].type << " "
                << modules[i].speed << "\n";
        }

        cout << endl;

        // --- Storage Info ---
        {
            const auto& all_disks = storage.get_all_storage_info();
            if (all_disks.empty()) {
                cout << "--- Storage Info ---\nNo drives detected.\n\n";
            }
            else {

                cout << "----------------------- STORAGE SUMMARY ------------------------\n";
                for (const auto& d : all_disks) {

                    // storage values: normal alignment, no zero padding
                    auto fmt_storage = [](const std::string& s) {
                        std::ostringstream oss;
                        double v = stod(s);
                        oss << std::fixed << std::setprecision(2)
                            << std::setw(7) << std::right << std::setfill(' ')
                            << v;
                        return oss.str();
                        };

                    cout << d.drive_letter << " [ (Used) "
                        << fmt_storage(d.used_space) << " GiB / "
                        << fmt_storage(d.total_space) << " GiB "
                        << d.used_percentage 
                        << " - " << d.file_system;

                    if (d.is_external)
                        cout << " Ext ]";
                    else
                        cout << " Int ]";

                    cout << "\n";
                }

                cout << "\n---------------------- DISK PERFORMANCE & DETAILS ----------------------\n";

                for (const auto& d : all_disks) {

                    // speed values: underscore padding
                    auto fmt_speed = [](const std::string& s) {
                        std::ostringstream tmp;
                        double v = stod(s);
                        tmp << std::fixed << std::setprecision(2) << v;
                        std::string val = tmp.str();

                        // total width 7, pad with underscores
                        int padding = 7 - val.size();
                        if (padding < 0) padding = 0;

                        return std::string(padding, ' ') + val;
                        };

                    cout << d.drive_letter << " [ Read: ("
                        << fmt_speed(d.read_speed)
                        << " MB/s) | Write: ("
                        << fmt_speed(d.write_speed)
                        << " MB/s) | " << d.serial_number;

                    if (d.is_external)
                        cout << " Ext ]";
                    else
                        cout << " Int ]";;

                    cout << "\n";
                }

                cout << "\n----------------- DISK PERFORMANCE & DETAILS (Predicted) ---------------\n";

                for (const auto& d : all_disks) {

                    auto fmt_speed = [](const std::string& s) {
                        std::ostringstream tmp;
                        double v = stod(s);
                        tmp << std::fixed << std::setprecision(2) << v;
                        std::string val = tmp.str();

                        int padding = 7 - val.size();
                        if (padding < 0) padding = 0;

                        return std::string(padding, ' ') + val;
                        };

                    cout << d.drive_letter << " [ Read: ("
                        << fmt_speed(d.predicted_read_speed)
                        << " MB/s) | Write: ("
                        << fmt_speed(d.predicted_write_speed)
                        << " MB/s) | " << d.serial_number;

                    if (d.is_external)
                        cout << " Ext ]";
                    else
                        cout << " Int ]";


                    cout << "\n";
                }

                cout << endl;
            }
        }





        // ---------------- Network (compact + any extra) ----------------
        cout << "--- Network Info (Compact + Extra) ---\n";
        cout << "Network Name: " << c_net.get_network_name() << "\n";
        cout << "Network Type: " << c_net.get_network_type() << "\n";
        cout << "IP (compact): " << c_net.get_network_ip() << "\n";
        // If NetworkInfo has more details, print them.
        // Example calls (comment/uncomment based on your API):
        // cout << "Adapters: " << net.list_adapters() << "\n";
        // cout << "Default Gateway: " << net.get_default_gateway() << "\n";
        cout << endl;




        // --- Audio & Power Info ---
        cout << "--- Audio & Power Info ---\n";
        cout << "Audio Devices: " << extra.get_audio_devices() << "\n";
        cout << "Power Status: " << extra.get_power_status() << "\n";
        cout << endl;

        // --- OS Info ---
        cout << "--- OS Info ---\n";
        cout << "Name: " << os.GetOSName() << "\n";
        cout << "Version: " << os.GetOSVersion() << "\n";
        cout << "Architecture: " << os.GetOSArchitecture() << "\n";
        cout << "Kernel: " << os.get_os_kernel_info() << "\n";
        cout << "Uptime: " << os.get_os_uptime() << "\n";
        cout << "Install Date: " << os.get_os_install_date() << "\n";
        cout << "Serial: " << os.get_os_serial_number() << "\n";
        cout << endl;

        // --- CPU Info ---
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

        // --- GPU Info ---
        {
            auto all_gpu_info = obj_gpu.get_all_gpu_info();
            if (all_gpu_info.empty()) {
                cout << "--- GPU Info ---\nNo GPU detected.\n\n";
            }
            else {
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
        }

        // --- Display Info ---
        cout << "--- Display Info ---\n";
        {
            auto monitors = display.get_all_displays();
            if (monitors.empty()) {
                cout << "No monitors detected.\n\n";
            }
            else {
                for (size_t i = 0; i < monitors.size(); ++i) {
                    auto& m = monitors[i];
                    cout << "Monitor " << i + 1 << ":\n";
                    cout << "  Brand: " << m.brand_name << "\n";
                    cout << "  Resolution: " << m.resolution << "\n";
                    cout << "  Refresh Rate: " << m.refresh_rate << " Hz\n";
                    cout << endl;
                }
            }
        }

        // --- BIOS & Motherboard Info ---
        cout << "--- BIOS & Motherboard Info ---\n";
        cout << "Bios Vendor: " << sys.get_bios_vendor() << "\n";
        cout << "Bios Version: " << sys.get_bios_version() << "\n";
        cout << "Bios Date: " << sys.get_bios_date() << "\n";
        cout << "Motherboard Model: " << sys.get_motherboard_model() << "\n";
        cout << "Motherboard Manufacturer: " << sys.get_motherboard_manufacturer() << "\n";
        cout << endl;

        // --- User Info ---
        cout << "--- User Info ---\n";
        cout << "Username: " << user.get_username() << "\n";
        cout << "Computer Name: " << user.get_computer_name() << "\n";
        cout << "Domain: " << user.get_domain_name() << "\n";
        cout << "Groups: " << user.get_user_groups() << "\n";
        cout << endl;

        // --- Performance Info ---
        cout << "--- Performance Info ---\n";
        cout << "System Uptime: " << perf.get_system_uptime() << "\n";
        cout << "CPU Usage: " << perf.get_cpu_usage_percent() << "%\n";
        cout << "RAM Usage: " << perf.get_ram_usage_percent() << "%\n";
        cout << "Disk Usage: " << perf.get_disk_usage_percent() << "%\n";
        cout << "GPU Usage: " << perf.get_gpu_usage_percent() << "%\n";
        cout << endl;



        // ---------------- End FULL INFO SECTION ----------------

        });//---------------ASCII art--------------------------

    cout << endl;

    // No standalone print functions anymore; all output is inside the ASCII art section.
    return 0;
}








/*



#include <iostream>
#include <iomanip>   // for setprecision
#include <vector>
#include <functional>

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

#include "AsciiArt.h"

using namespace std;

// Your existing functions (printStorageInfo, printAudioExtra, etc.) remain the same
void printStorageInfo(StorageInfo& storage) {
    const auto& disks = storage.get_all_storage_info();
    if (disks.empty()) {
        cout << "--- Storage Info ---\nNo drives detected.\n\n";
        return;
    }

    // STORAGE SUMMARY
    cout << "------------------STORAGE SUMMARY---------------------\n";
    for (const auto& d : disks) {
        cout << d.drive_letter << ": (Used) " << d.used_space << " / " << d.total_space
            << " " << d.used_percentage << " - " << d.file_system;

        if (d.is_external)
            cout << " [External]";
        cout << "\n";
    }

    cout << "\n-----------------DISK PERFORMANCE & DETAILS-----------------------\n";
    for (const auto& d : disks) {
        cout << d.drive_letter << ": [ Read: (" << d.read_speed << ") | Write: (" << d.write_speed
            << ") | SN: " << d.serial_number << " ]";

        if (d.is_external)
            cout << " | External";

        cout << "\n";
    }

    cout << "\n-------------DISK PERFORMANCE & DETAILS (Predicted)--------------\n";
    for (const auto& d : disks) {
        cout << d.drive_letter << ": [ Read: (" << d.predicted_read_speed << ") | Write: ("
            << d.predicted_write_speed << ") | SN: " << d.serial_number << " ]";

        if (d.is_external)
            cout << " | External";

        cout << "\n";
    }

    cout << std::endl;
}

void printAudioExtra(ExtraInfo& extra) {
    cout << "--- Audio & Power Info ---\n";
    cout << "Audio Devices: " << extra.get_audio_devices() << "\n";
    cout << "Power Status: " << extra.get_power_status() << "\n";
    cout << endl;
}

void printCPUInfo(CPUInfo& cpu) {
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

void printMemoryInfo(MemoryInfo& ram) {
    cout << "------------- Memory Info -------------\n";
    cout << "(Total: " << ram.get_total_memory() << " GB) ";
    cout << "(Free: " << ram.get_free_memory() << " GB) ";
    cout << "(Used: " << ram.used_ram() << "%)" << endl;
    cout << ram.get_formatted_memory_info() << "\n";
    cout << endl;
}

void printGPUInfo(GPUInfo& obj_gpu, DetailedGPUInfo& detailed_gpu_info) {
    auto all_gpu_info = obj_gpu.get_all_gpu_info();
    if (all_gpu_info.empty()) {
        cout << "--- GPU Info ---\nNo GPU detected.\n\n";
        return;
    }

    cout << "--- GPU Info ---\n";

    // Print part-by-part info for all GPUs
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

    // Detailed primary GPU info (if available)
    auto primary = detailed_gpu_info.primary_gpu_info();
    cout << "Primary GPU Details:\n";
    cout << "  Name: " << primary.name << "\n";
    cout << "  VRAM: " << primary.vram_gb << " GiB\n";
    cout << "  Frequency: " << primary.frequency_ghz << " GHz\n\n";
}

void printOSInfo(OSInfo& os) {
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

void printUserInfo(UserInfo& user) {
    cout << "--- User Info ---\n";
    cout << "Username: " << user.get_username() << "\n";
    cout << "Computer Name: " << user.get_computer_name() << "\n";
    cout << "Domain: " << user.get_domain_name() << "\n";
    cout << "Groups: " << user.get_user_groups() << "\n";
    cout << endl;
}

void printPerformanceInfo(PerformanceInfo& perf) {
    cout << "--- Performance Info ---\n";
    cout << "System Uptime: " << perf.get_system_uptime() << "\n";
    cout << "CPU Usage: " << perf.get_cpu_usage_percent() << "%\n";
    cout << "RAM Usage: " << perf.get_ram_usage_percent() << "%\n";
    cout << "Disk Usage: " << perf.get_disk_usage_percent() << "%\n";
    cout << "GPU Usage: " << perf.get_gpu_usage_percent() << "%\n";
    cout << endl;
}

void printDisplayInfo(DisplayInfo& display)
{
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

void printSystemInfo(SystemInfo& sys) {
    cout << "--- BIOS & Motherboard Info ---\n";
    cout << "Bios Vendor: " << sys.get_bios_vendor() << "\n";
    cout << "Bios Version: " << sys.get_bios_version() << "\n";
    cout << "Bios Date: " << sys.get_bios_date() << "\n";
    cout << "Motherboard Model: " << sys.get_motherboard_model() << "\n";
    cout << "Motherboard Manufacturer: " << sys.get_motherboard_manufacturer() << "\n";
    cout << endl;
}

int main() {

    // Initialize ASCII Art
    AsciiArt art;
    if (!art.loadFromFile("AsciiArt.txt")) {
        cout << "Note: ASCII art not loaded. Showing info only.\n\n";
    }

    // CentralControl removed - no config.json loaded here
    OSInfo os;
    CPUInfo cpu;
    MemoryInfo ram;
    GPUInfo obj_gpu;
    DetailedGPUInfo detailed_gpu_info;
    StorageInfo storage;
    NetworkInfo net;
    UserInfo user;
    PerformanceInfo perf;
    DisplayInfo display;
    ExtraInfo extra;
    SystemInfo sys;

    CompactAudio c_audio;
    CompactOS c_os;
    CompactCPU c_cpu;
    CompactScreen c_screen;
    CompactMemory c_memory;
    CompactSystem c_system;
    CompactGPU c_gpu;
    CompactPerformance c_perf;
    CompactUser c_user;
    CompactNetwork c_net;
    DiskInfo disk;

    // Use ASCII art with your existing compact output
    //--------------ascii art layout------------------------------------
    art.printWithArt([&]() {

        cout << endl;

        cout << "[OS]  -> " << c_os.getOSName()
            << c_os.getOSBuild()
            << " (" << c_os.getArchitecture() << ")"
            << " (uptime: " << c_os.getUptime() << ")" << endl;

        cout << "[CPU] -> " << c_cpu.getCPUName() << " ("
            << c_cpu.getCPUCores() << "C/"
            << c_cpu.getCPUThreads() << "T)"
            << std::fixed << std::setprecision(2)
            << " @ " << c_cpu.getClockSpeed() << " GHz " << endl;

        auto screens = c_screen.get_screens();
        int i = 1;
        for (const auto& s : screens) {
            cout << "[Display " << i++ << "] -> "
                << s.brand_name << " (" << s.resolution << ") @"
                << s.refresh_rate << "Hz\n";
        }

        cout << "[Memory] -> " << "(total: " << c_memory.get_total_memory() << " GB)"
            << " (free: " << c_memory.get_free_memory() << " GB)"
            << " ( " << c_memory.get_used_memory_percent() << "% ) " << endl;

        cout << "[Audio Input] -> " << c_audio.active_audio_input()
            << c_audio.active_audio_input_status() << endl;

        cout << "[Audio Output] -> " << c_audio.active_audio_output()
            << c_audio.active_audio_output_status() << endl;

        cout << "[BIOS] -> " << c_system.getBIOSInfo() << endl;
        cout << "[Motherboard] -> " << c_system.getMotherboardInfo() << endl;

        cout << "[GPU] -> " << c_gpu.getGPUName()
            << " (" << c_gpu.getGPUUsagePercent() << "%)"
            << " (" << c_gpu.getVRAMGB() << " GB)"
            << " (@" << c_gpu.getGPUFrequency() << ") " << endl;

        cout << "[Performance] -> "
            << "(CPU: " << c_perf.getCPUUsage() << "%) "
            << "(GPU: " << c_perf.getGPUUsage() << "%) "
            << "(RAM: " << c_perf.getRAMUsage() << "%) "
            << "(Disk: " << c_perf.getDiskUsage() << "%) " << endl;

        cout << "[User] -> @" << c_user.getUsername()
            << " -> (Domain: " << c_user.getDomain()
            << ") -> (Type: " << c_user.isAdmin() << ")"<< endl;

        cout << "[network] -> " << "(Name: " << c_net.get_network_name()
            << ") (Type: " << c_net.get_network_type()
            << ") (ip: " << c_net.get_network_ip() << ") " << endl;

        auto disks = disk.getAllDiskUsage();

        // print disk usage in percentage---------------------------------
        std::cout << "[Disk] -> ";
        for (const auto& d : disks) {
            std::cout << "(" << d.first[0] << ": "
                << std::fixed << std::setprecision(1)
                << d.second << "%) ";
        }
        std::cout << std::endl;
        // print disk capacity in GB---------------------------------------
        auto caps = disk.getDiskCapacity();
        std::cout << "[Disk Cap] -> ";
        for (const auto& c : caps) {
            std::cout << "(" << c.first[0] << "-" << c.second << "GB)";
        }
        std::cout << std::endl;

        cout << "------------- Memory Info -------------\n";
        cout << "(Total: " << ram.get_total_memory() << " GB) ";
        cout << "(Free: " << ram.get_free_memory() << " GB) ";
        cout << "(Used: " << ram.used_ram() << "%)" << endl;
        cout << ram.get_formatted_memory_info() << "\n";
        cout << endl;



        });//---------------ASCII art--------------------------

    cout << endl;

    // Unconditional prints (no central control) - EXACTLY AS BEFORE
    printMemoryInfo(ram);
    printAudioExtra(extra);
    printOSInfo(os);
    printCPUInfo(cpu);
    printGPUInfo(obj_gpu, detailed_gpu_info);
    printDisplayInfo(display);
    printSystemInfo(sys);
    printUserInfo(user);
    printPerformanceInfo(perf);
    //printStorageInfo(storage);
    return 0;
}



*/

/*


#include <iostream>
#include <iomanip>   // for setprecision
#include <vector>

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

#include "AsciiArt.h"

using namespace std;

// NOTE: CentralControl removed  all prints are unconditional (show everything available).

void printStorageInfo(StorageInfo& storage) {
    const auto& disks = storage.get_all_storage_info();
    if (disks.empty()) {
        cout << "--- Storage Info ---\nNo drives detected.\n\n";
        return;
    }

    // STORAGE SUMMARY
    cout << "------------------STORAGE SUMMARY---------------------\n";
    for (const auto& d : disks) {
        cout << d.drive_letter << ": (Used) " << d.used_space << " / " << d.total_space
            << " " << d.used_percentage << " - " << d.file_system;

        if (d.is_external)
            cout << " [External]";
        cout << "\n";
    }

    cout << "\n-----------------DISK PERFORMANCE & DETAILS-----------------------\n";
    for (const auto& d : disks) {
        cout << d.drive_letter << ": [ Read: (" << d.read_speed << ") | Write: (" << d.write_speed
            << ") | SN: " << d.serial_number << " ]";

        if (d.is_external)
            cout << " | External";

        cout << "\n";
    }

    cout << "\n-------------DISK PERFORMANCE & DETAILS (Predicted)--------------\n";
    for (const auto& d : disks) {
        cout << d.drive_letter << ": [ Read: (" << d.predicted_read_speed << ") | Write: ("
            << d.predicted_write_speed << ") | SN: " << d.serial_number << " ]";

        if (d.is_external)
            cout << " | External";

        cout << "\n";
    }

    cout << std::endl;
}

void printAudioExtra(ExtraInfo& extra) {
    cout << "--- Audio & Power Info ---\n";
    cout << "Audio Devices: " << extra.get_audio_devices() << "\n";
    cout << "Power Status: " << extra.get_power_status() << "\n";
    cout << endl;
}

void printCPUInfo(CPUInfo& cpu) {
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

void printMemoryInfo(MemoryInfo& ram) {
    cout << "------------- Memory Info -------------\n";
    cout << "(Total: " << ram.get_total_memory() << " GB) ";
    cout << "(Free: " << ram.get_free_memory() << " GB) ";
    cout << "(Used: " << ram.used_ram() << "%)" << endl;
    cout << ram.get_formatted_memory_info() << "\n";
    cout << endl;
}

void printGPUInfo(GPUInfo& obj_gpu, DetailedGPUInfo& detailed_gpu_info) {
    auto all_gpu_info = obj_gpu.get_all_gpu_info();
    if (all_gpu_info.empty()) {
        cout << "--- GPU Info ---\nNo GPU detected.\n\n";
        return;
    }

    cout << "--- GPU Info ---\n";

    // Print part-by-part info for all GPUs
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

    // Detailed primary GPU info (if available)
    auto primary = detailed_gpu_info.primary_gpu_info();
    cout << "Primary GPU Details:\n";
    cout << "  Name: " << primary.name << "\n";
    cout << "  VRAM: " << primary.vram_gb << " GiB\n";
    cout << "  Frequency: " << primary.frequency_ghz << " GHz\n\n";
}

void printOSInfo(OSInfo& os) {
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

void printUserInfo(UserInfo& user) {
    cout << "--- User Info ---\n";
    cout << "Username: " << user.get_username() << "\n";
    cout << "Computer Name: " << user.get_computer_name() << "\n";
    cout << "Domain: " << user.get_domain_name() << "\n";
    cout << "Groups: " << user.get_user_groups() << "\n";
    cout << endl;
}

void printPerformanceInfo(PerformanceInfo& perf) {
    cout << "--- Performance Info ---\n";
    cout << "System Uptime: " << perf.get_system_uptime() << "\n";
    cout << "CPU Usage: " << perf.get_cpu_usage_percent() << "%\n";
    cout << "RAM Usage: " << perf.get_ram_usage_percent() << "%\n";
    cout << "Disk Usage: " << perf.get_disk_usage_percent() << "%\n";
    cout << "GPU Usage: " << perf.get_gpu_usage_percent() << "%\n";
    cout << endl;
}

void printDisplayInfo(DisplayInfo& display)
{
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

void printSystemInfo(SystemInfo& sys) {
    cout << "--- BIOS & Motherboard Info ---\n";
    cout << "Bios Vendor: " << sys.get_bios_vendor() << "\n";
    cout << "Bios Version: " << sys.get_bios_version() << "\n";
    cout << "Bios Date: " << sys.get_bios_date() << "\n";
    cout << "Motherboard Model: " << sys.get_motherboard_model() << "\n";
    cout << "Motherboard Manufacturer: " << sys.get_motherboard_manufacturer() << "\n";
    cout << endl;
}

int main() {



    // CentralControl removed - no config.json loaded here
    OSInfo os;
    CPUInfo cpu;
    MemoryInfo ram;
    GPUInfo obj_gpu;
    DetailedGPUInfo detailed_gpu_info;
    StorageInfo storage;
    NetworkInfo net;
    UserInfo user;
    PerformanceInfo perf;
    DisplayInfo display;
    ExtraInfo extra;
    SystemInfo sys;

    CompactAudio c_audio;
    CompactOS c_os;
    CompactCPU c_cpu;
    CompactScreen c_screen;
    CompactMemory c_memory;
    CompactSystem c_system;
    CompactGPU c_gpu;
    CompactPerformance c_perf;
    CompactUser c_user;
    CompactNetwork c_net;

    cout << "[OS]  -> " << c_os.getOSName()
        << c_os.getOSBuild()
        << " (" << c_os.getArchitecture() << ")"
        << " (uptime: " << c_os.getUptime() << ")" << endl;

    cout << "[CPU] -> " << c_cpu.getCPUName() << " ("
        << c_cpu.getCPUCores() << "C/"
        << c_cpu.getCPUThreads() << "T)"
        << std::fixed << std::setprecision(2)
        << " @ " << c_cpu.getClockSpeed() << " GHz " << endl;

    auto screens = c_screen.get_screens();
    int i = 1;
    for (const auto& s : screens) {
        cout << "[Display " << i++ << "] -> "
            << s.brand_name << " (" << s.resolution << ") @"
            << s.refresh_rate << "Hz\n";
    }

    cout << "[Memory] -> " << "(total: " << c_memory.get_total_memory() << " GB)"
        << " (free: " << c_memory.get_free_memory() << " GB)"
        << " ( " << c_memory.get_used_memory_percent() << "% ) " << endl;

    cout << "[Audio Input] -> " << c_audio.active_audio_input()
        << c_audio.active_audio_input_status() << endl;

    cout << "[Audio Output] -> " << c_audio.active_audio_output()
        << c_audio.active_audio_output_status() << endl;

    cout << "[BIOS] -> " << c_system.getBIOSInfo() << endl;
    cout << "[Motherboard] -> " << c_system.getMotherboardInfo() << endl;

    cout << "[GPU] -> " << c_gpu.getGPUName()
        << " (" << c_gpu.getGPUUsagePercent() << "%)"
        << " (" << c_gpu.getVRAMGB() << " GB)"
        << " (@" << c_gpu.getGPUFrequency() << ") " << endl;

    cout << "[Performance] -> "
        << "(CPU: " << c_perf.getCPUUsage() << "%) "
        << "(GPU: " << c_perf.getGPUUsage() << "%) "
        << "(RAM: " << c_perf.getRAMUsage() << "%) "
        << "(Disk: " << c_perf.getDiskUsage() << "%) " << endl;

    cout << "[User] -> @" << c_user.getUsername()
        << " -> (Domain: " << c_user.getDomain()
        << ") -> ( Admin: " << c_user.isAdmin() << endl;

    cout << "[network] -> " << "(Name: " << c_net.get_network_name()
        << ") (Type: " << c_net.get_network_type()
        << ") (ip: " << c_net.get_network_ip() << ") " << endl;

    cout << endl;

    // Unconditional prints (no central control)
    printMemoryInfo(ram);
    printAudioExtra(extra);
    printOSInfo(os);
    printCPUInfo(cpu);
    printGPUInfo(obj_gpu, detailed_gpu_info);
    printDisplayInfo(display);
    printSystemInfo(sys);
    printUserInfo(user);
    printPerformanceInfo(perf);
    printStorageInfo(storage);
    return 0;
}





*/



/*

2nd main.cpp

#include <iostream>

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
#include "CentralControl.h"

#include "CompactAudio.h"
#include "CompactOS.h"
#include "CompactCPU.h"
#include "CompactMemory.h"
#include "CompactScreen.h"
#include "CompactMemory.h"
#include "CompactSystem.h"
#include "CompactGPU.h"
#include "CompactPerformance.h"
#include "CompactUser.h"
#include "CompactNetwork.h"

using namespace std;
void printStorageInfo(StorageInfo& storage, CentralControl& cc) {
    if (!cc.isModuleEnabled("StorageInfo")) return;

    const auto& disks = storage.get_all_storage_info();
    if (disks.empty()) {
        std::cout << "--- Storage Info ---\nNo drives detected.\n\n";
        return;
    }

    // STORAGE SUMMARY
    std::cout << "------------------STORAGE SUMMARY---------------------\n";
    for (const auto& d : disks) {
        if (!(cc.isSubModuleEnabled("StorageInfo", "fields", "drive_letter") &&
            cc.isSubModuleEnabled("StorageInfo", "fields", "used_space") &&
            cc.isSubModuleEnabled("StorageInfo", "fields", "total_space") &&
            cc.isSubModuleEnabled("StorageInfo", "fields", "used_percentage") &&
            cc.isSubModuleEnabled("StorageInfo", "fields", "file_system"))) {
            continue;
        }

        std::cout << d.drive_letter << ": (Used) " << d.used_space << " / " << d.total_space
            << " " << d.used_percentage << " - " << d.file_system;

        if (cc.isSubModuleEnabled("StorageInfo", "fields", "is_external") && d.is_external)
            std::cout << "[External]";
        std::cout << "\n";
    }

    std::cout << "\n-----------------DISK PERFORMANCE & DETAILS-----------------------\n";
    for (const auto& d : disks) {
        if (!(cc.isSubModuleEnabled("StorageInfo", "fields", "read_speed") &&
            cc.isSubModuleEnabled("StorageInfo", "fields", "write_speed") &&
            cc.isSubModuleEnabled("StorageInfo", "fields", "serial_number")))
            continue;

        std::cout << d.drive_letter << ": [ Read: (" << d.read_speed << ") | Write: (" << d.write_speed
            << ") | SN: " << d.serial_number << " ]";

        if (cc.isSubModuleEnabled("StorageInfo", "fields", "is_external") && d.is_external)
            std::cout << " | External ]";

        std::cout << "\n";
    }

    std::cout << "\n-------------DISK PERFORMANCE & DETAILS (Predicted)--------------\n";
    for (const auto& d : disks) {
        if (!(cc.isSubModuleEnabled("StorageInfo", "fields", "predicted_read_speed") &&
            cc.isSubModuleEnabled("StorageInfo", "fields", "predicted_write_speed") &&
            cc.isSubModuleEnabled("StorageInfo", "fields", "serial_number")))
            continue;

        std::cout << d.drive_letter << ": [ Read: (" << d.predicted_read_speed << ") | Write: ("
            << d.predicted_write_speed << ") | SN: " << d.serial_number << " ]";

        if (cc.isSubModuleEnabled("StorageInfo", "fields", "is_external") && d.is_external)
            std::cout << " | External ]";

        std::cout << "\n";
    }

    std::cout << std::endl;
}



// -------- Print Functions with Submodule Control --------

void printAudioExtra(ExtraInfo& extra, CentralControl& cc) {
    if (!cc.isModuleEnabled("ExtraInfo")) return;

    cout << "--- Audio & Power Info ---\n";
    if (cc.isSubModuleEnabled("ExtraInfo", "audio_devices"))
        cout << "Audio Devices: " << extra.get_audio_devices() << "\n";
    if (cc.isSubModuleEnabled("ExtraInfo", "power_status"))
        cout << "Power Status: " << extra.get_power_status() << "\n";
    cout << endl;
}

void printCPUInfo(CPUInfo& cpu, CentralControl& cc) {
    if (!cc.isModuleEnabled("CPUInfo")) return;

    cout << "--- CPU Info ---\n";
    if (cc.isSubModuleEnabled("CPUInfo", "brand"))
        cout << "Brand: " << cpu.get_cpu_info() << "\n";
    if (cc.isSubModuleEnabled("CPUInfo", "utilization"))
        cout << "Utilization: " << cpu.get_cpu_utilization() << "%\n";
    if (cc.isSubModuleEnabled("CPUInfo", "speed"))
        cout << "Speed: " << cpu.get_cpu_speed() << "\n";
    if (cc.isSubModuleEnabled("CPUInfo", "base_speed"))
        cout << "Base Speed: " << cpu.get_cpu_base_speed() << "\n";
    if (cc.isSubModuleEnabled("CPUInfo", "cores"))
        cout << "Cores: " << cpu.get_cpu_cores() << "\n";
    if (cc.isSubModuleEnabled("CPUInfo", "logical_processors"))
        cout << "Logical Processors: " << cpu.get_cpu_logical_processors() << "\n";
    if (cc.isSubModuleEnabled("CPUInfo", "sockets"))
        cout << "Sockets: " << cpu.get_cpu_sockets() << "\n";
    if (cc.isSubModuleEnabled("CPUInfo", "virtualization"))
        cout << "Virtualization: " << cpu.get_cpu_virtualization() << "\n";

    if (cc.isSubModuleEnabled("CPUInfo", "cache", "l1"))
        cout << "L1 Cache: " << cpu.get_cpu_l1_cache() << "\n";
    if (cc.isSubModuleEnabled("CPUInfo", "cache", "l2"))
        cout << "L2 Cache: " << cpu.get_cpu_l2_cache() << "\n";
    if (cc.isSubModuleEnabled("CPUInfo", "cache", "l3"))
        cout << "L3 Cache: " << cpu.get_cpu_l3_cache() << "\n";

    cout << endl;
}

void printMemoryInfo(MemoryInfo& ram, CentralControl& cc) {
    if (!cc.isModuleEnabled("MemoryInfo")) return;

    cout << "------------- Memory Info -------------\n";
    if (cc.isSubModuleEnabled("MemoryInfo", "total_memory"))
        cout << "(Total: " << ram.get_total_memory() << " GB) ";
    if (cc.isSubModuleEnabled("MemoryInfo", "free_memory"))
        cout << "(Free: " << ram.get_free_memory() << " GB) ";
    if (cc.isSubModuleEnabled("MemoryInfo", "used_percentage"))
        cout << "(Used: " << ram.used_ram() << "%)" << endl;
    if (cc.isSubModuleEnabled("MemoryInfo", "formatted_memory_info"))
        cout << ram.get_formatted_memory_info() << "\n";
    cout << endl;
}

void printGPUInfo(GPUInfo& obj_gpu, DetailedGPUInfo& detailed_gpu_info, CentralControl& cc) {
    if (!cc.isModuleEnabled("GPUInfo")) return;

    auto all_gpu_info = obj_gpu.get_all_gpu_info();
    if (all_gpu_info.empty()) {
        cout << "--- GPU Info ---\nNo GPU detected.\n\n";
        return;
    }

    cout << "--- GPU Info ---\n";

    bool part_by_part = cc.isSubModuleEnabled("GPUInfo", "part_by_part_info");
    bool detailed_mode = cc.isSubModuleEnabled("GPUInfo", "detailed_gpu_info");

    for (size_t i = 0; i < all_gpu_info.size(); ++i) {
        auto& g = all_gpu_info[i];
        if (part_by_part) {
            cout << "GPU " << i + 1 << ":\n";
            if (cc.isSubModuleEnabled("GPUInfo", "fields", "gpu_name"))
                cout << "  Name: " << g.gpu_name << "\n";
            if (cc.isSubModuleEnabled("GPUInfo", "fields", "gpu_memory"))
                cout << "  Memory: " << g.gpu_memory << "\n";
            if (cc.isSubModuleEnabled("GPUInfo", "fields", "gpu_usage"))
                cout << "  Usage: " << g.gpu_usage << "%\n";
            if (cc.isSubModuleEnabled("GPUInfo", "fields", "gpu_vendor"))
                cout << "  Vendor: " << g.gpu_vendor << "\n";
            if (cc.isSubModuleEnabled("GPUInfo", "fields", "gpu_driver_version"))
                cout << "  Driver Version: " << g.gpu_driver_version << "\n";
            if (cc.isSubModuleEnabled("GPUInfo", "fields", "gpu_temperature"))
                cout << "  Temperature: " << g.gpu_temperature << " C\n";
            if (cc.isSubModuleEnabled("GPUInfo", "fields", "gpu_core_count"))
                cout << "  Core Count: " << g.gpu_core_count << "\n";
            cout << endl;
        }
    }

    if (detailed_mode) {
        auto primary = detailed_gpu_info.primary_gpu_info();
        cout << "Primary GPU Details:\n";
        cout << "  Name: " << primary.name << "\n";
        cout << "  VRAM: " << primary.vram_gb << " GiB\n";
        cout << "  Frequency: " << primary.frequency_ghz << " GHz\n\n";
    }
}

void printOSInfo(OSInfo& os, CentralControl& cc) {
    if (!cc.isModuleEnabled("OSInfo")) return;

    cout << "--- OS Info ---\n";
    if (cc.isSubModuleEnabled("OSInfo", "name"))
        cout << "Name: " << os.GetOSName() << "\n";
    if (cc.isSubModuleEnabled("OSInfo", "version"))
        cout << "Version: " << os.GetOSVersion() << "\n";
    if (cc.isSubModuleEnabled("OSInfo", "architecture"))
        cout << "Architecture: " << os.GetOSArchitecture() << "\n";
    if (cc.isSubModuleEnabled("OSInfo", "kernel_version"))
        cout << "Kernel: " << os.get_os_kernel_info() << "\n";
    if (cc.isSubModuleEnabled("OSInfo", "uptime"))
        cout << "Uptime: " << os.get_os_uptime() << "\n";
    if (cc.isSubModuleEnabled("OSInfo", "install_date"))
        cout << "Install Date: " << os.get_os_install_date() << "\n";
    if (cc.isSubModuleEnabled("OSInfo", "serial_number"))
        cout << "Serial: " << os.get_os_serial_number() << "\n";
    cout << endl;
}

void printUserInfo(UserInfo& user, CentralControl& cc) {
    if (!cc.isModuleEnabled("UserInfo")) return;

    cout << "--- User Info ---\n";
    if (cc.isSubModuleEnabled("UserInfo", "username"))
        cout << "Username: " << user.get_username() << "\n";
    if (cc.isSubModuleEnabled("UserInfo", "computer_name"))
        cout << "Computer Name: " << user.get_computer_name() << "\n";
    if (cc.isSubModuleEnabled("UserInfo", "domain_name"))
        cout << "Domain: " << user.get_domain_name() << "\n";
    if (cc.isSubModuleEnabled("UserInfo", "user_groups"))
        cout << "Groups: " << user.get_user_groups() << "\n";
    cout << endl;
}

void printPerformanceInfo(PerformanceInfo& perf, CentralControl& cc) {
    if (!cc.isModuleEnabled("PerformanceInfo")) return;

    cout << "--- Performance Info ---\n";
    if (cc.isSubModuleEnabled("PerformanceInfo", "uptime"))
        cout << "System Uptime: " << perf.get_system_uptime() << "\n";
    if (cc.isSubModuleEnabled("PerformanceInfo", "cpu_usage"))
        cout << "CPU Usage: " << perf.get_cpu_usage_percent() << "%\n";
    if (cc.isSubModuleEnabled("PerformanceInfo", "ram_usage"))
        cout << "RAM Usage: " << perf.get_ram_usage_percent() << "%\n";
    if (cc.isSubModuleEnabled("PerformanceInfo", "disk_usage"))
        cout << "Disk Usage: " << perf.get_disk_usage_percent() << "%\n";
    if (cc.isSubModuleEnabled("PerformanceInfo", "gpu_usage"))
        cout << "GPU Usage: " << perf.get_gpu_usage_percent() << "%\n";
    cout << endl;
}

void printDisplayInfo(DisplayInfo& display, CentralControl& cc)
{
    if (!cc.isModuleEnabled("DisplayInfo")) return;

    cout << "--- Display Info ---\n";
    auto monitors = display.get_all_displays();
    if (monitors.empty()) {
        cout << "No monitors detected.\n\n";
        return;
    }

    for (size_t i = 0; i < monitors.size(); ++i) {
        auto& m = monitors[i];
        cout << "Monitor " << i + 1 << ":\n";

        // Show brand
        if (cc.isSubModuleEnabled("DisplayInfo", "show_brand"))
            cout << "  Brand: " << m.brand_name << "\n";

        if (cc.isSubModuleEnabled("DisplayInfo", "show_resolution"))
            cout << "  Resolution: " << m.resolution << "\n";

        if (cc.isSubModuleEnabled("DisplayInfo", "show_refresh_rate"))
            cout << "  Refresh Rate: " << m.refresh_rate << " Hz\n";

        cout << endl;
    }
}


void printSystemInfo(SystemInfo& sys, CentralControl& cc) {
    if (!cc.isModuleEnabled("SystemInfo")) return;

    cout << "--- BIOS & Motherboard Info ---\n";
    if (cc.isSubModuleEnabled("SystemInfo", "bios", "vendor"))
        cout << "Bios Vendor: " << sys.get_bios_vendor() << "\n";
    if (cc.isSubModuleEnabled("SystemInfo", "bios", "version"))
        cout << "Bios Version: " << sys.get_bios_version() << "\n";
    if (cc.isSubModuleEnabled("SystemInfo", "bios", "date"))
        cout << "Bios Date: " << sys.get_bios_date() << "\n";
    if (cc.isSubModuleEnabled("SystemInfo", "motherboard", "model"))
        cout << "Motherboard Model: " << sys.get_motherboard_model() << "\n";
    if (cc.isSubModuleEnabled("SystemInfo", "motherboard", "manufacturer"))
        cout << "Motherboard Manufacturer: " << sys.get_motherboard_manufacturer() << "\n";
    cout << endl;
}

// -------- MAIN --------
int main() {
    CentralControl central("config.json");

    OSInfo os;
    CPUInfo cpu;
    MemoryInfo ram;
    GPUInfo obj_gpu;
    DetailedGPUInfo detailed_gpu_info;
    StorageInfo storage;
    NetworkInfo net;
    UserInfo user;
    PerformanceInfo perf;
    DisplayInfo display;
    ExtraInfo extra;
    SystemInfo sys;

    CompactAudio c_audio;
    CompactOS c_os;
    CompactCPU c_cpu;
    CompactScreen c_screen;
    CompactMemory c_memory;
    CompactSystem c_system;
    CompactGPU c_gpu;
    CompactPerformance c_perf;
    CompactUser c_user;
    CompactNetwork c_net;




    cout << "[OS]  -> " << c_os.getOSName()
         << c_os.getOSBuild()
         << "(" << c_os.getArchitecture() << ")"
         << "(uptime: " <<c_os.getUptime() << ")" <<  endl;

    cout << "[CPU] -> " << c_cpu.getCPUName() << " ("
        << c_cpu.getCPUCores() << "C/"
        << c_cpu.getCPUThreads() << "T)"
        << std::fixed << std::setprecision(2)
        << " @ " << c_cpu.getClockSpeed() << " GHz " << endl;

       // << "(" << c_cpu.getUsagePercent() << "%)" << endl;


    auto screens = c_screen.get_screens();
    int i = 1;
    for (const auto& s : screens) {
    cout << "[Display " << i++ << "] -> "
         << s.brand_name << " (" << s.resolution << ") @"
         << s.refresh_rate << "Hz\n";
    }

    cout << "[Memory] -> " << "(total: " << c_memory.get_total_memory() << " GB)"
        << " (free: " << c_memory.get_free_memory() << " GB)"
        << " ( " << c_memory.get_used_memory_percent() << "% ) " << endl;

    cout << "[Audio Input] -> " << c_audio.active_audio_input()
        << c_audio.active_audio_input_status() << endl ;

    cout << "[Audio Output] -> " << c_audio.active_audio_output()
        << c_audio.active_audio_output_status() << endl;

    cout << "[BIOS] -> " << c_system.getBIOSInfo() << endl;
    cout << "[Motherboard] -> " << c_system.getMotherboardInfo() << endl;

    cout << "[GPU] - > " << c_gpu.getGPUName()
         << " ("   << c_gpu.getGPUUsagePercent() << "%)"
         << " ("   << c_gpu.getVRAMGB() <<" GB)"
         << " (@" << c_gpu.getGPUFrequency() << ") " << endl;

    cout << "[Performance] -> "
         << "(CPU: " << c_perf.getCPUUsage() << "%) "
         << "(GPU: " << c_perf.getGPUUsage() << "%) "
         << "(RAM: " << c_perf.getRAMUsage() << "%) "
         << "(Disk: " << c_perf.getDiskUsage() << "%) "; cout << endl;

    cout << "[User] -> @" << c_user.getUsername()
        << " -> (Domain: " << c_user.getDomain()
        << ") -> ( " << c_user.isAdmin() << " )";
    cout << endl;



    cout << "[network] -> " << "(Name: " << c_net.get_network_name()
        << ") (Type: " << c_net.get_network_type()
        << ") (ip: " << c_net.get_network_ip() << ") " << endl;

    cout << endl ;

    printStorageInfo(storage, central);
    printMemoryInfo(ram, central);
    printAudioExtra(extra, central);
    printOSInfo(os, central);
    printCPUInfo(cpu, central);
    printGPUInfo(obj_gpu, detailed_gpu_info, central);
    printDisplayInfo(display, central);
    printSystemInfo(sys, central);
    printUserInfo(user, central);
    printPerformanceInfo(perf, central);




    return 0;
}




*/


/* previous main.cpp---------------------------------------ignore for

#include <iostream>
#include "OSInfo.h"
#include "CPUInfo.h"
#include "MemoryInfo.h"
#include "GPUInfo.h"
#include "StorageInfo.h"
#include "NetworkInfo.h"
#include "DetailedGPUInfo.h"
#include <iomanip>
#include "PerformanceInfo.h"
#include "UserInfo.h"
#include "SystemInfo.h"
#include "DisplayInfo.h"
#include "ExtraInfo.h"
#include "CentralControl.h"
using namespace std;

int main() {
    OSInfo os;
    CPUInfo cpu;
    MemoryInfo ram;
    GPUInfo obj_gpu;
    vector<gpu_data> all_gpu_info = obj_gpu.get_all_gpu_info();
    StorageInfo storage;
    vector<storage_data> disks = storage.get_all_storage_info();
    NetworkInfo net;
    DetailedGPUInfo detailed_gpu_info;
    PerformanceInfo perf;
    UserInfo user;
    SystemInfo sys;
    DisplayInfo display;
    ExtraInfo extra;

    CentralControl central("config.json"); // load config files








    // Audio Devices
    cout << extra.get_audio_devices() << endl;
    cout << extra.get_power_status() << endl;


    vector<MonitorInfo> monitors = display.get_all_displays();
    if (monitors.empty())
    {
        cout << "No monitors detected." << endl;
    }
    for (size_t i = 0; i < monitors.size(); ++i)
    {
        cout << "Monitor " << i + 1 << ":" ;
        cout << "  Resolution: " << monitors[i].resolution;
        cout << "  Refresh Rate: " << monitors[i].refresh_rate << " Hz" << endl;
    }
    cout << endl << endl;





    cout << "=== BIOS & Motherboard Info ===" << endl;
    cout << "Bios Vendor: " << sys.get_bios_vendor() << endl;
    cout << "Bios Version: " << sys.get_bios_version() << endl;
    cout << "Bios Date: " << sys.get_bios_date() << endl;
    cout << "Motherboard Model: " << sys.get_motherboard_model() << endl;
    cout << "Motherboard Manufacturer: " << sys.get_motherboard_manufacturer() << endl;
    cout << endl << endl;



    //print userinfo-------------------------------------------------------------
    cout << "Username       : " << user.get_username() << endl;
    cout << "Computer Name  : " << user.get_computer_name() << endl;
    cout << "Domain Name    : " << user.get_domain_name() << endl;
    cout << "User Groups    : " << user.get_user_groups() << endl;
    cout << endl << endl;

   //print performanceinfo------------------------------------------------------
   ; cout << "Uptime: " << perf.get_system_uptime() << "\n";
    //std::cout << "CPU: " << std::fixed << std::setprecision(2) << perf.get_cpu_usage_percent() << " %\n";
    std::cout << "RAM: " << perf.get_ram_usage_percent() << " %\n";
    std::cout << "Disk: " << perf.get_disk_usage_percent() << " %\n";
    std::cout << "GPU: " << perf.get_gpu_usage_percent() << " %\n";
    cout << endl << endl;



    //print gpu info -------------------------------------------------------------
   // there're possiblities of having multiple gpus...loop requred
    for (size_t gpu_loop = 0; gpu_loop < all_gpu_info.size(); ++gpu_loop)
    {
        cout << "GPU " << gpu_loop << " Name: " << all_gpu_info[gpu_loop].gpu_name << endl;
        cout << "GPU " << gpu_loop << " Memory: " << all_gpu_info[gpu_loop].gpu_memory << endl;
        cout << "GPU " << gpu_loop << " Usage: " << all_gpu_info[gpu_loop].gpu_usage << endl;
        cout << "GPU " << gpu_loop << " Vendor: " << all_gpu_info[gpu_loop].gpu_vendor << endl;
        cout << "GPU " << gpu_loop << " Driver Version: " << all_gpu_info[gpu_loop].gpu_driver_version << endl;
        cout << "GPU " << gpu_loop << " Temperature: " << all_gpu_info[gpu_loop].gpu_temperature << endl;
        cout << "GPU " << gpu_loop << " Core Count:" << all_gpu_info[gpu_loop].gpu_core_count << endl;
        cout << endl;
    }
    cout << endl << endl;
    //print memory infos----------------------------------------------------------
    //cout << endl << endl;
    cout << "Total Memory : " << ram.get_total_memory() << " GB" << endl;
    cout << "Used Memory : " << ram.get_free_memory() << " GB" << endl;
    cout << "Memory Used In Percentage : " << ram.used_ram() << "%" << endl;
    cout << ram.get_formatted_memory_info() << endl << endl;

    //print os infos-------------------------------------------------------------
    cout << "OS Version: " << os.GetOSVersion() << std::endl;
    cout << "OS Architecture: " << os.GetOSArchitecture() << std::endl;
    cout << "OS Name: " << os.GetOSName() << std::endl;
    cout << "OS Kernel Version: " << os.get_os_kernel_info() << endl;
    cout << "os UP time: " << os.get_os_uptime() << endl;
    cout << "os installaton date: " << os.get_os_install_date() << endl;
    cout << "os serial number:" << os.get_os_serial_number() << endl;

    //get gpu info in different style--------------------------------------------

    auto primary = detailed_gpu_info.primary_gpu_info();
    cout << "GPU 0 : " << primary.name
        << " @ " << primary.frequency_ghz << " GHz ( "
        << primary.vram_gb << " GIB)" << endl;

    auto all = detailed_gpu_info.get_all_gpus();
    cout << "GPU 0 : " << primary.name
        << " @ " << primary.frequency_ghz << " GHz ( "
        << primary.vram_gb << " GIB)";
    cout << endl << endl;


    //print cpu infos-------------------------------------------------------------

    cout << "CPU Brand: " << cpu.get_cpu_info() << endl;
    cout << "Utilization: " << cpu.get_cpu_utilization() << "%" << endl;
    cout << "Speed: " << cpu.get_cpu_speed() << endl;
    cout << "Base Speed: " << cpu.get_cpu_base_speed() << endl;
    cout << "Sockets: " << cpu.get_cpu_sockets() << endl;
    cout << "Cores: " << cpu.get_cpu_cores() << endl;
    cout << "Logical Processors: " << cpu.get_cpu_logical_processors() << endl;
    cout << "Virtualization: " << cpu.get_cpu_virtualization() << endl;
    cout << "L1 Cache: " << cpu.get_cpu_l1_cache() << endl;
    cout << "L2 Cache: " << cpu.get_cpu_l2_cache() << endl;
    cout << "L3 Cache: " << cpu.get_cpu_l3_cache() << endl;
    // cout << "Up Time: " << cpu.get_system_uptime() << endl;
    // cout << "Processes: " << cpu.get_process_count() << endl;
    // cout << "Threads: " << cpu.get_thread_count() << endl;
    // cout << "Handles: " << cpu.get_handle_count() << endl;
    cout << endl << endl;

    // --- Part 1: Disk Space Summary ---
    cout << "\n[ STORAGE SUMMARY ]\n";
    for (auto& d : disks) {
        cout << d.drive_letter << ": (Used) " << d.used_space
            << " / " << d.total_space << " " << d.used_percentage
            << " - " << d.file_system;
        if (d.is_external) cout << "[External]";
        cout << endl;
    }
    cout << endl << endl;

    // --- Part 2: Accurate Disk Performance ---
    cout << "[ DISK PERFORMANCE & DETAILS ]\n";
    for (auto& d : disks) {
        cout << d.drive_letter << ": [ Read: (" << d.read_speed
            << ") | Write: (" << d.write_speed << ") | SN: "
            << d.serial_number;
        if (d.is_external) cout << " | External";
        cout << " ]" << endl;
    }
    cout << endl << endl;

    // --- Part 3: Predicted Disk Performance ---
    cout << "[ DISK PERFORMANCE & DETAILS (Predicted) ]\n";
    for (auto& d : disks) {
        cout << d.drive_letter << ": [ Read: (" << d.predicted_read_speed
            << ") | Write: (" << d.predicted_write_speed << ") | SN: "
            << d.serial_number;
        if (d.is_external) cout << " | External";
        cout << " ]" << endl;
    }
    cout << endl << endl;

    //print network infos
    cout << "Local IP (wifi): " << net.get_local_ip() << endl;
    cout << "MAC Address: " << net.get_mac_address() << endl;
    cout << "Locale: " << net.get_locale() << endl;
    cout << "Public ip:" << net.get_public_ip() << endl;
    cout << "(SSID) Network Name: " << net.get_network_name() << endl;
    cout << "Network Speed:" << net.get_network_speed() << endl;

    cout << endl << endl;

        return 0;
}

    */
