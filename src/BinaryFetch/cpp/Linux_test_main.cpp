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

using namespace std;

int main() {
    cout << "========================================================\n";
    cout << "          BINARYFETCH LINUX COMPACT MODULES TEST        \n";
    cout << "========================================================\n\n";

    // 1. TimeInfo
    cout << "--- [TimeInfo] ---\n";
    TimeInfo timeInfo;
    cout << "Local Time: " << timeInfo.getHour() << ":" << timeInfo.getMinute() << ":" << timeInfo.getSecond() << "\n";
    cout << "Date: " << timeInfo.getDayName() << ", " << timeInfo.getMonthName() << " " << timeInfo.getDay() << ", " << timeInfo.getYearNumber() << "\n";
    cout << "Leap Year: " << timeInfo.getLeapYear() << " | Week Number: " << timeInfo.getWeekNumber() << "\n\n";

    // 2. CompactUser
    cout << "--- [CompactUser] ---\n";
    CompactUser user;
    cout << "Username: " << user.getUsername() << "\n";
    cout << "Domain: " << user.getDomain() << "\n";
    cout << "Admin Status: " << user.isAdmin() << "\n\n";

    // 3. CompactOS
    cout << "--- [CompactOS] ---\n";
    CompactOS os;
    cout << "OS Name: " << os.getOSName() << "\n";
    cout << "OS Kernel/Build: " << os.getOSBuild() << "\n";
    cout << "Architecture: " << os.getArchitecture() << "\n";
    cout << "Uptime: " << os.getUptime() << "\n\n";

    // 4. CompactCPU
    cout << "--- [CompactCPU] ---\n";
    CompactCPU cpu;
    cout << "CPU Name: " << cpu.getCPUName() << "\n";
    cout << "Cores: " << cpu.getCPUCores() << " | Threads: " << cpu.getCPUThreads() << "\n";
    cout << "Clock Speed: " << cpu.getClockSpeed() << " GHz\n";
    cout << "CPU Utilization: " << cpu.getUsagePercent() << "%\n\n";

    // 5. CompactMemory
    cout << "--- [CompactMemory] ---\n";
    CompactMemory memory;
    cout << "Total Memory: " << memory.get_total_memory() << " GB\n";
    cout << "Free Memory: " << memory.get_free_memory() << " GB\n";
    cout << "Used Memory: " << memory.get_used_memory_percent() << "%\n";
    cout << "RAM Slots (Used/Available): " << memory.memory_slot_used() << "/" << memory.memory_slot_available() << "\n\n";

    // 6. CompactPerformance
    cout << "--- [CompactPerformance] ---\n";
    CompactPerformance perf;
    cout << "CPU Usage: " << perf.getCPUUsage() << "%\n";
    cout << "RAM Usage: " << perf.getRAMUsage() << "%\n";
    cout << "Disk Usage: " << perf.getDiskUsage() << "%\n";
    cout << "GPU Usage: " << perf.getGPUUsage() << "%\n\n";

    // 7. CompactGPU
    cout << "--- [CompactGPU] ---\n";
    cout << "GPU Name: " << CompactGPU::getGPUName() << "\n";
    cout << "VRAM: " << CompactGPU::getVRAMGB() << " GB\n";
    cout << "Usage: " << CompactGPU::getGPUUsagePercent() << "%\n";
    cout << "Frequency: " << CompactGPU::getGPUFrequency() << "\n";
    cout << "Temperature: " << CompactGPU::getGPUTemperature() << " C\n\n";

    // 8. CompactScreen
    cout << "--- [CompactScreen] ---\n";
    CompactScreen screen;
    auto screens = screen.getScreens();
    cout << "Detected Screens: " << screens.size() << "\n";
    for (size_t i = 0; i < screens.size(); ++i) {
        cout << "  Screen " << i + 1 << ": " << screens[i].name << "\n";
        cout << "    Resolution: " << screens[i].current_width << "x" << screens[i].current_height 
             << " (Native: " << screens[i].native_width << "x" << screens[i].native_height << ")\n";
        cout << "    Refresh Rate: " << screens[i].refresh_rate << " Hz\n";
        cout << "    Scale Percent: " << screens[i].scale_percent << "% (" << screens[i].scale_mul << ")\n";
        cout << "    Upscaling Status: " << screens[i].upscale << "\n";
    }
    cout << "\n";

    // 9. CompactSystem
    cout << "--- [CompactSystem] ---\n";
    CompactSystem systemObj;
    cout << "BIOS: " << systemObj.getBIOSInfo() << "\n";
    cout << "Motherboard: " << systemObj.getMotherboardInfo() << "\n\n";

    // 10. CompactAudio
    cout << "--- [CompactAudio] ---\n";
    CompactAudio audio;
    cout << "Active Output: " << audio.active_audio_output() << " (" << audio.active_audio_output_status() << ")\n";
    cout << "Active Input: " << audio.active_audio_input() << " (" << audio.active_audio_input_status() << ")\n\n";

    // 11. CompactNetwork
    cout << "--- [CompactNetwork] ---\n";
    CompactNetwork network;
    cout << "Network IP: " << network.get_network_ip() << "\n";
    cout << "Network Type: " << network.get_network_type() << "\n";
    cout << "Network Name: " << network.get_network_name() << "\n\n";

    // 12. DiskInfo
    cout << "--- [DiskInfo] ---\n";
    DiskInfo disk;
    auto diskUsages = disk.getAllDiskUsage();
    auto diskCapacities = disk.getDiskCapacity();
    cout << "Disk Mount Points & Usage:\n";
    for (size_t i = 0; i < diskUsages.size(); ++i) {
        cout << "  Mount: " << diskUsages[i].first 
             << " | Usage: " << diskUsages[i].second << "%";
        // Find capacity matching this mount
        for (const auto& cap : diskCapacities) {
            if (cap.first == diskUsages[i].first) {
                cout << " | Capacity: " << cap.second << " GB";
                break;
            }
        }
        cout << "\n";
    }
    cout << "\n";

    















StorageInfo storage;
 
    cout << "---------------- DISK PERFORMANCE & DETAILS ----------------\n";
    storage.process_storage_info([](const storage_data& d) {
        cout << d.drive_letter << " [ Read: " << setw(8) << d.read_speed
             << " MB/s | Write: " << setw(8) << d.write_speed
             << " MB/s ]  " << d.file_system
             << "  SN-" << d.serial_number
             << "  " << (d.is_external ? "Ext" : "Int") << " ]\n";
    });
 
    cout << "\n---------------- DISK PERFORMANCE & DETAILS (Predicted) ----------------\n";
    for (const auto& d : storage.get_all_storage_info()) {
        cout << d.drive_letter << " [ Read: " << setw(8) << d.predicted_read_speed
             << " MB/s | Write: " << setw(8) << d.predicted_write_speed
             << " MB/s ]  " << d.storage_type
             << "  SN-" << d.serial_number
             << "  " << (d.is_external ? "Ext" : "Int") << " ]\n";
    }


















    return 0;
}