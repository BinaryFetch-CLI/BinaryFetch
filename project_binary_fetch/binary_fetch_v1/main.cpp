// main.cpp (AsciiArt separated into header and implementation files)

#include <iostream>
#include <iomanip>        // Formatting utilities (setw, precision)
#include <vector>
#include <functional>
#include <sstream>        // For string stream operations
#include <fstream>
#include <string>

// ASCII Art functionality
#include "AsciiArt.h"

// ------------------ Full System Info Modules ------------------
#include "OSInfo.h"             // OS name, version, build info
#include "CPUInfo.h"            // CPU model, threads, cores, base/boost clocks
#include "MemoryInfo.h"         // RAM capacity, usage, speed, type
#include "GPUInfo.h"            // GPU model and basic information
#include "StorageInfo.h"        // Disk drives, partitions, used/free space
#include "NetworkInfo.h"        // Active network adapters, IP, speeds
#include "DetailedGPUInfo.h"    // Deep GPU details: VRAM usage, clocks, temps
#include "PerformanceInfo.h"    // CPU load, RAM load, GPU usage
#include "UserInfo.h"           // Username, PC name, domain
#include "SystemInfo.h"         // Motherboard, BIOS, system manufacturer
#include "DisplayInfo.h"        // Monitor resolution, refresh rate, scaling
#include "ExtraInfo.h"          // Additional misc system data


// ------------------ Compact Mode Output Modules ------------------
#include "CompactAudio.h"       // Audio device summary for compact mode
#include "CompactOS.h"          // Lightweight OS summary
#include "CompactCPU.h"         // Lightweight CPU summary
#include "CompactMemory.h"      // Lightweight RAM summary
#include "CompactScreen.h"      // Lightweight screen resolution summary
#include "CompactSystem.h"      // Lightweight motherboard/system summary
#include "CompactGPU.h"         // Lightweight GPU summary
#include "CompactPerformance.h" // Lightweight performance stats
#include "CompactUser.h"        // Lightweight user info
#include "CompactNetwork.h"     // Lightweight network info
#include "compact_disk_info.h"  // Lightweight storage/disk info (compact mode)

using namespace std;

// ------------------ main (modified to stream output) ------------------

int main() {

    // Initialize ASCII Art
    AsciiArt art;
    if (!art.loadFromFile("AsciiArt.txt")) {
        // fallback: a small placeholder art (keeps indentation)
        std::ofstream tmp("AsciiArt.txt"); // optional: create a default file
        tmp << "BinaryFetch\n";
        tmp.close();
        art.loadFromFile("AsciiArt.txt");
        std::cout << "Note: ASCII art not loaded from file; using placeholder.\n";
    }

    // Create LivePrinter
    LivePrinter lp(art);

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

    // ---------------- Now stream info line-by-line ----------------

    // Header
    lp.push(""); // keep art line 0 + blank info (or you can push a title)
    lp.push("_>> BinaryFetch_____________________________________________________");

    // Minimal OS
    {
        std::ostringstream ss;
        ss << "[OS]  -> " << c_os.getOSName()
            << c_os.getOSBuild()
            << " (" << c_os.getArchitecture() << ")"
            << " (uptime: " << c_os.getUptime() << ")";
        lp.push(ss.str());
    }

    // Minimal CPU
    {
        std::ostringstream ss;
        ss << "[CPU] -> " << c_cpu.getCPUName() << " ("
            << c_cpu.getCPUCores() << "C/"
            << c_cpu.getCPUThreads() << "T)"
            << std::fixed << std::setprecision(2)
            << " @ " << c_cpu.getClockSpeed() << " GHz ";
        lp.push(ss.str());
    }

    // Displays
    {
        auto screens = c_screen.get_screens();
        int idx = 1;
        if (screens.empty()) {
            lp.push("[Display] -> No displays detected");
        }
        else {
            for (const auto& s : screens) {
                std::ostringstream ss;
                ss << "[Display " << idx++ << "] -> "
                    << s.brand_name << " (" << s.resolution << ") @"
                    << s.refresh_rate << "Hz";
                lp.push(ss.str());
            }
        }
    }

    // Memory minimal
    {
        std::ostringstream ss;
        ss << "[Memory] -> " << "(total: " << c_memory.get_total_memory() << " GB)"
            << " (free: " << c_memory.get_free_memory() << " GB)"
            << " ( " << c_memory.get_used_memory_percent() << "% ) ";
        lp.push(ss.str());
    }

    // Audio
    {
        std::ostringstream ss1, ss2;
        ss1 << "[Audio Input] -> " << c_audio.active_audio_input() << c_audio.active_audio_input_status();
        ss2 << "[Audio Output] -> " << c_audio.active_audio_output() << c_audio.active_audio_output_status();
        lp.push(ss1.str());
        lp.push(ss2.str());
    }

    // BIOS & Motherboard (compact) - safe concatenation via ostringstream
    {
        std::ostringstream ss1;
        ss1 << "[BIOS] -> " << c_system.getBIOSInfo();
        lp.push(ss1.str());
        std::ostringstream ss2;
        ss2 << "[Motherboard] -> " << c_system.getMotherboardInfo();
        lp.push(ss2.str());
    }

    // GPU minimal
    {
        std::ostringstream ss;
        ss << "[GPU] -> " << c_gpu.getGPUName()
            << " (" << c_gpu.getGPUUsagePercent() << "%)"
            << " (" << c_gpu.getVRAMGB() << " GB)"
            << " (@" << c_gpu.getGPUFrequency() << ") ";
        lp.push(ss.str());
    }

    // Minimal Performance
    {
        std::ostringstream ss;
        ss << "[Performance] -> "
            << "(CPU: " << c_perf.getCPUUsage() << "%) "
            << "(GPU: " << c_perf.getGPUUsage() << "%) "
            << "(RAM: " << c_perf.getRAMUsage() << "%) "
            << "(Disk: " << c_perf.getDiskUsage() << "%) ";
        lp.push(ss.str());
    }

    // User
    {
        std::ostringstream ss;
        ss << "[User] -> @" << c_user.getUsername()
            << " -> (Domain: " << c_user.getDomain() << ")"
            << " -> (Type: " << c_user.isAdmin() << ")";
        lp.push(ss.str());
    }

    // Network minimal
    {
        std::ostringstream ss;
        ss << "[network] -> " << "(Name: " << c_net.get_network_name()
            << ") (Type: " << c_net.get_network_type()
            << ") (ip: " << c_net.get_network_ip() << ") ";
        lp.push(ss.str());
    }

    // Disk usage (compact)
    {
        auto disks = disk.getAllDiskUsage();
        std::ostringstream ss;
        ss << "[Disk] -> ";
        for (const auto& d : disks) {
            ss << "(" << d.first[0] << ": "
                << std::fixed << std::setprecision(1)
                << d.second << "%) ";
        }
        lp.push(ss.str());

        // capacities
        auto caps = disk.getDiskCapacity();
        std::ostringstream sc;
        sc << "[Disk Cap] -> ";
        for (const auto& c : caps) sc << "(" << c.first[0] << "-" << c.second << "GB)";
        lp.push(sc.str());
    }

    // Full detailed section (Memory Info)
    {
        lp.push(""); // blank line
        lp.push("---------------Memory Info--------------");
        {
            std::ostringstream ss;
            ss << "(Total: " << ram.getTotal() << " GB) "
                << "(Free: " << ram.getFree() << " GB) "
                << "(Used: " << ram.getUsedPercentage() << "%)";
            lp.push(ss.str());
        }

        const auto& modules = ram.getModules();
        for (size_t i = 0; i < modules.size(); ++i) {
            // --- Zero-pad capacity ---
            std::string cap = modules[i].capacity;   // e.g. "8GB"
            int num = 0;
            try { num = std::stoi(cap); }
            catch (...) { num = 0; }
            std::ostringstream capOut;
            capOut << std::setw(2) << std::setfill('0') << num << "GB";

            std::ostringstream ss;
            ss << "Memory " << i << ": "
                << "(Used: " << ram.getUsedPercentage() << "%) "
                << capOut.str() << " "
                << modules[i].type << " "
                << modules[i].speed;
            lp.push(ss.str());
        }
    }
     
    // Storage Info (detailed) - STREAMING VERSION
    {
        lp.push("");
        lp.push("------------------------- STORAGE SUMMARY --------------------------");

        auto fmt_storage = [](const std::string& s) -> std::string {
            std::ostringstream oss;
            double v = 0.0;
            try { v = stod(s); }
            catch (...) { v = 0.0; }
            oss << std::fixed << std::setprecision(2)
                << std::setw(7) << std::right << std::setfill(' ')
                << v;
            return oss.str();
            };

        auto fmt_speed = [](const std::string& s) -> std::string {
            std::ostringstream tmp;
            double v = 0.0;
            try { v = stod(s); }
            catch (...) { v = 0.0; }
            tmp << std::fixed << std::setprecision(2) << v;
            std::string val = tmp.str();
            int padding = 7 - (int)val.size();
            if (padding < 0) padding = 0;
            return std::string(padding, ' ') + val;
            };

        // Track disks for later sections
        std::vector<storage_data> all_disks_captured;

        // Process each disk as it's ready
        storage.process_storage_info([&](const storage_data& d) {
            // Store for later sections
            all_disks_captured.push_back(d);

            // Build and push SUMMARY line immediately
            std::ostringstream ss;
            ss << d.storage_type << " " << d.drive_letter
                << " [ (Used) " << fmt_storage(d.used_space)
                << " GiB /" << fmt_storage(d.total_space)
                << " GiB  " << d.used_percentage
                << " - " << d.file_system << " "
                << (d.is_external ? "Ext ]" : "Int ]");
            lp.push(ss.str());
            });

        // Now print performance sections using captured data
        if (!all_disks_captured.empty()) {
            lp.push("");
            lp.push("-------------------- DISK PERFORMANCE & DETAILS --------------------");

            for (const auto& d : all_disks_captured) {
                std::ostringstream ss;
                ss << d.drive_letter << " [ Read: "
                    << fmt_speed(d.read_speed)
                    << " MB/s | Write: "
                    << fmt_speed(d.write_speed)
                    << " MB/s | " << d.serial_number
                    << (d.is_external ? " Ext ]" : " Int ]");
                lp.push(ss.str());
            }

            lp.push("");
            lp.push("---------------- DISK PERFORMANCE & DETAILS (Predicted) ------------");

            for (const auto& d : all_disks_captured) {
                std::ostringstream ss;
                ss << d.drive_letter << " [ Read: "
                    << fmt_speed(d.predicted_read_speed)
                    << " MB/s | Write: "
                    << fmt_speed(d.predicted_write_speed)
                    << " MB/s | " << d.serial_number
                    << (d.is_external ? " Ext ]" : " Int ]");
                lp.push(ss.str());
            }
        }
        else {
            lp.push("No drives detected.");
        }
    }

    // Network (Compact + Extra)
    {
        cout << endl;
        lp.push("#-Network Info-----------------------------------------------------#");
        {
            std::ostringstream ss;
            ss << "Network Name               : " << net.get_network_name();
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "Network Type               : " << c_net.get_network_type();
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "IP                         : " << net.get_public_ip();
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "Locale                     : " << net.get_locale();
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "Mac address                : " << net.get_mac_address();
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "Network avg upload speed   : " << net.get_network_upload_speed();
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "Network avg download speed : " << net.get_network_download_speed();
            lp.push(ss.str());
        }
    }




    // OS Info
    {
        cout << endl;
        lp.push("#-Operating System ------------------------------------------------#");
        {
            std::ostringstream ss;
            ss << "Name                       : " << os.GetOSName();
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "Version                    : " << os.GetOSVersion();
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "Architecture               : " << os.GetOSArchitecture();
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "Kernel                     : " << os.get_os_kernel_info();
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "Uptime                     : " << os.get_os_uptime();
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "Install Date               : " << os.get_os_install_date();
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "Serial                     : " << os.get_os_serial_number();
            lp.push(ss.str());
        }
    }


    // CPU Info
    {
        cout << endl;
        lp.push("#-CPU Info --------------------------------------------------------#");
        {
            std::ostringstream ss;
            ss << "Brand                      : " << cpu.get_cpu_info();
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "Utilization                : " << cpu.get_cpu_utilization() << "%";
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "Speed                      : " << cpu.get_cpu_speed();
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "Base Speed                 : " << cpu.get_cpu_base_speed();
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "Cores                      : " << cpu.get_cpu_cores();
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "Logical Processors         : " << cpu.get_cpu_logical_processors();
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "Sockets                    : " << cpu.get_cpu_sockets();
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "Virtualization             : " << cpu.get_cpu_virtualization();
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "L1 Cache                   : " << cpu.get_cpu_l1_cache();
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "L2 Cache                   : " << cpu.get_cpu_l2_cache();
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "L3 Cache                   : " << cpu.get_cpu_l3_cache();
            lp.push(ss.str());
        }
    }


    // GPU Info (detailed)-------------------------------------------------------------
    {
        cout << endl;
        auto all_gpu_info = obj_gpu.get_all_gpu_info();
        if (all_gpu_info.empty()) {
            lp.push("#-GPU Info --------------------------------------------------------#");
            lp.push("No GPU detected.");
        }
        else {
            lp.push("#-GPU Info --------------------------------------------------------#");
            for (size_t i = 0; i < all_gpu_info.size(); ++i) {
                auto& g = all_gpu_info[i];

                // GPU index line (dynamic length handled to meet 27-char rule)
                {
                    std::ostringstream label;
                    if (i == 0)
                    {
                        label << "GPU " << (i + 1);
                    }
                    else {
                        label << "#-" << "GPU " << (i + 1) << "------------------------------------------------------------#";
                    }
                    
                    std::string lbl = label.str();
                    if (lbl.length() < 27) lbl += std::string(27 - lbl.length(), ' ');
                    std::ostringstream ss;
                    ss << lbl ;
                    lp.push(ss.str());
                }

                {
                    std::ostringstream ss;
                    ss << "|-> Name                   : " << g.gpu_name;
                    lp.push(ss.str());
                }
                {
                    std::ostringstream ss;
                    ss << "|-> Memory                 : " << g.gpu_memory;
                    lp.push(ss.str());
                }
                {
                    std::ostringstream ss;
                    ss << "|-> Usage                  : " << g.gpu_usage << "%";
                    lp.push(ss.str());
                }
                {
                    std::ostringstream ss;
                    ss << "|-> Vendor                 : " << g.gpu_vendor;
                    lp.push(ss.str());
                }
                {
                    std::ostringstream ss;
                    ss << "|-> Driver Version         : " << g.gpu_driver_version;
                    lp.push(ss.str());
                }
                {
                    std::ostringstream ss;
                    ss << "|-> Temperature            : " << g.gpu_temperature << " C";
                    lp.push(ss.str());
                }
                {
                    std::ostringstream ss;
                    ss << "#-> Core Count             : " << g.gpu_core_count;
                    lp.push(ss.str());
                }
            }

            auto primary = detailed_gpu_info.primary_gpu_info();
            {
                std::ostringstream ss;
                ss << "#-Primary GPU Details----------------------------------------------#";
                lp.push(ss.str());
            }
            {
                std::ostringstream ss;
                ss << "|-> Name                   : " << primary.name;
                lp.push(ss.str());
            }
            {
                std::ostringstream ss;
                ss << "|-> VRAM                   : " << primary.vram_gb << " GiB";
                lp.push(ss.str());
            }
            {
                std::ostringstream ss;
                ss << "#-> Frequency              : " << primary.frequency_ghz << " GHz";
                lp.push(ss.str());
            }
        }
    }


    // Display Info
    {
        cout << endl;
        //lp.push("#-Display----------------------------------------------------------#");
        auto monitors = display.get_all_displays();
        if (monitors.empty()) {
            lp.push("No monitors detected.");
        }
        else {
            for (size_t i = 0; i < monitors.size(); ++i) {
                auto& m = monitors[i];
                {
                    std::ostringstream ss;
                    ss << "#-Monitor " << (i + 1) << "--------------------------------------------------------#";
                    lp.push(ss.str());
                }
                {
                    std::ostringstream ss;
                    ss << "Brand                      : " << m.brand_name;
                    lp.push(ss.str());
                }
                {
                    std::ostringstream ss;
                    ss << "Resolution                 : " << m.resolution;
                    lp.push(ss.str());
                }
                {
                    std::ostringstream ss;
                    ss << "Refresh Rate               : " << m.refresh_rate << " Hz";
                    lp.push(ss.str());
                }
            }
        }
    }


    // BIOS & Motherboard Info
    {
        cout << endl;
        lp.push("#-BIOS & Motherboard Info -----------------------------------------#");
        {
            std::ostringstream ss;
            ss << "Bios Vendor                : " << sys.get_bios_vendor();
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "Bios Version               : " << sys.get_bios_version();
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "Bios Date                  : " << sys.get_bios_date();
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "Motherboard Model          : " << sys.get_motherboard_model();
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "Motherboard Manufacturer   : " << sys.get_motherboard_manufacturer();
            lp.push(ss.str());
        }
    }


    // User Info
    {
        cout << endl;
        lp.push("#-User Info -------------------------------------------------------#");
        {
            std::ostringstream ss;
            ss << "Username                   : " << user.get_username();
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "Computer Name              : " << user.get_computer_name();
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "Domain                     : " << user.get_domain_name();
            lp.push(ss.str());
        }
        {
           //std::ostringstream ss;
           // ss << "Groups                     : " << user.get_user_groups();
           // lp.push(ss.str());
        }
    }


    // Performance Info
    {
        cout << endl;
        lp.push("#-Performance Info ------------------------------------------------#");
        {
            std::ostringstream ss;
            ss << "System Uptime              : " << perf.get_system_uptime();
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "CPU Usage                  : " << perf.get_cpu_usage_percent() << "%";
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "RAM Usage                  : " << perf.get_ram_usage_percent() << "%";
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "Disk Usage                 : " << perf.get_disk_usage_percent() << "%";
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << "GPU Usage                  : " << perf.get_gpu_usage_percent() << "%";
            lp.push(ss.str());
        }
    }


    // Audio & Power
    {
        cout << endl;
        ExtraInfo audio;

       

        // Get and display OUTPUT devices
        vector<AudioDevice> outputDevices = audio.get_output_devices();
        lp.push("#-Audio Output ----------------------------------------------------#");
        int audio_output_device_count = 0;
        for (const auto& device : outputDevices) {
            audio_output_device_count++;
            ostringstream oss;
            oss << "~ " << audio_output_device_count   << " " << device.name;
            if (device.isActive) {
                // You can add color codes here if LivePrinter supports it
                oss << " (active)";
            }
            lp.push(oss.str());
        }

        // Get and display INPUT devices
        vector<AudioDevice> inputDevices = audio.get_input_devices();
        lp.push("#-Audio Input ----------------------------------------------------#");
        int audio_input_device_count = 0;
        for (const auto& device : inputDevices) {
            audio_input_device_count++;
            ostringstream oss;
            oss << "~ "<< audio_input_device_count << " " << device.name;
            if (device.isActive) {
                oss << " (active)";
            }
            lp.push(oss.str());
        }

        lp.push("");  // Empty line for spacing

        // Get and display POWER status
        PowerStatus power = audio.get_power_status();
        ostringstream ossPower;
        lp.push("#-Power Status ---------------------------------------------------#");

        if (!power.hasBattery) {
            ossPower << "[Wired connection]";
        }
        else {
            ossPower << "Battery powered (" << power.batteryPercent << "%)";
            if (power.isCharging) {
                ossPower << " (Charging)";
            }
            else {
                ossPower << " (Not Charging)";
            }
        }
        lp.push(ossPower.str());

        
    }


    // ---------------- End of info lines ----------------

    // Print remaining ASCII art lines (if art is taller than info)
    lp.finish();

    std::cout << std::endl;
    return 0;
}


/*  main.cpp documentation:

------------------------------------------------
CLASS: AsciiArt
OBJECT: art
DESCRIPTION: Handles ASCII art loading and display.
FUNCTIONS:
    bool loadFromFile(const std::string& filename) -> bool
        Loads ASCII art from a file. Returns true if successful.
------------------------------------------------
CLASS: LivePrinter
OBJECT: lp
DESCRIPTION: Streams system info line-by-line alongside ASCII art.
FUNCTIONS:
    void push(const std::string& line) -> void
        Adds a line to the output queue.
    void finish() -> void
        Completes printing, including remaining ASCII art lines.
------------------------------------------------
CLASS: OSInfo
OBJECT: os
DESCRIPTION: Retrieves full OS details.
FUNCTIONS:
    std::string GetOSName() -> std::string
    std::string GetOSVersion() -> std::string
    std::string GetOSArchitecture() -> std::string
    std::string get_os_kernel_info() -> std::string
    std::string get_os_uptime() -> std::string
    std::string get_os_install_date() -> std::string
    std::string get_os_serial_number() -> std::string
------------------------------------------------
CLASS: CPUInfo
OBJECT: cpu
DESCRIPTION: Retrieves CPU details and statistics.
FUNCTIONS:
    std::string get_cpu_info() -> std::string
    float get_cpu_utilization() -> float
    float get_cpu_speed() -> float
    float get_cpu_base_speed() -> float
    int get_cpu_cores() -> int
    int get_cpu_logical_processors() -> int
    int get_cpu_sockets() -> int
    bool get_cpu_virtualization() -> bool
    std::string get_cpu_l1_cache() -> std::string
    std::string get_cpu_l2_cache() -> std::string
    std::string get_cpu_l3_cache() -> std::string
------------------------------------------------
CLASS: MemoryInfo
OBJECT: ram
DESCRIPTION: Retrieves RAM details.
FUNCTIONS:
    float getTotal() -> float
    float getFree() -> float
    float getUsedPercentage() -> float
    struct Module {
        std::string capacity;
        std::string type;
        std::string speed;
    };
    std::vector<Module> getModules() -> std::vector<Module>
------------------------------------------------
CLASS: GPUInfo
OBJECT: obj_gpu
DESCRIPTION: Retrieves GPU basic info.
FUNCTIONS:
    struct GPU {
        std::string gpu_name;
        std::string gpu_memory;
        float gpu_usage;
        std::string gpu_vendor;
        std::string gpu_driver_version;
        float gpu_temperature;
        int gpu_core_count;
    };
    std::vector<GPU> get_all_gpu_info() -> std::vector<GPU>
------------------------------------------------
CLASS: DetailedGPUInfo
OBJECT: detailed_gpu_info
DESCRIPTION: Provides detailed GPU stats.
FUNCTIONS:
    struct PrimaryGPU {
        std::string name;
        float vram_gb;
        float frequency_ghz;
    };
    PrimaryGPU primary_gpu_info() -> PrimaryGPU
------------------------------------------------
CLASS: StorageInfo
OBJECT: storage
DESCRIPTION: Detailed storage and disk info.
FUNCTIONS:
    struct Disk {
        std::string storage_type;
        std::string drive_letter;
        std::string used_space;
        std::string total_space;
        float used_percentage;
        std::string file_system;
        bool is_external;
        std::string read_speed;
        std::string write_speed;
        std::string predicted_read_speed;
        std::string predicted_write_speed;
        std::string serial_number;
    };
    std::vector<Disk> get_all_storage_info() -> std::vector<Disk>
------------------------------------------------
CLASS: NetworkInfo
OBJECT: net
DESCRIPTION: Retrieves network adapter info.
FUNCTIONS:
    std::string get_network_name() -> std::string
    std::string get_network_type() -> std::string
    std::string get_network_ip() -> std::string
------------------------------------------------
CLASS: UserInfo
OBJECT: user
DESCRIPTION: Retrieves logged-in user information.
FUNCTIONS:
    std::string get_username() -> std::string
    std::string get_computer_name() -> std::string
    std::string get_domain_name() -> std::string
    std::string get_user_groups() -> std::string
------------------------------------------------
CLASS: PerformanceInfo
OBJECT: perf
DESCRIPTION: Retrieves system performance metrics.
FUNCTIONS:
    float get_cpu_usage_percent() -> float
    float get_ram_usage_percent() -> float
    float get_disk_usage_percent() -> float
    float get_gpu_usage_percent() -> float
    std::string get_system_uptime() -> std::string
------------------------------------------------
CLASS: DisplayInfo
OBJECT: display
DESCRIPTION: Retrieves connected monitor info.
FUNCTIONS:
    struct Monitor {
        std::string brand_name;
        std::string resolution;
        int refresh_rate;
    };
    std::vector<Monitor> get_all_displays() -> std::vector<Monitor>
------------------------------------------------
CLASS: ExtraInfo
OBJECTS: extra, audio
DESCRIPTION: Provides additional system info such as audio and power.
FUNCTIONS:
    void get_audio_devices() -> void
    void get_power_status() -> void
------------------------------------------------
CLASS: SystemInfo
OBJECT: sys
DESCRIPTION: Motherboard and BIOS info.
FUNCTIONS:
    std::string get_bios_vendor() -> std::string
    std::string get_bios_version() -> std::string
    std::string get_bios_date() -> std::string
    std::string get_motherboard_model() -> std::string
    std::string get_motherboard_manufacturer() -> std::string
------------------------------------------------
CLASS: CompactAudio
OBJECT: c_audio
DESCRIPTION: Lightweight audio info.
FUNCTIONS:
    std::string active_audio_input() -> std::string
    std::string active_audio_input_status() -> std::string
    std::string active_audio_output() -> std::string
    std::string active_audio_output_status() -> std::string
------------------------------------------------
CLASS: CompactOS
OBJECT: c_os
DESCRIPTION: Lightweight OS summary.
FUNCTIONS:
    std::string getOSName() -> std::string
    std::string getOSBuild() -> std::string
    std::string getArchitecture() -> std::string
    std::string getUptime() -> std::string
------------------------------------------------
CLASS: CompactCPU
OBJECT: c_cpu
DESCRIPTION: Lightweight CPU summary.
FUNCTIONS:
    std::string getCPUName() -> std::string
    int getCPUCores() -> int
    int getCPUThreads() -> int
    float getClockSpeed() -> float
------------------------------------------------
CLASS: CompactMemory
OBJECT: c_memory
DESCRIPTION: Lightweight memory summary.
FUNCTIONS:
    float get_total_memory() -> float
    float get_free_memory() -> float
    float get_used_memory_percent() -> float
------------------------------------------------
CLASS: CompactScreen
OBJECT: c_screen
DESCRIPTION: Lightweight display summary.
FUNCTIONS:
    struct Screen {
        std::string brand_name;
        std::string resolution;
        int refresh_rate;
    };
    std::vector<Screen> get_screens() -> std::vector<Screen>
------------------------------------------------
CLASS: CompactSystem
OBJECT: c_system
DESCRIPTION: Lightweight BIOS and motherboard summary.
FUNCTIONS:
    std::string getBIOSInfo() -> std::string
    std::string getMotherboardInfo() -> std::string
------------------------------------------------
CLASS: CompactGPU
OBJECT: c_gpu
DESCRIPTION: Lightweight GPU summary.
FUNCTIONS:
    std::string getGPUName() -> std::string
    float getGPUUsagePercent() -> float
    float getVRAMGB() -> float
    float getGPUFrequency() -> float
------------------------------------------------
CLASS: CompactPerformance
OBJECT: c_perf
DESCRIPTION: Lightweight performance metrics.
FUNCTIONS:
    float getCPUUsage() -> float
    float getGPUUsage() -> float
    float getRAMUsage() -> float
    float getDiskUsage() -> float
------------------------------------------------
CLASS: CompactUser
OBJECT: c_user
DESCRIPTION: Lightweight user info.
FUNCTIONS:
    std::string getUsername() -> std::string
    std::string getDomain() -> std::string
    bool isAdmin() -> bool
------------------------------------------------
CLASS: CompactNetwork
OBJECT: c_net
DESCRIPTION: Lightweight network summary.
FUNCTIONS:
    std::string get_network_name() -> std::string
    std::string get_network_type() -> std::string
    std::string get_network_ip() -> std::string
------------------------------------------------
CLASS: DiskInfo
OBJECT: disk
DESCRIPTION: Compact disk usage info.
FUNCTIONS:
    std::vector<std::pair<std::string, float>> getAllDiskUsage() -> std::vector<std::pair<std::string, float>>
    std::vector<std::pair<std::string, float>> getDiskCapacity() -> std::vector<std::pair<std::string, float>>
------------------------------------------------
*/

