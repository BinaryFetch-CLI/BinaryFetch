// main.cpp (AsciiArt integrated)

#include <iostream>
#include <iomanip>        // Formatting utilities (setw, precision)
#include <vector>
#include <functional>
#include <sstream>        // For string stream operations
#include <fstream>
#include <string>
#include <regex>
#include <locale>
#include <codecvt>
#include <cwchar>

#ifdef _WIN32
#include <windows.h>
#endif

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


// ------------------ UI / Visual Components ------------------
// AsciiArt is implemented inline below (no separate AsciiArt.cpp/h required)

using namespace std;

// ---------------- Helper functions for AsciiArt ----------------

// Strip ANSI escape sequences (like "\x1b[31m") from string
static std::string stripAnsiSequences(const std::string& s) {
    static const std::regex ansi_re("\x1B\\[[0-9;]*[A-Za-z]");
    return std::regex_replace(s, ansi_re, "");
}

// Convert UTF-8 string to wstring
static std::wstring utf8_to_wstring(const std::string& s) {
    try {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        return conv.from_bytes(s);
    }
    catch (...) {
        std::wstring w;
        w.reserve(s.size());
        for (unsigned char c : s) w.push_back(static_cast<wchar_t>(c));
        return w;
    }
}

// Return displayed width of a wide character
static int char_display_width(wchar_t wc) {
#if !defined(_WIN32)
    int w = wcwidth(wc);
    return (w < 0) ? 0 : w;
#else
    if (wc == 0) return 0;
    if (wc < 0x1100) return 1;
    if ((wc >= 0x1100 && wc <= 0x115F) ||
        (wc >= 0x2E80 && wc <= 0xA4CF) ||
        (wc >= 0xAC00 && wc <= 0xD7A3) ||
        (wc >= 0xF900 && wc <= 0xFAFF) ||
        (wc >= 0xFE10 && wc <= 0xFE19) ||
        (wc >= 0xFE30 && wc <= 0xFE6F) ||
        (wc >= 0xFF00 && wc <= 0xFF60) ||
        (wc >= 0x20000 && wc <= 0x2FFFD) ||
        (wc >= 0x30000 && wc <= 0x3FFFD))
        return 2;
    return 1;
#endif
}

// Return visible width of UTF-8 string
static size_t visible_width(const std::string& s) {
    const std::string cleaned = stripAnsiSequences(s);
    const std::wstring w = utf8_to_wstring(cleaned);
    size_t width = 0;
    for (size_t i = 0; i < w.size(); ++i) width += static_cast<size_t>(char_display_width(w[i]));
    return width;
}

// ---------------- Sanitize leading invisible characters ----------------
static void sanitizeLeadingInvisible(std::string& s) {
    // Remove UTF-8 BOM (EF BB BF)
    if (s.size() >= 3 &&
        (unsigned char)s[0] == 0xEF &&
        (unsigned char)s[1] == 0xBB &&
        (unsigned char)s[2] == 0xBF) {
        s.erase(0, 3);
    }

    // Remove leading zero-width spaces (U+200B -> E2 80 8B)
    while (s.size() >= 3 &&
        (unsigned char)s[0] == 0xE2 &&
        (unsigned char)s[1] == 0x80 &&
        (unsigned char)s[2] == 0x8B) {
        s.erase(0, 3);
    }
}

// ---------------- AsciiArt class ----------------

class AsciiArt {
public:
    AsciiArt();
    bool loadFromFile(const std::string& filename);
    bool isEnabled() const;
    void setEnabled(bool enable);
    void clear();
    void printWithArt(std::function<void()> infoPrinter);

private:
    std::vector<std::string> artLines;
    int maxWidth;
    int height;
    bool enabled;
    int spacing;
};

AsciiArt::AsciiArt() : maxWidth(0), height(0), enabled(true), spacing(2) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
}

bool AsciiArt::loadFromFile(const std::string& filename) {
    artLines.clear();
    std::ifstream file(filename);
    if (!file.is_open()) {
        enabled = false;
        maxWidth = 0;
        height = 0;
        return false;
    }

    std::string line;
    maxWidth = 0;
    bool isFirstLine = true;

    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();

        // Only sanitize the first line for BOM / zero-width characters
        if (isFirstLine) {
            sanitizeLeadingInvisible(line);
            isFirstLine = false;
        }

        artLines.push_back(line);
        size_t vlen = visible_width(line);
        if (static_cast<int>(vlen) > maxWidth) maxWidth = static_cast<int>(vlen);
    }

    height = static_cast<int>(artLines.size());
    enabled = !artLines.empty();
    return enabled;
}

bool AsciiArt::isEnabled() const {
    return enabled;
}

void AsciiArt::setEnabled(bool enable) {
    enabled = enable;
}

void AsciiArt::clear() {
    artLines.clear();
    maxWidth = 0;
    height = 0;
}

void AsciiArt::printWithArt(std::function<void()> infoPrinter) {
    if (!enabled) {
        infoPrinter();
        return;
    }

    // Capture infoPrinter output
    std::ostringstream oss;
    std::streambuf* oldBuf = std::cout.rdbuf();
    std::cout.rdbuf(oss.rdbuf());
    infoPrinter();
    std::cout.rdbuf(oldBuf);

    std::istringstream iss(oss.str());
    std::vector<std::string> infoLines;
    std::string line;
    while (std::getline(iss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        infoLines.push_back(line);
    }

    int artHeight = height;
    int infoHeight = static_cast<int>(infoLines.size());
    int maxHeight = (artHeight > infoHeight) ? artHeight : infoHeight;

    // Compute max visible width for info
    int maxInfoWidth = 0;
    for (size_t i = 0; i < infoLines.size(); ++i) {
        size_t w = visible_width(infoLines[i]);
        if (static_cast<int>(w) > maxInfoWidth) maxInfoWidth = static_cast<int>(w);
    }

    for (int i = 0; i < maxHeight; ++i) {
        // ASCII art
        if (i < artHeight) {
            std::cout << artLines[i];
            size_t curW = visible_width(artLines[i]);
            if (static_cast<int>(curW) < maxWidth)
                std::cout << std::string(maxWidth - curW, ' ');
        }
        else {
            std::cout << std::string(maxWidth, ' ');
        }

        // spacing
        if (spacing > 0) std::cout << std::string(spacing, ' ');

        // info
        if (i < infoHeight) {
            std::cout << infoLines[i];
            size_t curW = visible_width(infoLines[i]);
            if (static_cast<int>(curW) < maxInfoWidth)
                std::cout << std::string(maxInfoWidth - curW, ' ');
        }
        else {
            std::cout << std::string(maxInfoWidth, ' ');
        }

        std::cout << "\n";
    }
}

// ------------------ main (your original main.cpp content) ------------------

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


        /*================[ Minimal OS ]==================*/

        cout << "[OS]  -> " << c_os.getOSName()
            << c_os.getOSBuild()
            << " (" << c_os.getArchitecture() << ")"
            << " (uptime: " << c_os.getUptime() << ")" << endl;


        /*================[ minimal CPU ]==================*/

        cout << "[CPU] -> " << c_cpu.getCPUName() << " ("
            << c_cpu.getCPUCores() << "C/"
            << c_cpu.getCPUThreads() << "T)"
            << std::fixed << std::setprecision(2)
            << " @ " << c_cpu.getClockSpeed() << " GHz " << endl;

        /*================[ minimal SCREEN ]==================*/

        auto screens = c_screen.get_screens();
        int i = 1;
        for (const auto& s : screens) {
            cout << "[Display " << i++ << "] -> "
                << s.brand_name << " (" << s.resolution << ") @"
                << s.refresh_rate << "Hz\n";
        }

        /*================[ minimal MEMORY ]==================*/

        cout << "[Memory] -> " << "(total: " << c_memory.get_total_memory() << " GB)"
            << " (free: " << c_memory.get_free_memory() << " GB)"
            << " ( " << c_memory.get_used_memory_percent() << "% ) " << endl;

        /*================[ MINIMAL AUDIO ]==================*/

        cout << "[Audio Input] -> " << c_audio.active_audio_input()
            << c_audio.active_audio_input_status() << endl;

        cout << "[Audio Output] -> " << c_audio.active_audio_output()
            << c_audio.active_audio_output_status() << endl;

        /*================[ MINIMAL BIOS ]==================*/

        cout << "[BIOS] -> " << c_system.getBIOSInfo() << endl;
        cout << "[Motherboard] -> " << c_system.getMotherboardInfo() << endl;

        /*================[ MINIMAL GPU ]==================*/

        cout << "[GPU] -> " << c_gpu.getGPUName()
            << " (" << c_gpu.getGPUUsagePercent() << "%)"
            << " (" << c_gpu.getVRAMGB() << " GB)"
            << " (@" << c_gpu.getGPUFrequency() << ") " << endl;

        /*================[ MINIMAL PERFORMANCE ]==================*/

        cout << "[Performance] -> "
            << "(CPU: " << c_perf.getCPUUsage() << "%) "
            << "(GPU: " << c_perf.getGPUUsage() << "%) "
            << "(RAM: " << c_perf.getRAMUsage() << "%) "
            << "(Disk: " << c_perf.getDiskUsage() << "%) " << endl;

        /*================[ MINIMAL USER INFO ]==================*/

        cout << "[User] -> @" << c_user.getUsername()
            << " -> (Domain: " << c_user.getDomain()
            << ") -> (Type: " << c_user.isAdmin() << ")" << endl;

        /*================[ MINIMAL NETWORK INFO ]==================*/

        cout << "[network] -> " << "(Name: " << c_net.get_network_name()
            << ") (Type: " << c_net.get_network_type()
            << ") (ip: " << c_net.get_network_ip() << ") " << endl;


        /*================[ MINIMAL ALL DISK USAGE ]==================*/

        auto disks = disk.getAllDiskUsage();

        // print disk usage in percentage
        //e.g. [Disk] -> (C: 97%) (D: 45%) (G: 91%) (H: 58%) (I: 62%) (Y: 22%)
        std::cout << "[Disk] -> ";
        for (const auto& d : disks) {
            std::cout << "(" << d.first[0] << ": "
                << std::fixed << std::setprecision(1)
                << d.second << "%) ";
        }
        std::cout << std::endl;

        // print disk capacity in GB
        //  e.g. [Disk Cap] -> (C-237GB)(D-465GB)(G-112GB)(H-931GB)(I-110GB)(Y-465GB)
        auto caps = disk.getDiskCapacity();
        std::cout << "[Disk Cap] -> ";
        for (const auto& c : caps) {
            std::cout << "(" << c.first[0] << "-" << c.second << "GB)";
        }
        std::cout << std::endl;


        // ---------------- FULL DETAILED INFO (A order) ----------------
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

        // --- Storage Info --- inside main.cpp
        {
            const auto& all_disks = storage.get_all_storage_info();
            if (all_disks.empty()) {
                cout << "--- Storage Info ---\nNo drives detected.\n\n";
            }
            else {
                cout << "------------------------ STORAGE SUMMARY --------------------------\n";
                for (const auto& d : all_disks) {
                    auto fmt_storage = [](const std::string& s) {
                        std::ostringstream oss;
                        double v = stod(s);
                        oss << std::fixed << std::setprecision(2)
                            << std::setw(7) << std::right << std::setfill(' ')
                            << v;
                        return oss.str();
                        };

                    // Print: SSD Disk (C:)
                    cout << d.storage_type << " " << d.drive_letter
                        << " [ (Used) " << fmt_storage(d.used_space)
                        << " GiB / " << fmt_storage(d.total_space)
                        << " GiB " << d.used_percentage
                        << " - " << d.file_system << " ";

                    // Only print Int/Ext here
                    if (d.is_external)
                        cout << "Ext]";
                    else
                        cout << "Int]";

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

                    auto fmt_speed = [](const std::string& s)
                        {
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


    return 0;
}
