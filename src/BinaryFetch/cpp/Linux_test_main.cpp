#include <iostream>
#include <vector>
#include <iomanip>
#include "CompactAudio.h"
#include "CompactCPU.h"
#include "CompactGPU.h"
#include "CompactMemory.h"
#include "CompactNetwork.h"
#include "CompactOS.h"
#include "CompactPerformance.h"
#include "CompactScreen.h"
#include "CompactSystem.h"
#include "CompactUser.h"
#include "compact_disk_info.h"
#include "TimeInfo.h"
#include "StorageInfo.h"

#include <thread>
#include <chrono>


// detailed sections
#include "CPUInfo.h"
#include "PerformanceInfo.h"
#include "DisplayInfo.h"
#include "ExtraInfo.h"
#include "GPUInfo.h"
#include "MemoryInfo.h"
#include "NetworkInfo.h"
#include "OSInfo.h"
#include "PerformanceInfo.h"
#include "StorageInfo.h"
#include "SystemInfo.h"
#include "UserInfo.h"
#include "DetailedGPUInfo.h"


using namespace std;

int main() {
    cout << "========================================\n";
    cout << "         BINARYFETCH - SYSTEM INFO       \n";
    cout << "========================================\n\n";

    // What time is it right now?
    cout << "[Time------------------------------------]\n";
    TimeInfo time;
    cout << "Now: " << time.getHour() << ":" << time.getMinute() << ":" << time.getSecond() << "\n";
    cout << "Today: " << time.getDayName() << ", " << time.getMonthName() << " " << time.getDay() << ", " << time.getYearNumber() << "\n";
    cout << "Leap Year? " << time.getLeapYear() << " | Week: " << time.getWeekNumber() << "\n\n";

    // Who's logged in?
    cout << "[User------------------------------------]\n";
    CompactUser user;
    cout << "Name: " << user.getUsername() << "\n";
    cout << "Domain: " << user.getDomain() << "\n";
    cout << "Is Admin: " << user.isAdmin() << "\n\n";

    // Basic OS details
    cout << "[Operating System------------------------------------]\n";
    CompactOS os;
    cout << "OS: " << os.getOSName() << "\n";
    cout << "Kernel: " << os.getOSBuild() << "\n";
    cout << "Arch: " << os.getArchitecture() << "\n";
    cout << "Uptime: " << os.getUptime() << "\n\n";

    // Processor info
    cout << "[CPU------------------------------------]\n";
    CompactCPU cpu;
    cout << "Name: " << cpu.getCPUName() << "\n";
    cout << "Cores: " << cpu.getCPUCores() << " | Threads: " << cpu.getCPUThreads() << "\n";
    cout << "Speed: " << cpu.getClockSpeed() << " GHz\n";
    cout << "Usage: " << cpu.getUsagePercent() << "%\n\n";

    // RAM info
    cout << "[Memory------------------------------------]\n";
    CompactMemory memory;
    cout << "Total: " << memory.get_total_memory() << " GB\n";
    cout << "Free: " << memory.get_free_memory() << " GB\n";
    cout << "Used: " << memory.get_used_memory_percent() << "%\n";
    cout << "Slots Used/Available: " << memory.memory_slot_used() << "/" << memory.memory_slot_available() << "\n\n";

    // Quick performance snapshot
    cout << "[Performance------------------------------------]\n";
    CompactPerformance perf;
    cout << "CPU: " << perf.getCPUUsage() << "%\n";
    cout << "RAM: " << perf.getRAMUsage() << "%\n";
    cout << "Disk: " << perf.getDiskUsage() << "%\n";
    cout << "GPU: " << perf.getGPUUsage() << "%\n\n";

    // Graphics card info
    cout << "[GPU------------------------------------]\n";
    cout << "Name: " << CompactGPU::getGPUName() << "\n";
    cout << "VRAM: " << CompactGPU::getVRAMGB() << " GB\n";
    cout << "Usage: " << CompactGPU::getGPUUsagePercent() << "%\n";
    cout << "Frequency: " << CompactGPU::getGPUFrequency() << "\n";
    cout << "Temp: " << CompactGPU::getGPUTemperature() << " C\n\n";

    // Connected screens/monitors
    cout << "[Screens------------------------------------]\n";
    CompactScreen screen;
    auto screens = screen.getScreens();
    cout << "Count: " << screens.size() << "\n";
    for (size_t i = 0; i < screens.size(); ++i) {
        cout << "  Screen " << i + 1 << ": " << screens[i].name << "\n";
        cout << "    Resolution: " << screens[i].current_width << "x" << screens[i].current_height
             << " (Native: " << screens[i].native_width << "x" << screens[i].native_height << ")\n";
        cout << "    Refresh Rate: " << screens[i].refresh_rate << " Hz\n";
        cout << "    Scale: " << screens[i].scale_percent << "% (" << screens[i].scale_mul << ")\n";
        cout << "    Upscaling: " << screens[i].upscale << "\n";
    }
    cout << "\n";

    // Motherboard / BIOS
    cout << "[System------------------------------------]\n";
    CompactSystem systemInfo;
    cout << "BIOS: " << systemInfo.getBIOSInfo() << "\n";
    cout << "Motherboard: " << systemInfo.getMotherboardInfo() << "\n\n";

    // Speakers / mic status
    cout << "[Audio------------------------------------]\n";
    CompactAudio audio;
    cout << "Output: " << audio.active_audio_output() << " (" << audio.active_audio_output_status() << ")\n";
    cout << "Input: " << audio.active_audio_input() << " (" << audio.active_audio_input_status() << ")\n\n";

    // Wifi / network status
    cout << "[Network------------------------------------]\n";
    CompactNetwork network;
    cout << "IP: " << network.get_network_ip() << "\n";
    cout << "Type: " << network.get_network_type() << "\n";
    cout << "Name: " << network.get_network_name() << "\n\n";

    // Disk space usage
    cout << "[Disks - Usage------------------------------------]\n";
    DiskInfo disk;
    auto diskUsages = disk.getAllDiskUsage();
    auto diskCapacities = disk.getDiskCapacity();

    for (size_t i = 0; i < diskUsages.size(); ++i) {
        cout << "  " << diskUsages[i].first << " -> " << diskUsages[i].second << "% used";

        // match capacity to this same mount point
        for (const auto& cap : diskCapacities) {
            if (cap.first == diskUsages[i].first) {
                cout << " (Capacity: " << cap.second << " GB)";
                break;
            }
        }
        cout << "\n";
    }
    cout << "\n";

    // Real disk speed test (actual read/write)
    cout << "[Disks - Real Speed Test------------------------------------]\n";
    StorageInfo storage;
    storage.process_storage_info([](const storage_data& d) {
        cout << d.drive_letter
             << " | Read: "  << setw(8) << d.read_speed  << " MB/s"
             << " | Write: " << setw(8) << d.write_speed << " MB/s"
             << " | " << d.file_system
             << " | SN-" << d.serial_number
             << " | " << (d.is_external ? "External" : "Internal") << "\n";
    });
    cout << "\n";

    // Estimated disk speed (no actual test, just a guess based on type)
    cout << "[Disks - Estimated Speed------------------------------------]\n";
    for (const auto& d : storage.get_all_storage_info()) {
        cout << d.drive_letter
             << " | Read: "  << setw(8) << d.predicted_read_speed  << " MB/s"
             << " | Write: " << setw(8) << d.predicted_write_speed << " MB/s"
             << " | " << d.storage_type
             << " | SN-" << d.serial_number
             << " | " << (d.is_external ? "External" : "Internal") << "\n";
    }



cout << "--- [CPUInfo] ---\n";
    CPUInfo cpuInfo;
    
    // root needed
    cout << "CPU: " << cpuInfo.get_cpu_info() << "\n";
    cout << "Base Speed: " << cpuInfo.get_cpu_base_speed() << "\n";
    cout << "Current Speed: " << cpuInfo.get_cpu_speed() << "\n";
    cout << "Utilization: " << cpuInfo.get_cpu_utilization() << "%\n";

    cout << "Sockets: " << cpuInfo.get_cpu_sockets() << "\n";
    cout << "Cores: " << cpuInfo.get_cpu_cores() << "\n";
    cout << "Logical Processors: " << cpuInfo.get_cpu_logical_processors() << "\n";

    cout << "Virtualization: " << cpuInfo.get_cpu_virtualization() << "\n";

    cout << "L1 Cache: " << cpuInfo.get_cpu_l1_cache() << "\n";
    cout << "L2 Cache: " << cpuInfo.get_cpu_l2_cache() << "\n";
    cout << "L3 Cache: " << cpuInfo.get_cpu_l3_cache() << "\n";

    cout << "Uptime: " << cpuInfo.get_system_uptime() << "\n";
    cout << "Processes: " << cpuInfo.get_process_count() << "\n";
    cout << "Threads: " << cpuInfo.get_thread_count() << "\n";
    cout << "Handles: " << cpuInfo.get_handle_count() << "\n";




   // Detailed display info
    cout << "--- [DisplayInfo] ---\n";
    DisplayInfo display;
    auto displayScreens = display.getScreens();
 
    cout << "Detected Screens: " << displayScreens.size() << "\n";
 
    for (size_t i = 0; i < displayScreens.size(); ++i) {
        const auto& s = displayScreens[i];
        cout << "\nScreen " << i + 1 << ": " << s.name << "\n";
        cout << "  Resolution: " << s.current_width << "x" << s.current_height
             << " @ " << s.refresh_rate << " Hz\n";
        cout << "  Native: " << s.native_resolution << "\n";
        cout << "  Aspect Ratio: " << s.aspect_ratio << "\n";
        cout << "  Scale: " << s.scale_percent << "% (" << s.scale_mul << ")\n";
        cout << "  Upscale: " << s.upscale << " (" << s.dsr_type << ")\n";
    }
 
    if (displayScreens.empty()) {
        cout << "No screens detected.\n";
    }




DetailedGPUInfo gpuInfo;

    cout << "--- [DetailedGPUInfo] ---" << endl;

    vector<GPUData> gpus = gpuInfo.get_all_gpus();

    if (gpus.empty()) {
        cout << "No GPUs detected." << endl;
        
    }

    cout << "Detected GPUs: " << gpus.size() << endl;

    for (const auto& gpu : gpus) {
        cout << "GPU " << gpu.index << ": " << gpu.name << endl;

        cout << "  VRAM: ";
        if (gpu.vram_gb > 0.0f) {
            cout << fixed << setprecision(2) << gpu.vram_gb << " GB" << endl;
        } else {
            cout << "N/A" << endl;
        }

        cout << "  Frequency: ";
        if (gpu.frequency_ghz > 0.0f) {
            cout << fixed << setprecision(2) << gpu.frequency_ghz << " GHz" << endl;
        } else {
            cout << "Unknown" << endl;
        }
    }

    cout << endl;
    cout << "--- Primary GPU ---" << endl;

    GPUData primary = gpuInfo.primary_gpu_info();

    cout << "Name: " << primary.name << endl;

    cout << "VRAM: ";
    if (primary.vram_gb > 0.0f) {
        cout << fixed << setprecision(2) << primary.vram_gb << " GB" << endl;
    } else {
        cout << "N/A" << endl;
    }

    cout << "Frequency: ";
    if (primary.frequency_ghz > 0.0f) {
        cout << fixed << setprecision(2) << primary.frequency_ghz << " GHz" << endl;
    } else {
        cout << "Unknown" << endl;
    }







    cout << "----------------------------------------------------" << endl;
ExtraInfo extraInfo;

    cout << "--- [ExtraInfo] ---" << endl;

    cout << endl;
    cout << "Output Devices:" << endl;
    vector<AudioDevice> outputs = extraInfo.get_output_devices();
    if (outputs.empty()) {
        cout << "  None found." << endl;
    } else {
        for (const auto& device : outputs) {
            cout << "  " << device.name;
            if (device.isActive) cout << " [Active]";
            cout << endl;
        }
    }

    cout << endl;
    cout << "Input Devices:" << endl;
    vector<AudioDevice> inputs = extraInfo.get_input_devices();
    if (inputs.empty()) {
        cout << "  None found." << endl;
    } else {
        for (const auto& device : inputs) {
            cout << "  " << device.name;
            if (device.isActive) cout << " [Active]";
            cout << endl;
        }
    }

    cout << endl;
    cout << "Power Status:" << endl;
    PowerStatus power = extraInfo.get_power_status();

    cout << "  Has Battery: " << (power.hasBattery ? "Yes" : "No") << endl;

    if (power.hasBattery) {
        cout << "  Battery Percent: " << power.batteryPercent << "%" << endl;
        cout << "  Charging: " << (power.isCharging ? "Yes" : "No") << endl;
    }

    cout << "  AC Online: " << (power.isACOnline ? "Yes" : "No") << endl;





cout << "----------------------------------------------------" << endl;

vector<gpu_data> gpu_info_list = GPUInfo::get_all_gpu_info();

if (gpu_info_list.empty()) {
    cout << "No GPUs detected on this system.\n";
    
}

cout << "Detected " << gpu_info_list.size() << " GPU(s):\n\n";

for (size_t i = 0; i < gpu_info_list.size(); ++i) {
    const gpu_data& gpu = gpu_info_list[i];

    cout << "GPU " << i << ": " << gpu.gpu_name << "\n";
    cout << "  Vendor:       " << gpu.gpu_vendor << "\n";
    cout << "  Driver:       " << gpu.gpu_driver_version << "\n";
    cout << "  Memory:       " << gpu.gpu_memory << "\n";

    if (gpu.gpu_usage < 0.0f) {
        cout << "  Usage:        Unknown\n";
    } else {
        cout << "  Usage:        "
             << fixed << setprecision(0)
             << gpu.gpu_usage << " %\n";
    }

    if (gpu.gpu_temperature <= 0.0f) {
        cout << "  Temperature:  Unknown\n";
    } else {
        cout << "  Temperature:  "
             << fixed << setprecision(1)
             << gpu.gpu_temperature << " C\n";
    }

    if (gpu.gpu_frequency <= 0.0f) {
        cout << "  Frequency:    Unknown\n";
    } else {
        cout << "  Frequency:    "
             << fixed << setprecision(0)
             << gpu.gpu_frequency << " MHz\n";
    }

    if (gpu.gpu_core_count <= 0) {
        cout << "  Core Count:   Unknown\n";
    } else {
        cout << "  Core Count:   "
             << gpu.gpu_core_count << "\n";
    }

    cout << "\n";
}

cout << "--- Primary GPU quick stats (GPUInfo:: helper methods) ---\n";

float usage = GPUInfo::get_gpu_usage();

if (usage < 0.0f) {
    cout << "Usage:       Unknown\n";
} else {
    cout << "Usage:       "
         << fixed << setprecision(0)
         << usage << " %\n";
}

float temperature = GPUInfo::get_gpu_temperature();

if (temperature <= 0.0f) {
    cout << "Temperature: Unknown\n";
} else {
    cout << "Temperature: "
         << fixed << setprecision(1)
         << temperature << " C\n";
}

int coreCount = GPUInfo::get_gpu_core_count();

if (coreCount <= 0) {
    cout << "Core Count:  Unknown\n";
} else {
    cout << "Core Count:  "
         << coreCount << "\n";
}





cout << "-------------------memory---------------------------------" << endl;


MemoryInfo memInfo;

    // ---- Overall system memory ----
    cout << "===== System Memory =====" << endl;
    cout << "Total : " << memInfo.getTotal() << " GB" << endl;
    cout << "Free  : " << memInfo.getFree()  << " GB" << endl;
    cout << "Used  : " << memInfo.getUsedPercentage() << " %" << endl;
    cout << endl;

    // ---- Per-module (DIMM) info ----
    const auto& modules = memInfo.getModules();

    cout << "===== Memory Modules =====" << endl;
    cout << "Detected " << modules.size() << " module(s)" << endl;
    cout << endl;

    if (modules.empty()) {
        cout << "No module information available." << endl;
        cout << "(Requires dmidecode; try running with sudo.)" << endl;
        
    }

    int slot = 1;
    for (const auto& module : modules) {
        cout << "Module " << slot << ":" << endl;
        cout << "  Size  : " << module.capacity << endl;
        cout << "  Type  : " << module.type     << endl;
        cout << "  Speed : " << module.speed    << endl;
        cout << endl;
        slot++;
    }







    OSInfo osInfo;

    cout << "========================================\n";
    cout << "            OS INFO TEST                 \n";
    cout << "========================================\n\n";

    cout << "OS Version      : " << osInfo.GetOSVersion() << "\n";
    cout << "Architecture    : " << osInfo.GetOSArchitecture() << "\n";
    cout << "OS Name/Edition : " << osInfo.GetOSName() << "\n";
    cout << "Install Date    : " << osInfo.get_os_install_date() << "\n";
    cout << "Serial Number   : " << osInfo.get_os_serial_number() << "\n";
    cout << "Uptime          : " << osInfo.get_os_uptime() << "\n";
    cout << "Kernel Info     : " << osInfo.get_os_kernel_info() << "\n";








    cout << "========================================\n";
    cout << "         PERFORMANCE INFO TEST           \n";
    cout << "========================================\n\n";

    PerformanceInfo perfDetailed;

cout << "System Uptime : " << perfDetailed.get_system_uptime() << "\n\n";

cout << "Sampling CPU usage...\n";
float cpu1 = perfDetailed.get_cpu_usage_percent();
cout << "CPU Usage (first sample)  : " << fixed << setprecision(1) << cpu1 << " %\n";

this_thread::sleep_for(chrono::milliseconds(500));

float cpu2 = perfDetailed.get_cpu_usage_percent();
cout << "CPU Usage (second sample) : " << fixed << setprecision(1) << cpu2 << " %\n\n";

float ram = perfDetailed.get_ram_usage_percent();
cout << "RAM Usage  : ";
if (ram < 0.0f) cout << "Unknown\n";
else cout << fixed << setprecision(1) << ram << " %\n";

float diskUsage = perfDetailed.get_disk_usage_percent();
cout << "Disk Usage : ";
if (diskUsage < 0.0f) cout << "Unknown\n";
else cout << fixed << setprecision(1) << diskUsage << " %\n";

float gpu = perfDetailed.get_gpu_usage_percent();
cout << "GPU Usage  : ";
if (gpu < 0.0f) cout << "Unknown (no NVIDIA/AMD GPU usage source detected)\n";
else cout << fixed << setprecision(1) << gpu << " %\n";



cout << "========================================\n";
    cout << "         SYSTEM INFO TEST                \n";
    cout << "========================================\n\n";

    SystemInfo sys;

    cout << "[BIOS]\n";
    cout << "Vendor  : " << sys.get_bios_vendor() << "\n";
    cout << "Version : " << sys.get_bios_version() << "\n";
    cout << "Date    : " << sys.get_bios_date() << "\n\n";

    cout << "[Motherboard]\n";
    cout << "Manufacturer : " << sys.get_motherboard_manufacturer() << "\n";
    cout << "Model        : " << sys.get_motherboard_model() << "\n";


cout << "========================================\n";
    cout << "         User INFO TEST                \n";
    cout << "========================================\n\n";


 UserInfo userInfo;
    
    cout << "====== User Information ======" << endl;
    cout << "Username: " << userInfo.get_username() << endl;
    cout << "Computer Name: " << userInfo.get_computer_name() << endl;
    cout << "Domain Name: " << userInfo.get_domain_name() << endl;
    cout << "User Groups: " << userInfo.get_user_groups() << endl;









    return 0;
}