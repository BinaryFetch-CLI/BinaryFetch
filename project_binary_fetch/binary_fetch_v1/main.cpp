// main.cpp (AsciiArt integrated, incremental/real-time printing)
// Option A: ASCII art on left, info printed line-by-line as produced.

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

    // getters for real-time printing
    int getHeight() const { return height; }
    int getMaxWidth() const { return maxWidth; }
    int getSpacing() const { return spacing; }
    const std::string& getLine(int i) const { return artLines[i]; }
    int getLineWidth(int i) const { return (i >= 0 && i < (int)artWidths.size()) ? artWidths[i] : 0; }

private:
    std::vector<std::string> artLines;
    std::vector<int> artWidths;
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
    artWidths.clear();
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
        artWidths.push_back((int)vlen);
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
    artWidths.clear();
    maxWidth = 0;
    height = 0;
}

// ---------------- LivePrinter (incremental printing) ----------------
//
// This prints a single info line immediately next to the corresponding ASCII-art line.
// Call push(infoLine) for every logical output line in the order you want them shown.
// After all lines are pushed, call finish() to print remaining art lines (if any).
//

class LivePrinter {
public:
    LivePrinter(const AsciiArt& artRef) : art(artRef), index(0) {}

    // push one info line; prints the art line at current index (or padding) + spacing + info + newline
    void push(const std::string& infoLine) {
        printArtAndPad();
        if (!infoLine.empty()) std::cout << infoLine;
        std::cout << '\n';
        std::cout.flush();
        ++index;
    }

    // same as push but for blank info (just prints art line)
    void pushBlank() {
        printArtAndPad();
        std::cout << '\n';
        std::cout.flush();
        ++index;
    }

    // When no more info lines remain, print leftover art lines
    void finish() {
        while (index < art.getHeight()) {
            printArtAndPad();
            std::cout << '\n';
            ++index;
        }
    }

private:
    const AsciiArt& art;
    int index;

    void printArtAndPad() {
        int artH = art.getHeight();
        int maxW = art.getMaxWidth();
        int spacing = art.getSpacing();

        if (index < artH) {
            const std::string& a = art.getLine(index);
            std::cout << a;
            int curW = art.getLineWidth(index);
            if (curW < maxW) std::cout << std::string(maxW - curW, ' ');
        }
        else {
            // no art line here, print blank area
            if (maxW > 0) std::cout << std::string(maxW, ' ');
        }

        if (spacing > 0) std::cout << std::string(spacing, ' ');
    }
};

// ---------------- small helper to push multi-line formatted strings ----------------
static void pushFormattedLines(LivePrinter& lp, const std::string& s) {
    std::istringstream iss(s);
    std::string line;
    while (std::getline(iss, line)) {
        // remove trailing '\r' if present
        if (!line.empty() && line.back() == '\r') line.pop_back();
        lp.push(line);
    }
}

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

    // ---------------- Now stream info line-by-line ----------------

    // Header
    lp.push(""); // keep art line 0 + blank info (or you can push a title)
    lp.push("_>> BinaryFetch____________________________________________________");

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

    // Storage Info (detailed)
    {
        const auto& all_disks = storage.get_all_storage_info();
        if (all_disks.empty()) {
            lp.push("--- Storage Info ---");
            lp.push("No drives detected.");
        }
        else {
            cout << endl; 
            lp.push("------------------------ STORAGE SUMMARY --------------------------");
            for (const auto& d : all_disks) {
                auto fmt_storage = [](const std::string& s) {
                    std::ostringstream oss;
                    double v = 0.0;
                    try { v = stod(s); }
                    catch (...) { v = 0.0; }
                    oss << std::fixed << std::setprecision(2)
                        << std::setw(7) << std::right << std::setfill(' ')
                        << v;
                    return oss.str();
                    };

                std::ostringstream ss;
                ss << d.storage_type << " " << d.drive_letter
                    << " [ (Used) " << fmt_storage(d.used_space)
                    << " GiB / " << fmt_storage(d.total_space)
                    << " GiB " << d.used_percentage
                    << " - " << d.file_system << " "
                    << (d.is_external ? "Ext]" : "Int]");
                lp.push(ss.str());
            }

            lp.push("");
            lp.push("---------------------- DISK PERFORMANCE & DETAILS ----------------------");

            for (const auto& d : all_disks) {
                auto fmt_speed = [](const std::string& s) {
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

                std::ostringstream ss;
                ss << d.drive_letter << " [ Read: ("
                    << fmt_speed(d.read_speed)
                    << " MB/s) | Write: ("
                    << fmt_speed(d.write_speed)
                    << " MB/s) | " << d.serial_number
                    << (d.is_external ? " Ext ]" : " Int ]");
                lp.push(ss.str());
            }

            lp.push("");
            lp.push("----------------- DISK PERFORMANCE & DETAILS (Predicted) ---------------");

            for (const auto& d : all_disks) {
                auto fmt_speed = [](const std::string& s) {
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

                std::ostringstream ss;
                ss << d.drive_letter << " [ Read: ("
                    << fmt_speed(d.predicted_read_speed)
                    << " MB/s) | Write: ("
                    << fmt_speed(d.predicted_write_speed)
                    << " MB/s) | " << d.serial_number
                    << (d.is_external ? " Ext ]" : " Int ]");
                lp.push(ss.str());
            }
        }
    }

    // Network (Compact + Extra)
    { 
        
        cout << endl;
            lp.push("--- Network Info (Compact + Extra) ---");
            {
                std::ostringstream ss; ss << "Network Name: " << c_net.get_network_name(); lp.push(ss.str());
            }
            {
                std::ostringstream ss; ss << "Network Type: " << c_net.get_network_type(); lp.push(ss.str());
            }
            {
                std::ostringstream ss; ss << "IP (compact): " << c_net.get_network_ip(); lp.push(ss.str());
            }

        
       
    }

    // Audio & Power
    {
        cout << endl;
        lp.push("--- Audio & Power Info ---");

        // Use full audio (ExtraInfo) - it prints directly
        ExtraInfo audio;

        // Redirect cout to a stringstream temporarily
        std::ostringstream oss;
        std::streambuf* oldCout = std::cout.rdbuf(oss.rdbuf());

        audio.get_audio_devices(); // prints to oss

        // Restore cout
        std::cout.rdbuf(oldCout);

        // Push captured lines to LivePrinter
        std::istringstream iss(oss.str());
        std::string line;
        while (std::getline(iss, line)) {
            lp.push(line);
        }

        // Power info (ExtraInfo already prints directly in your class)
        std::ostringstream ossPower;
        oldCout = std::cout.rdbuf(ossPower.rdbuf());

        audio.get_power_status();

        std::cout.rdbuf(oldCout);

        std::istringstream issPower(ossPower.str());
        while (std::getline(issPower, line)) {
            lp.push(line);
        }
    }



    // OS Info
    {   
        cout << endl;
        lp.push("--- OS Info ---");
        {
            std::ostringstream ss; ss << "Name: " << os.GetOSName(); lp.push(ss.str());
        }
        {
            std::ostringstream ss; ss << "Version: " << os.GetOSVersion(); lp.push(ss.str());
        }
        {
            std::ostringstream ss; ss << "Architecture: " << os.GetOSArchitecture(); lp.push(ss.str());
        }
        {
            std::ostringstream ss; ss << "Kernel: " << os.get_os_kernel_info(); lp.push(ss.str());
        }
        {
            std::ostringstream ss; ss << "Uptime: " << os.get_os_uptime(); lp.push(ss.str());
        }
        {
            std::ostringstream ss; ss << "Install Date: " << os.get_os_install_date(); lp.push(ss.str());
        }
        {
            std::ostringstream ss; ss << "Serial: " << os.get_os_serial_number(); lp.push(ss.str());
        }
    }

    // CPU Info
    {   
        cout << endl;
        lp.push("--- CPU Info ---");
        {
            std::ostringstream ss; ss << "Brand: " << cpu.get_cpu_info(); lp.push(ss.str());
        }
        {
            std::ostringstream ss; ss << "Utilization: " << cpu.get_cpu_utilization() << "%"; lp.push(ss.str());
        }
        {
            std::ostringstream ss; ss << "Speed: " << cpu.get_cpu_speed(); lp.push(ss.str());
        }
        {
            std::ostringstream ss; ss << "Base Speed: " << cpu.get_cpu_base_speed(); lp.push(ss.str());
        }
        {
            std::ostringstream ss; ss << "Cores: " << cpu.get_cpu_cores(); lp.push(ss.str());
        }
        {
            std::ostringstream ss; ss << "Logical Processors: " << cpu.get_cpu_logical_processors(); lp.push(ss.str());
        }
        {
            std::ostringstream ss; ss << "Sockets: " << cpu.get_cpu_sockets(); lp.push(ss.str());
        }
        {
            std::ostringstream ss; ss << "Virtualization: " << cpu.get_cpu_virtualization(); lp.push(ss.str());
        }
        {
            std::ostringstream ss; ss << "L1 Cache: " << cpu.get_cpu_l1_cache(); lp.push(ss.str());
        }
        {
            std::ostringstream ss; ss << "L2 Cache: " << cpu.get_cpu_l2_cache(); lp.push(ss.str());
        }
        {
            std::ostringstream ss; ss << "L3 Cache: " << cpu.get_cpu_l3_cache(); lp.push(ss.str());
        }
    }

    // GPU Info (detailed)
    {
        cout << endl;
        auto all_gpu_info = obj_gpu.get_all_gpu_info();
        if (all_gpu_info.empty()) {
            lp.push("--- GPU Info ---");
            lp.push("No GPU detected.");
        }
        else {
            lp.push("--- GPU Info ---");
            for (size_t i = 0; i < all_gpu_info.size(); ++i) {
                auto& g = all_gpu_info[i];
                {
                    std::ostringstream ss; ss << "GPU " << (i + 1) << ":"; lp.push(ss.str());
                }
                {
                    std::ostringstream ss; ss << "  Name: " << g.gpu_name; lp.push(ss.str());
                }
                {
                    std::ostringstream ss; ss << "  Memory: " << g.gpu_memory; lp.push(ss.str());
                }
                {
                    std::ostringstream ss; ss << "  Usage: " << g.gpu_usage << "%"; lp.push(ss.str());
                }
                {
                    std::ostringstream ss; ss << "  Vendor: " << g.gpu_vendor; lp.push(ss.str());
                }
                {
                    std::ostringstream ss; ss << "  Driver Version: " << g.gpu_driver_version; lp.push(ss.str());
                }
                {
                    std::ostringstream ss; ss << "  Temperature: " << g.gpu_temperature << " C"; lp.push(ss.str());
                }
                {
                    std::ostringstream ss; ss << "  Core Count: " << g.gpu_core_count; lp.push(ss.str());
                }
            }

            auto primary = detailed_gpu_info.primary_gpu_info();
            {
                std::ostringstream ss; ss << "Primary GPU Details:"; lp.push(ss.str());
            }
            {
                std::ostringstream ss; ss << "  Name: " << primary.name; lp.push(ss.str());
            }
            {
                std::ostringstream ss; ss << "  VRAM: " << primary.vram_gb << " GiB"; lp.push(ss.str());
            }
            {
                std::ostringstream ss; ss << "  Frequency: " << primary.frequency_ghz << " GHz"; lp.push(ss.str());
            }
        }
    }

    // Display Info
    {
        cout << endl;
        lp.push("--- Display Info ---");
        auto monitors = display.get_all_displays();
        if (monitors.empty()) {
            lp.push("No monitors detected.");
        }
        else {
            for (size_t i = 0; i < monitors.size(); ++i) {
                auto& m = monitors[i];
                {
                    std::ostringstream ss; ss << "Monitor " << (i + 1) << ":"; lp.push(ss.str());
                }
                {
                    std::ostringstream ss; ss << "  Brand: " << m.brand_name; lp.push(ss.str());
                }
                {
                    std::ostringstream ss; ss << "  Resolution: " << m.resolution; lp.push(ss.str());
                }
                {
                    std::ostringstream ss; ss << "  Refresh Rate: " << m.refresh_rate << " Hz"; lp.push(ss.str());
                }
            }
        }
    }

    // BIOS & Motherboard Info
    {
        cout << endl;
        lp.push("--- BIOS & Motherboard Info ---");
        {
            std::ostringstream ss; ss << "Bios Vendor: " << sys.get_bios_vendor(); lp.push(ss.str());
        }
        {
            std::ostringstream ss; ss << "Bios Version: " << sys.get_bios_version(); lp.push(ss.str());
        }
        {
            std::ostringstream ss; ss << "Bios Date: " << sys.get_bios_date(); lp.push(ss.str());
        }
        {
            std::ostringstream ss; ss << "Motherboard Model: " << sys.get_motherboard_model(); lp.push(ss.str());
        }
        {
            std::ostringstream ss; ss << "Motherboard Manufacturer: " << sys.get_motherboard_manufacturer(); lp.push(ss.str());
        }
    }

    // User Info
    {   
        cout << endl;
        lp.push("--- User Info ---");
        {
            std::ostringstream ss; ss << "Username: " << user.get_username(); lp.push(ss.str());
        }
        {
            std::ostringstream ss; ss << "Computer Name: " << user.get_computer_name(); lp.push(ss.str());
        }
        {
            std::ostringstream ss; ss << "Domain: " << user.get_domain_name(); lp.push(ss.str());
        }
        {
            std::ostringstream ss; ss << "Groups: " << user.get_user_groups(); lp.push(ss.str());
        }
    }

    // Performance Info
    {    
        cout << endl;
        lp.push("--- Performance Info ---");
        {
            std::ostringstream ss; ss << "System Uptime: " << perf.get_system_uptime(); lp.push(ss.str());
        }
        {
            std::ostringstream ss; ss << "CPU Usage: " << perf.get_cpu_usage_percent() << "%"; lp.push(ss.str());
        }
        {
            std::ostringstream ss; ss << "RAM Usage: " << perf.get_ram_usage_percent() << "%"; lp.push(ss.str());
        }
        {
            std::ostringstream ss; ss << "Disk Usage: " << perf.get_disk_usage_percent() << "%"; lp.push(ss.str());
        }
        {
            std::ostringstream ss; ss << "GPU Usage: " << perf.get_gpu_usage_percent() << "%"; lp.push(ss.str());
        }
    }

    // ---------------- End of info lines ----------------

    // Print remaining ASCII art lines (if art is taller than info)
    lp.finish();

    std::cout << std::endl;
    return 0;
}
