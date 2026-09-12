

//Welcome to BinaryFetch entry point (main.cpp) 

#include <iostream>       // Standard input/output stream (cin, cout) 
#include <iomanip>        // Formatting utilities (setw, precision, setfill) 
#include <vector>         // Dynamic array container 
#include <functional>     // Function objects and wrappers (function) 
#include <sstream>        // String stream operations for parsing/conversion 
#include <fstream>        // File stream operations (reading/writing files) 
#include <string>         // Standard string class and methods 
#include <regex>          // Regular expressions for pattern matching 
#include <algorithm>



#ifdef _WIN32
     #include <windows.h>      // Core Windows API functions (handles, processes) 
     #include <shlobj.h>       // Shell object functions (folder paths, UI) 
     #include <direct.h>       // Directory and file handling functions (_mkdir, _chdir) 
     #include <comdef.h>       // Native C++ compiler COM support 
     #include <Wbemidl.h>      // WMI (Windows Management Instrumentation) interfaces 
#endif 

// ASCII Art functionality
#include "AsciiArt.h" // main.cpp (AsciiArt separated into header and implementation files)
#include "core/config_management.h"


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
#include "TimeInfo.h"           //returns current time info (second, minute, hour, day, week, month, year, leap year, etc)



#include "nlohmann/json.hpp" 
using json = nlohmann::json;
using namespace std;




// Builds a customizable performance bar using the metric's JSON settings.
std::string makeVisualizer(float percentage, const ConfigManager& config,
                           const std::string& module, const std::string& field)
{
    const std::string path = "fields." + field + ".visualizer.";

    if (!config.getNestedBool(module, path + "enabled", false))
        return "";

    int width = config.getNestedInt(module, path + "width", 0);

    std::string filled = config.getNestedString(module, path + "filled", "");
    std::string empty  = config.getNestedString(module, path + "empty", "");
    std::string left   = config.getNestedString(module, path + "left", "");
    std::string right  = config.getNestedString(module, path + "right", "");

    std::string filledColor = config.getNestedColor(module, path + "filled_color", "white");
    std::string emptyColor  = config.getNestedColor(module, path + "empty_color", "white");
    std::string leftColor   = config.getNestedColor(module, path + "left_color", "white");
    std::string rightColor  = config.getNestedColor(module, path + "right_color", "white");

    if (width <= 0 || filled.empty() || empty.empty())
        return "";

    percentage = std::clamp(percentage, 0.0f, 100.0f);

    int filledCount = static_cast<int>((percentage / 100.0f) * width);
    int emptyCount = width - filledCount;

    std::string result = leftColor + left;

    result += filledColor;

    for (int i = 0; i < filledCount; ++i)
        result += filled;

    result += emptyColor;

    for (int i = 0; i < emptyCount; ++i)
        result += empty;

    result += rightColor + right;

    return result + config.getResetColor();
}




// Runs field lambdas from `fields` in the order specified by `order`.
// Each order entry is trimmed of trailing spaces to find the matching
// field key; any trailing spaces present in the raw JSON entry are
// re-appended to `ss` after the field runs, letting the JSON "order"
// array control inter-field spacing directly (e.g. "name " adds one
// space after the name field, "name  " adds two, "name" adds none).
void runOrderedFields(const std::vector<std::string>& order,
                       const std::map<std::string, std::function<void()>>& fields,
                       std::ostringstream& ss)
{
    for (const auto& rawKey : order) {
        size_t endPos = rawKey.find_last_not_of(' ');
        std::string key = (endPos == std::string::npos) ? "" : rawKey.substr(0, endPos + 1);
        std::string trailingSpaces = (endPos == std::string::npos) ? rawKey : rawKey.substr(endPos + 1);

        auto it = fields.find(key);
        if (it != fields.end()) {
            it->second();
            if (!trailingSpaces.empty()) ss << trailingSpaces;
        }
    }
}


int main(){

    
   #ifdef _WIN32
          SetConsoleOutputCP(CP_UTF8); // UTF-8 output on Windows console
   #endif


        // SIMPLIFIED ASCII ART LOADING 
        // Just call loadFromFile() - it handles everything automatically!
        // - Checks C:\Users\<User>\AppData\BinaryFetch\BinaryArt.txt
        //      or, ~/.config/BinaryFetch/BinaryArt.txt
        // - If missing, create a new file named "BinaryArts.txt" then paste the 
        // default ASCII art based on distro and loads from there.
        // - User can modify their art anytime from their config folder
    AsciiArt art;
    if (!art.loadFromFile()) {
        cout << "Warning: ASCII art could not be loaded. Continuing without art.\n";
        // Program continues even if art fails to load
    }

    // CONFIG MANAGEMENT 
    // DEV_MODE = true  → load default JSON directly from project folder (fast iteration 🧪)
    // DEV_MODE = false → production: read/create C:\Users\Public\BinaryFetch\BinaryFetch_Config.json 🛰️
    //                    (self-heals from embedded EXE resource if the file is missing)
    //                    NEVER overwrites an existing user config.
    bool DEV_MODE = true; // ← set to true while developing, false before shipping
    ConfigManager config(DEV_MODE);
    string r = config.getResetColor();

	// Anyway....this is how we're allowed to print emojis in C++ console
    // cout << u8"😄 ❤️ 🎉 🚀 ⭐ 🐱 🍕 🎮 😭 🌈\n"; 


    // Create LivePrinter
    LivePrinter lp(art);


    // create objects of all classes here 
    OSInfo os;                           
    CPUInfo cpu;
    MemoryInfo ram;
    GPUInfo obj_gpu;
    DetailedGPUInfo detailed_gpu_info;
    StorageInfo storage;
    NetworkInfo net;
    UserInfo user;
    PerformanceInfo perf;
    DisplayInfo di;
    ExtraInfo extra;
    SystemInfo sys;

    CompactAudio c_audio;
    CompactOS c_os;
    CompactCPU c_cpu;
   // CompactScreen c_screen;
    CompactMemory c_memory;
    CompactSystem c_system;
    CompactGPU c_gpu;
    CompactPerformance c_perf;
    CompactUser c_user;
    CompactNetwork c_net;
    DiskInfo disk;
    TimeInfo time;

   


  std::map<std::string, std::function<void()>> sections;

// ==================== HEADER BANNER ====================
sections["header_settings"] = [&]() {
    if (!config.isEnabled("header_settings")) return;

    ostringstream ss;
    string r = config.getResetColor();
    
    // Prefix - from JSON (e.g., "~>>")
    ss << config.getColor("header_settings", "header_prefix_color" )
       << config.getPrefix("header_settings", "header_prefix" ) << r;
    
    // Title - from JSON (e.g., "BinaryFetch")
    ss << config.getColor("header_settings", "title_color")
       << config.getLabel("header_settings", "title", "") << r;
    
    // Suffix - from JSON (e.g., "-------------------------*")
    ss << config.getColor("header_settings", "header_suffix_color")
       << config.getPrefix("header_settings", "header_suffix", "") << r;
    
    lp.push(ss.str());
};




//   ██████╗ ██████╗ ███╗   ███╗██████╗  █████╗  ██████╗████████╗
//  ██╔════╝██╔═══██╗████╗ ████║██╔══██╗██╔══██╗██╔════╝╚══██╔══╝
//  ██║     ██║   ██║██╔████╔██║██████╔╝███████║██║        ██║   
//  ██║     ██║   ██║██║╚██╔╝██║██╔═══╝ ██╔══██║██║        ██║   
//  ╚██████╗╚██████╔╝██║ ╚═╝ ██║██║     ██║  ██║╚██████╗   ██║   
//   ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚═╝     ╚═╝  ╚═╝ ╚═════╝   ╚═╝   
//                       C O M P A C T   M O D U L E S


// ==================== COMPACT TIME ====================
sections["compact_date_and_time"] = [&]() {
    if (!config.isEnabled("compact_date_and_time")) return;
    TimeInfo time;
    ostringstream ss;

    // line spacing
    int spacing = config.getNestedInt("compact_date_and_time","top_line_spacing",0);
    for (int n = 0; n < spacing; n++) {lp.push("");}

    // Prefix - comes entirely from JSON (can be emoji, text, or empty)
    if (config.isFieldEnabled("compact_date_and_time", "prefixes.show")) {
        ss << config.getColor("compact_date_and_time", "prefixes.prefix_color", "")
           << config.getPrefix("compact_date_and_time", "prefixes.prefix", "") << r;
    }

    // ---- Register each orderable subsection as a named lambda ----
    std::map<std::string, std::function<void()>> fields;

    // ---------- TIME SECTION ----------
    fields["time"] = [&]() {
        if (!config.isNestedEnabled("compact_date_and_time", "time", "enabled")) return;

        ss << config.getNestedColor("compact_date_and_time", "time", "bracket", "")
           << config.getNestedString("compact_date_and_time", "time.open", "(") << r;

        if (config.isNestedEnabled("compact_date_and_time", "time", "show_label")) {
            ss << config.getNestedColor("compact_date_and_time", "time", "label", "")
               << config.getNestedString("compact_date_and_time", "time.label_text", "Time: ") << r;
        }

        bool wrote = false;

        if (config.isNestedEnabled("compact_date_and_time", "time", "show_hour")) {
            ss << config.getNestedColor("compact_date_and_time", "time", "hour", "")
               << setw(2) << setfill('0') << time.getHour() << r;
            wrote = true;
        }

        if (config.isNestedEnabled("compact_date_and_time", "time", "show_minute")) {
            if (wrote) ss << config.getNestedColor("compact_date_and_time", "time", "sep", "")
                          << config.getNestedString("compact_date_and_time", "time.sep_text", ":") << r;
            ss << config.getNestedColor("compact_date_and_time", "time", "minute", "")
               << setw(2) << setfill('0') << time.getMinute() << r;
            wrote = true;
        }

        if (config.isNestedEnabled("compact_date_and_time", "time", "show_second")) {
            if (wrote) ss << config.getNestedColor("compact_date_and_time", "time", "sep", "")
                          << config.getNestedString("compact_date_and_time", "time.sep_text", ":") << r;
            ss << config.getNestedColor("compact_date_and_time", "time", "second", "")
               << setw(2) << setfill('0') << time.getSecond() << r;
        }

        ss << config.getNestedColor("compact_date_and_time", "time", "bracket", "")
           << config.getNestedString("compact_date_and_time", "time.close", ")") << r;
    };

    // ---------- DATE SECTION ----------
    fields["date"] = [&]() {
        if (!config.isNestedEnabled("compact_date_and_time", "date", "enabled")) return;

        ss << config.getNestedColor("compact_date_and_time", "date", "bracket", "")
           << config.getNestedString("compact_date_and_time", "date.open", "(") << r;

        if (config.isNestedEnabled("compact_date_and_time", "date", "show_label")) {
            ss << config.getNestedColor("compact_date_and_time", "date", "label", "")
               << config.getNestedString("compact_date_and_time", "date.label_text", "Date: ") << r;
        }

        bool wrote = false;

        if (config.isNestedEnabled("compact_date_and_time", "date", "show_day")) {
            ss << config.getNestedColor("compact_date_and_time", "date", "day", "")
               << setw(2) << setfill('0') << time.getDay() << r;
            wrote = true;
        }

        if (config.isNestedEnabled("compact_date_and_time", "date", "show_month_name")) {
            if (wrote) ss << config.getNestedColor("compact_date_and_time", "date", "sep", "")
                          << config.getNestedString("compact_date_and_time", "date.sep_text", " : ") << r;
            ss << config.getNestedColor("compact_date_and_time", "date", "month_name", "")
               << time.getMonthName() << r;
            wrote = true;
        }

        if (config.isNestedEnabled("compact_date_and_time", "date", "show_month_num")) {
            if (wrote) ss << config.getNestedColor("compact_date_and_time", "date", "num_sep_color", "")
                          << config.getNestedString("compact_date_and_time", "date.num_sep_text", " ") << r;
            ss << config.getNestedColor("compact_date_and_time", "date", "month_num", "")
               << setw(2) << setfill('0') << time.getMonthNumber() << r;
            wrote = true;
        }

        if (config.isNestedEnabled("compact_date_and_time", "date", "show_year")) {
            if (wrote) ss << config.getNestedColor("compact_date_and_time", "date", "sep", "")
                          << config.getNestedString("compact_date_and_time", "date.sep_text", " : ") << r;
            ss << config.getNestedColor("compact_date_and_time", "date", "year", "")
               << time.getYearNumber() << r;
        }

        ss << config.getNestedColor("compact_date_and_time", "date", "bracket", "")
           << config.getNestedString("compact_date_and_time", "date.close", ")") << r;
    };

    // ---------- WEEK SECTION ----------
    fields["week"] = [&]() {
        if (!config.isNestedEnabled("compact_date_and_time", "week", "enabled")) return;

        ss << config.getNestedColor("compact_date_and_time", "week", "bracket", "")
           << config.getNestedString("compact_date_and_time", "week.open", "(") << r;

        if (config.isNestedEnabled("compact_date_and_time", "week", "show_label")) {
            ss << config.getNestedColor("compact_date_and_time", "week", "label", "")
               << config.getNestedString("compact_date_and_time", "week.label_text", "Week: ") << r;
        }

        bool wrote = false;

        if (config.isNestedEnabled("compact_date_and_time", "week", "show_num")) {
            ss << config.getNestedColor("compact_date_and_time", "week", "num", "")
               << time.getWeekNumber() << r;
            wrote = true;
        }

        if (config.isNestedEnabled("compact_date_and_time", "week", "show_day_name")) {
            if (wrote) ss << config.getNestedColor("compact_date_and_time", "week", "sep", "")
                          << config.getNestedString("compact_date_and_time", "week.sep_text", " - ") << r;
            ss << config.getNestedColor("compact_date_and_time", "week", "day_name", "")
               << time.getDayName() << r;
        }

        ss << config.getNestedColor("compact_date_and_time", "week", "bracket", "")
           << config.getNestedString("compact_date_and_time", "week.close", ")") << r;
    };

    // ---------- LEAP YEAR SECTION ----------
    fields["leap_year"] = [&]() {
        if (!config.isNestedEnabled("compact_date_and_time", "leap_year", "enabled")) return;

        ss << config.getNestedColor("compact_date_and_time", "leap_year", "bracket", "")
           << config.getNestedString("compact_date_and_time", "leap_year.open", "(") << r;

        if (config.isNestedEnabled("compact_date_and_time", "leap_year", "show_label")) {
            ss << config.getNestedColor("compact_date_and_time", "leap_year", "label", "")
               << config.getNestedString("compact_date_and_time", "leap_year.label_text", "Leap Year: ") << r;
        }

        if (config.isNestedEnabled("compact_date_and_time", "leap_year", "show_val")) {
            ss << config.getNestedColor("compact_date_and_time", "leap_year", "val", "")
               << time.getLeapYear() << r;
        }

        ss << config.getNestedColor("compact_date_and_time", "leap_year", "bracket", "")
           << config.getNestedString("compact_date_and_time", "leap_year.close", ")") << r;
    };

    // ---- Run subsections in the order JSON specifies, with spacing controlled by trailing spaces in each entry ----
    static const std::vector<std::string> defaultOrder =
        {"time ", "date ", "week ", "leap_year"};
    auto order = config.getStringArray("compact_date_and_time", "order", defaultOrder);

    runOrderedFields(order, fields, ss);

    lp.push(ss.str());
};


// ==================== COMPACT OPERATING SYSTEM ====================
sections["compact_operating_system"] = [&]() {
    if (!config.isEnabled("compact_operating_system")) return;
    ostringstream ss;

    // line spacing
    int spacing = config.getNestedInt("compact_operating_system","top_line_spacing",0);
    for (int n = 0; n < spacing; n++) {lp.push("");}

    // Prefix - comes entirely from JSON (can be emoji, text, or empty)
    if (config.isFieldEnabled("compact_operating_system", "prefixes.show")) {
        ss << config.getColor("compact_operating_system", "prefixes.prefix_color", "")
           << config.getPrefix("compact_operating_system", "prefixes.prefix", "") << r;
    }

    // Label
    ss << config.getColor("compact_operating_system", "label.color", "")
       << config.getLabel("compact_operating_system", "label.text", "OS") << r;

    // Separator
    ss << config.getColor("compact_operating_system", "separator.color", "")
       << config.getPrefix("compact_operating_system", "separator.text", ":") << " " << r;

    // ---- Register each orderable field as a named lambda ----
    std::map<std::string, std::function<void()>> fields;

    fields["name"] = [&]() {
        if (!config.isFieldEnabled("compact_operating_system", "fields.name.show")) return;
        ss << config.getColor("compact_operating_system", "fields.name.value_color", "")
           << c_os.getOSName() << r;
    };

    fields["build"] = [&]() {
        if (!config.isFieldEnabled("compact_operating_system", "fields.build.show")) return;
        ss << config.getColor("compact_operating_system", "fields.build.value_color", "")
           << c_os.getOSBuild() << r;
    };

    fields["arch"] = [&]() {
        if (!config.isFieldEnabled("compact_operating_system", "fields.arch.show")) return;
        ss << config.getColor("compact_operating_system", "brackets.color", "")
           << config.getPrefix("compact_operating_system", "brackets.open", "(") << r
           << config.getColor("compact_operating_system", "fields.arch.value_color", "")
           << c_os.getArchitecture() << r
           << config.getColor("compact_operating_system", "brackets.color", "")
           << config.getPrefix("compact_operating_system", "brackets.close", ")") << r;
    };

    fields["uptime"] = [&]() {
        if (!config.isFieldEnabled("compact_operating_system", "fields.uptime.show")) return;
        ss << config.getColor("compact_operating_system", "brackets.color", "")
           << config.getPrefix("compact_operating_system", "brackets.open", "(") << r
           << config.getColor("compact_operating_system", "fields.uptime.label_color", "")
           << config.getLabel("compact_operating_system", "fields.uptime.label", "uptime: ") << r
           << config.getColor("compact_operating_system", "fields.uptime.value_color", "")
           << c_os.getUptime() << r
           << config.getColor("compact_operating_system", "brackets.color", "")
           << config.getPrefix("compact_operating_system", "brackets.close", ")") << r;
    };

    // ---- Run fields in the order JSON specifies, with spacing controlled by trailing spaces in each entry ----
    static const std::vector<std::string> defaultOrder =
        {"name ", "build", "arch ", "uptime"};
    auto order = config.getStringArray("compact_operating_system", "order", defaultOrder);

    runOrderedFields(order, fields, ss);

    lp.push(ss.str());
};

// -------------compact processor------------
sections["compact_processor"] = [&]() {
    if (!config.isEnabled("compact_processor")) return;
    ostringstream ss;

    int spacing = config.getNestedInt("compact_processor","top_line_spacing",0);
    for (int n = 0; n < spacing; n++) {lp.push("");}

    if (config.isFieldEnabled("compact_processor", "prefixes.show")) {
        ss << config.getColor("compact_processor", "prefixes.prefix_color", "")
           << config.getPrefix("compact_processor", "prefixes.prefix", "") << r;
    }

    ss << config.getColor("compact_processor", "label.color", "")
       << config.getLabel("compact_processor", "label.text", "CPU") << r;

    ss << config.getColor("compact_processor", "separator.color", "")
       << config.getPrefix("compact_processor", "separator.text", ":") << " " << r;

    std::map<std::string, std::function<void()>> fields;

    fields["name"] = [&]() {
        if (!config.isFieldEnabled("compact_processor", "fields.name.show")) return;
        ss << config.getColor("compact_processor", "fields.name.value_color", "")
           << c_cpu.getCPUName() << r;
        // no trailing space here anymore — "order" controls it now
    };

    fields["cores_threads"] = [&]() {
        bool showCores = config.isFieldEnabled("compact_processor", "fields.cores.show");
        bool showThreads = config.isFieldEnabled("compact_processor", "fields.threads.show");
        if (!showCores && !showThreads) return;

        ss << config.getColor("compact_processor", "brackets.color", "")
           << config.getPrefix("compact_processor", "brackets.open", "(") << r;

        if (showCores) {
            ss << config.getColor("compact_processor", "fields.cores.value_color", "")
               << c_cpu.getCPUCores() << r
               << config.getColor("compact_processor", "fields.cores.value_suffix_color", "")
               << config.getLabel("compact_processor", "fields.cores.value_suffix", "C") << r;
        }

        if (showCores && showThreads) {
            ss << config.getColor("compact_processor", "separator.divider_color", "")
               << config.getPrefix("compact_processor", "separator.divider", "/") << r;
        }

        if (showThreads) {
            ss << config.getColor("compact_processor", "fields.threads.value_color", "")
               << c_cpu.getCPUThreads() << r
               << config.getColor("compact_processor", "fields.threads.value_suffix_color", "")
               << config.getLabel("compact_processor", "fields.threads.value_suffix", "T") << r;
        }

        ss << config.getColor("compact_processor", "brackets.color", "")
           << config.getPrefix("compact_processor", "brackets.close", ")") << r;
        // no trailing space here anymore
    };

    fields["clock"] = [&]() {
        if (!config.isFieldEnabled("compact_processor", "fields.clock.show")) return;
        ss << fixed << setprecision(2)
           << config.getColor("compact_processor", "fields.clock.at_symbol_color", "")
           << config.getLabel("compact_processor", "fields.clock.at_symbol", "@") << r
           << config.getColor("compact_processor", "fields.clock.value_color", "") << " "
           << c_cpu.getClockSpeed()
           << config.getColor("compact_processor", "fields.clock.unit_color", "")
           << config.getLabel("compact_processor", "fields.clock.unit", " GHz") << r;
    };

    static const std::vector<std::string> defaultOrder =
        {"cores_threads ", "name ", "clock"};
    auto order = config.getStringArray("compact_processor", "order", defaultOrder);

    runOrderedFields(order, fields, ss);

    lp.push(ss.str());
};

// ==================== COMPACT GPU ====================
sections["compact_graphics_card"] = [&]() {
    if (!config.isEnabled("compact_graphics_card")) return;
    ostringstream ss;

    // line spacing json driven
    int spacing = config.getNestedInt("compact_graphics_card","top_line_spacing",0);
    for (int n = 0; n < spacing; n++) {lp.push("");}

    // Prefix - comes entirely from JSON (can be emoji, text, or empty)
    if (config.isFieldEnabled("compact_graphics_card", "prefixes.show")) {
        ss << config.getColor("compact_graphics_card", "prefixes.prefix_color", "")
           << config.getPrefix("compact_graphics_card", "prefixes.prefix", "") << r;
    }

    // Label
    ss << config.getColor("compact_graphics_card", "label.color", "")
       << config.getLabel("compact_graphics_card", "label.text", "GPU") << r;

    // Separator
    ss << config.getColor("compact_graphics_card", "separator.color", "")
       << config.getPrefix("compact_graphics_card", "separator.text", ":") << " " << r;

    // ---- Register each orderable field as a named lambda ----
    std::map<std::string, std::function<void()>> fields;

    fields["name"] = [&]() {
        if (!config.isFieldEnabled("compact_graphics_card", "fields.name.show")) return;
        ss << config.getColor("compact_graphics_card", "fields.name.value_color", "")
           << c_gpu.getGPUName() << r;
    };

    fields["usage"] = [&]() {
        if (!config.isFieldEnabled("compact_graphics_card", "fields.usage.show")) return;
        ss << config.getColor("compact_graphics_card", "brackets.color", "")
           << config.getPrefix("compact_graphics_card", "brackets.open", "(") << r
           << config.getColor("compact_graphics_card", "fields.usage.value_color", "")
           << c_gpu.getGPUUsagePercent()
           << config.getColor("compact_graphics_card", "fields.usage.unit_color", "")
           << config.getLabel("compact_graphics_card", "fields.usage.unit", "%") << r
           << config.getColor("compact_graphics_card", "brackets.color", "")
           << config.getPrefix("compact_graphics_card", "brackets.close", ")") << r;
    };

    fields["vram"] = [&]() {
        if (!config.isFieldEnabled("compact_graphics_card", "fields.vram.show")) return;
        ss << config.getColor("compact_graphics_card", "brackets.color", "")
           << config.getPrefix("compact_graphics_card", "brackets.open", "(") << r
           << config.getColor("compact_graphics_card", "fields.vram.value_color", "")
           << c_gpu.getVRAMGB()
           << config.getColor("compact_graphics_card", "fields.vram.unit_color", "")
           << config.getLabel("compact_graphics_card", "fields.vram.unit", " GB") << r
           << config.getColor("compact_graphics_card", "brackets.color", "")
           << config.getPrefix("compact_graphics_card", "brackets.close", ")") << r;
    };

    fields["freq"] = [&]() {
        if (!config.isFieldEnabled("compact_graphics_card", "fields.freq.show")) return;
        ss << config.getColor("compact_graphics_card", "brackets.color", "")
           << config.getPrefix("compact_graphics_card", "brackets.open", "(") << r
           << config.getColor("compact_graphics_card", "fields.freq.at_symbol_color", "")
           << config.getLabel("compact_graphics_card", "fields.freq.at_symbol", "@") << r
           << config.getColor("compact_graphics_card", "fields.freq.value_color", "")
           << c_gpu.getGPUFrequency() << r
           << config.getColor("compact_graphics_card", "brackets.color", "")
           << config.getPrefix("compact_graphics_card", "brackets.close", ")") << r;
    };

    // ---- Run fields in the order JSON specifies, with spacing controlled by trailing spaces in each entry ----
    static const std::vector<std::string> defaultOrder =
        {"name ", "usage ", "vram ", "freq"};
    auto order = config.getStringArray("compact_graphics_card", "order", defaultOrder);

    runOrderedFields(order, fields, ss);

    lp.push(ss.str());
};

// ==================== COMPACT DISPLAY ====================
sections["compact_display_monitor"] = [&]() {
    if (!config.isEnabled("compact_display_monitor")) return;
    CompactScreen screenDetector;
    auto screens = screenDetector.getScreens();

    // line spacing json driven
    int spacing = config.getNestedInt("compact_display_monitor","top_line_spacing",0);
    for (int n = 0; n < spacing; n++) {lp.push("");}

    if (screens.empty()) {
        ostringstream ss;
        ss << config.getColor("compact_display_monitor", "header.text_color", "")
           << config.getLabel("compact_display_monitor", "header.text", "Display") << r
           << config.getColor("compact_display_monitor", "header.separator_color", "")
           << config.getPrefix("compact_display_monitor", "header.separator", ":") << " " << r
           << config.getColor("compact_display_monitor", "fields.name.value_color", "")
           << config.getLabel("compact_display_monitor", "no_displays_text", "No displays detected") << r;
        lp.push(ss.str());
        return;
    }

    for (size_t i = 0; i < screens.size(); ++i) {
        const auto& screen = screens[i];
        ostringstream ss;

        // Prefix - comes entirely from JSON
        if (config.isFieldEnabled("compact_display_monitor", "prefixes.show")) {
            ss << config.getColor("compact_display_monitor", "prefixes.prefix_color", "")
               << config.getPrefix("compact_display_monitor", "prefixes.prefix", "") << r;
        }

        // Header: Display N:
        ss << config.getColor("compact_display_monitor", "header.text_color", "")
           << config.getLabel("compact_display_monitor", "header.text", "Display") << " " << (i + 1) << r
           << config.getColor("compact_display_monitor", "header.separator_color", "")
           << config.getPrefix("compact_display_monitor", "header.separator", ":") << " " << r;

        // ---- Register each orderable field as a named lambda ----
        std::map<std::string, std::function<void()>> fields;

        fields["name"] = [&]() {
            if (!config.isFieldEnabled("compact_display_monitor", "fields.name.show")) return;
            ss << config.getColor("compact_display_monitor", "fields.name.value_color", "")
               << screen.name << r;
        };

        fields["resolution"] = [&]() {
            if (!config.isFieldEnabled("compact_display_monitor", "fields.resolution.show")) return;
            ss << config.getColor("compact_display_monitor", "brackets.color", "")
               << config.getPrefix("compact_display_monitor", "brackets.open", "(") << r
               << config.getColor("compact_display_monitor", "fields.resolution.value_color", "")
               << screen.native_width << r
               << config.getColor("compact_display_monitor", "fields.resolution.x_color", "")
               << config.getLabel("compact_display_monitor", "fields.resolution.x_symbol", " x ") << r
               << config.getColor("compact_display_monitor", "fields.resolution.value_color", "")
               << screen.native_height << r
               << config.getColor("compact_display_monitor", "brackets.color", "")
               << config.getPrefix("compact_display_monitor", "brackets.close", ")") << r;
        };

        fields["scale"] = [&]() {
            if (!config.isFieldEnabled("compact_display_monitor", "fields.scale.show")) return;
            ss << config.getColor("compact_display_monitor", "brackets.color", "")
               << config.getPrefix("compact_display_monitor", "brackets.open", "(") << r
               << config.getColor("compact_display_monitor", "fields.scale.label_color", "")
               << config.getLabel("compact_display_monitor", "fields.scale.label", "Scale: ") << r
               << config.getColor("compact_display_monitor", "fields.scale.value_color", "")
               << screen.scale_percent
               << config.getColor("compact_display_monitor", "fields.scale.unit_color", "")
               << config.getLabel("compact_display_monitor", "fields.scale.unit", "%") << r
               << config.getColor("compact_display_monitor", "brackets.color", "")
               << config.getPrefix("compact_display_monitor", "brackets.close", ")") << r;
        };

        fields["upscale"] = [&]() {
            if (!config.isFieldEnabled("compact_display_monitor", "fields.upscale.show")) return;
            ss << config.getColor("compact_display_monitor", "brackets.color", "")
               << config.getPrefix("compact_display_monitor", "brackets.open", "(") << r
               << config.getColor("compact_display_monitor", "fields.upscale.label_color", "")
               << config.getLabel("compact_display_monitor", "fields.upscale.label", "upscale: ") << r
               << config.getColor("compact_display_monitor", "fields.upscale.value_color", "")
               << screen.upscale << r
               << config.getColor("compact_display_monitor", "brackets.color", "")
               << config.getPrefix("compact_display_monitor", "brackets.close", ")") << r;
        };

        fields["refresh"] = [&]() {
            if (!config.isFieldEnabled("compact_display_monitor", "fields.refresh.show")) return;
            ss << config.getColor("compact_display_monitor", "brackets.color", "")
               << config.getPrefix("compact_display_monitor", "brackets.open", "(") << r
               << config.getColor("compact_display_monitor", "fields.refresh.at_symbol_color", "")
               << config.getLabel("compact_display_monitor", "fields.refresh.at_symbol", "@") << r
               << config.getColor("compact_display_monitor", "fields.refresh.value_color", "")
               << screen.refresh_rate
               << config.getColor("compact_display_monitor", "fields.refresh.unit_color", "")
               << config.getLabel("compact_display_monitor", "fields.refresh.unit", "Hz") << r
               << config.getColor("compact_display_monitor", "brackets.color", "")
               << config.getPrefix("compact_display_monitor", "brackets.close", ")") << r;
        };

        // ---- Run fields in the order JSON specifies, with spacing controlled by trailing spaces in each entry ----
        static const std::vector<std::string> defaultOrder =
            {"name ", "resolution ", "scale ", "upscale ", "refresh"};
        auto order = config.getStringArray("compact_display_monitor", "order", defaultOrder);

        runOrderedFields(order, fields, ss);

        lp.push(ss.str());
    }
};

// ==================== COMPACT MEMORY ====================
sections["compact_system_memory"] = [&]() {
    if (!config.isEnabled("compact_system_memory")) return;
    ostringstream ss;

    // line spacing json driven
    int spacing = config.getNestedInt("compact_system_memory","top_line_spacing",0);
    for (int n = 0; n < spacing; n++) {lp.push("");}

    // Prefix - comes entirely from JSON
    if (config.isFieldEnabled("compact_system_memory", "prefixes.show")) {
        ss << config.getColor("compact_system_memory", "prefixes.prefix_color", "")
           << config.getPrefix("compact_system_memory", "prefixes.prefix", "") << r;
    }

    // Label
    ss << config.getColor("compact_system_memory", "label.color", "")
       << config.getLabel("compact_system_memory", "label.text", "Memory") << r;

    // Separator
    ss << config.getColor("compact_system_memory", "separator.color", "")
       << config.getPrefix("compact_system_memory", "separator.text", ":") << " " << r;

    // ---- Register each orderable field as a named lambda ----
    std::map<std::string, std::function<void()>> fields;

    fields["total"] = [&]() {
        if (!config.isFieldEnabled("compact_system_memory", "fields.total.show")) return;
        ss << config.getColor("compact_system_memory", "brackets.color", "")
           << config.getPrefix("compact_system_memory", "brackets.open", "(") << r
           << config.getColor("compact_system_memory", "fields.total.label_color", "")
           << config.getLabel("compact_system_memory", "fields.total.label", "total: ") << r
           << config.getColor("compact_system_memory", "fields.total.value_color", "")
           << c_memory.get_total_memory()
           << config.getColor("compact_system_memory", "fields.total.unit_color", "")
           << config.getLabel("compact_system_memory", "fields.total.unit", " GB") << r
           << config.getColor("compact_system_memory", "brackets.color", "")
           << config.getPrefix("compact_system_memory", "brackets.close", ")") << r;
    };

    fields["free"] = [&]() {
        if (!config.isFieldEnabled("compact_system_memory", "fields.free.show")) return;
        ss << config.getColor("compact_system_memory", "brackets.color", "")
           << config.getPrefix("compact_system_memory", "brackets.open", "(") << r
           << config.getColor("compact_system_memory", "fields.free.label_color", "")
           << config.getLabel("compact_system_memory", "fields.free.label", "free: ") << r
           << config.getColor("compact_system_memory", "fields.free.value_color", "")
           << c_memory.get_free_memory()
           << config.getColor("compact_system_memory", "fields.free.unit_color", "")
           << config.getLabel("compact_system_memory", "fields.free.unit", " GB") << r
           << config.getColor("compact_system_memory", "brackets.color", "")
           << config.getPrefix("compact_system_memory", "brackets.close", ")") << r;
    };

    fields["percent"] = [&]() {
        if (!config.isFieldEnabled("compact_system_memory", "fields.percent.show")) return;
        ss << config.getColor("compact_system_memory", "brackets.color", "")
           << config.getPrefix("compact_system_memory", "brackets.open", "(") << r
           << config.getColor("compact_system_memory", "fields.percent.value_color", "")
           << c_memory.get_used_memory_percent()
           << config.getColor("compact_system_memory", "fields.percent.unit_color", "")
           << config.getLabel("compact_system_memory", "fields.percent.unit", "%") << r
           << config.getColor("compact_system_memory", "brackets.color", "")
           << config.getPrefix("compact_system_memory", "brackets.close", ")") << r;
    };

    // ---- Run fields in the order JSON specifies, with spacing controlled by trailing spaces in each entry ----
    static const std::vector<std::string> defaultOrder =
        {"total ", "free ", "percent"};
    auto order = config.getStringArray("compact_system_memory", "order", defaultOrder);

    runOrderedFields(order, fields, ss);

    lp.push(ss.str());
};

// ==================== COMPACT AUDIO ====================
sections["compact_audio_devices"] = [&]() {
    if (!config.isEnabled("compact_audio_devices")) return;

    // ---- Register each orderable device-line as a named lambda ----
    std::map<std::string, std::function<void()>> fields;

    fields["input"] = [&]() {
        if (!config.isFieldEnabled("compact_audio_devices", "input.show")) return;
        ostringstream ss;

        // line spacing json driven
        int spacing = config.getNestedInt("compact_audio_devices","input.top_line_spacing",0);
        for (int n = 0; n < spacing; n++) {lp.push("");}

        // Input prefix - from JSON
        if (config.isFieldEnabled("compact_audio_devices", "input.prefixes.show")) {
            ss << config.getColor("compact_audio_devices", "input.prefixes.prefix_color", "")
               << config.getPrefix("compact_audio_devices", "input.prefixes.prefix", "") << r;
        }

        // Input label
        ss << config.getColor("compact_audio_devices", "input.label.color", "")
           << config.getLabel("compact_audio_devices", "input.label.text", "Audio Input") << r;

        // Input separator
        ss << config.getColor("compact_audio_devices", "input.separator.color", "")
           << config.getPrefix("compact_audio_devices", "input.separator.text", ":") << " " << r;

        // Input device name
        ss << config.getColor("compact_audio_devices", "input.device_color", "")
           << c_audio.active_audio_input() << r << " ";

        // Input status
        ss << config.getColor("compact_audio_devices", "input.status_brackets_color", "")
           << config.getNestedString("compact_audio_devices", "input.status_bracket_open", "[") << r
           << config.getColor("compact_audio_devices", "input.status_color", "")
           << c_audio.active_audio_input_status() << r
           << config.getColor("compact_audio_devices", "input.status_brackets_color", "")
           << config.getNestedString("compact_audio_devices", "input.status_bracket_close", "]") << r;

        lp.push(ss.str());
    };

    fields["output"] = [&]() {
        if (!config.isFieldEnabled("compact_audio_devices", "output.show")) return;
        ostringstream ss;

        // line spacing json driven
        int spacing = config.getNestedInt("compact_audio_devices","output.top_line_spacing",0);
        for (int n = 0; n < spacing; n++) {lp.push("");}

        // Output prefix - from JSON
        if (config.isFieldEnabled("compact_audio_devices", "output.prefixes.show")) {
            ss << config.getColor("compact_audio_devices", "output.prefixes.prefix_color", "")
               << config.getPrefix("compact_audio_devices", "output.prefixes.prefix", "") << r;
        }

        // Output label
        ss << config.getColor("compact_audio_devices", "output.label.color", "")
           << config.getLabel("compact_audio_devices", "output.label.text", "Audio Output") << r;

        // Output separator
        ss << config.getColor("compact_audio_devices", "output.separator.color", "")
           << config.getPrefix("compact_audio_devices", "output.separator.text", ":") << " " << r;

        // Output device name
        ss << config.getColor("compact_audio_devices", "output.device_color", "")
           << c_audio.active_audio_output() << r << " ";

        // Output status
        ss << config.getColor("compact_audio_devices", "output.status_brackets_color", "")
           << config.getNestedString("compact_audio_devices", "output.status_bracket_open", "[") << r
           << config.getColor("compact_audio_devices", "output.status_color", "")
           << c_audio.active_audio_output_status() << r
           << config.getColor("compact_audio_devices", "output.status_brackets_color", "")
           << config.getNestedString("compact_audio_devices", "output.status_bracket_close", "]") << r;

        lp.push(ss.str());
    };

    // ---- Run device-lines in the order JSON specifies ----
    static const std::vector<std::string> defaultOrder =
        {"input", "output"};
    auto order = config.getStringArray("compact_audio_devices", "order", defaultOrder);

    for (const auto& key : order) {
        auto it = fields.find(key);
        if (it != fields.end()) it->second();
    }
};

// ==================== COMPACT PERFORMANCE ====================
sections["compact_resource_usage"] = [&]() {
    if (!config.isEnabled("compact_resource_usage")) return;
    ostringstream ss;

    // line spacing json driven
    int spacing = config.getNestedInt("compact_resource_usage","top_line_spacing",0);
    for (int n = 0; n < spacing; n++) {lp.push("");}

    // Prefix - from JSON
    if (config.isFieldEnabled("compact_resource_usage", "prefixes.show")) {
        ss << config.getColor("compact_resource_usage", "prefixes.prefix_color", "")
           << config.getPrefix("compact_resource_usage", "prefixes.prefix", "") << r;
    }

    // Label
    ss << config.getColor("compact_resource_usage", "label.color", "")
       << config.getLabel("compact_resource_usage", "label.text", "Performance") << r;

    // Separator
    ss << config.getColor("compact_resource_usage", "separator.color", "")
       << config.getPrefix("compact_resource_usage", "separator.text", ":") << r
       << config.getColor("compact_resource_usage", "separator.suffix_color", "")
       << config.getPrefix("compact_resource_usage", "separator.suffix", " ") << r;

    // Generic helper: prints one bracketed "(Label: value%)" stat block, fully JSON-driven per field
    auto addPerf = [&](const string& field, auto val) {
        if (!config.isFieldEnabled("compact_resource_usage", "fields." + field + ".show")) return;

        ss << config.getColor("compact_resource_usage", "fields." + field + ".bracket_color", "")
           << config.getPrefix("compact_resource_usage", "fields." + field + ".bracket_open", "(") << r

           << config.getColor("compact_resource_usage", "fields." + field + ".label_color", "")
           << config.getLabel("compact_resource_usage", "fields." + field + ".label", field) << r

           << config.getColor("compact_resource_usage", "fields." + field + ".label_suffix_color", "")
           << config.getPrefix("compact_resource_usage", "fields." + field + ".label_suffix", ": ") << r

           << config.getColor("compact_resource_usage", "fields." + field + ".value_color", "")
           << val << r

           << config.getColor("compact_resource_usage", "fields." + field + ".unit_color", "")
           << config.getLabel("compact_resource_usage", "fields." + field + ".unit", "%") << r

           << config.getColor("compact_resource_usage", "fields." + field + ".bracket_color", "")
           << config.getPrefix("compact_resource_usage", "fields." + field + ".bracket_close", ")") << r;
    };

    // ---- Register each orderable stat as a named lambda ----
    std::map<std::string, std::function<void()>> fields;

    fields["cpu"]  = [&]() { addPerf("cpu",  c_perf.getCPUUsage()); };
    fields["gpu"]  = [&]() { addPerf("gpu",  c_perf.getGPUUsage()); };
    fields["ram"]  = [&]() { addPerf("ram",  c_perf.getRAMUsage()); };
    fields["disk"] = [&]() { addPerf("disk", c_perf.getDiskUsage()); };

    // ---- Run fields in the order JSON specifies, with spacing controlled by trailing spaces in each entry ----
    static const std::vector<std::string> defaultOrder =
        {"cpu ", "gpu ", "ram ", "disk"};
    auto order = config.getStringArray("compact_resource_usage", "order", defaultOrder);

    runOrderedFields(order, fields, ss);

    lp.push(ss.str());
};

// ==================== COMPACT USER ====================
sections["compact_user_account"] = [&]() {
    if (!config.isEnabled("compact_user_account")) return;
    ostringstream ss;

    // line spacing json driven
    int spacing = config.getNestedInt("compact_user_account","top_line_spacing",0);
    for (int n = 0; n < spacing; n++) {lp.push("");}

    // Prefix - from JSON
    if (config.isFieldEnabled("compact_user_account", "prefixes.show")) {
        ss << config.getColor("compact_user_account", "prefixes.prefix_color", "")
           << config.getPrefix("compact_user_account", "prefixes.prefix", "") << r;
    }

    // Label
    ss << config.getColor("compact_user_account", "label.color", "")
       << config.getLabel("compact_user_account", "label.text", "User") << r;

    // Separator
    ss << config.getColor("compact_user_account", "separator.color", "")
       << config.getPrefix("compact_user_account", "separator.text", ":") << " " << r;

    // ---- Register each orderable field as a named lambda ----
    std::map<std::string, std::function<void()>> fields;

    fields["username"] = [&]() {
        if (!config.isFieldEnabled("compact_user_account", "fields.username.show")) return;
        ss << config.getColor("compact_user_account", "fields.username.prefix_color", "")
           << config.getPrefix("compact_user_account", "fields.username.prefix", "@") << r
           << config.getColor("compact_user_account", "fields.username.value_color", "")
           << c_user.getUsername() << r;
    };

    fields["domain"] = [&]() {
        if (!config.isFieldEnabled("compact_user_account", "fields.domain.show")) return;
        ss << config.getColor("compact_user_account", "brackets.color", "")
           << config.getPrefix("compact_user_account", "brackets.open", "(") << r
           << config.getColor("compact_user_account", "fields.domain.label_color", "")
           << config.getLabel("compact_user_account", "fields.domain.label", "Domain: ") << r
           << config.getColor("compact_user_account", "fields.domain.value_color", "")
           << c_user.getDomain() << r
           << config.getColor("compact_user_account", "brackets.color", "")
           << config.getPrefix("compact_user_account", "brackets.close", ")") << r;
    };

    fields["type"] = [&]() {
        if (!config.isFieldEnabled("compact_user_account", "fields.type.show")) return;
        ss << config.getColor("compact_user_account", "brackets.color", "")
           << config.getPrefix("compact_user_account", "brackets.open", "(") << r
           << config.getColor("compact_user_account", "fields.type.label_color", "")
           << config.getLabel("compact_user_account", "fields.type.label", "Type: ") << r
           << config.getColor("compact_user_account", "fields.type.value_color", "")
           << c_user.isAdmin() << r
           << config.getColor("compact_user_account", "brackets.color", "")
           << config.getPrefix("compact_user_account", "brackets.close", ")") << r;
    };

    // ---- Run fields in the order JSON specifies, with spacing controlled by trailing spaces in each entry ----
    static const std::vector<std::string> defaultOrder =
        {"username ", "domain ", "type"};
    auto order = config.getStringArray("compact_user_account", "order", defaultOrder);

    runOrderedFields(order, fields, ss);

    lp.push(ss.str());
};

// ==================== COMPACT NETWORK ====================
sections["compact_network_connection"] = [&]() {
    if (!config.isEnabled("compact_network_connection")) return;

    // line spacing json driven
    int spacing = config.getNestedInt("compact_network_connection","top_line_spacing",0);
    for (int n = 0; n < spacing; n++) {lp.push("");}

    ostringstream ss;

    // Prefix - from JSON
    if (config.isFieldEnabled("compact_network_connection", "prefixes.show")) {
        ss << config.getColor("compact_network_connection", "prefixes.prefix_color", "")
           << config.getPrefix("compact_network_connection", "prefixes.prefix", "") << r;
    }

    // Label
    ss << config.getColor("compact_network_connection", "label.color", "")
       << config.getLabel("compact_network_connection", "label.text", "Network") << r;

    // Separator
    ss << config.getColor("compact_network_connection", "separator.color", "")
       << config.getPrefix("compact_network_connection", "separator.text", ":") << " " << r;

    // Generic helper: prints one bracketed "(Label: value)" block, fully JSON-driven per field
    auto addNetField = [&](const string& field, const string& value) {
        if (!config.isFieldEnabled("compact_network_connection", "fields." + field + ".show")) return;

        ss << config.getNestedColor("compact_network_connection", "fields." + field + ".bracket_color", "")
           << config.getNestedString("compact_network_connection", "fields." + field + ".bracket_open", "(")
           << r
           << config.getNestedColor("compact_network_connection", "fields." + field + ".label_color", "")
           << config.getNestedString("compact_network_connection", "fields." + field + ".label", "")
           << r
           << config.getNestedColor("compact_network_connection", "fields." + field + ".label_suffix_color", "")
           << config.getNestedString("compact_network_connection", "fields." + field + ".label_suffix", "")
           << r
           << config.getColor("compact_network_connection", "fields." + field + ".value_color", "")
           << value
           << r
           << config.getNestedColor("compact_network_connection", "fields." + field + ".bracket_color", "")
           << config.getNestedString("compact_network_connection", "fields." + field + ".bracket_close", ")")
           << r;
    };

    // ---- Register each orderable field as a named lambda ----
    std::map<std::string, std::function<void()>> fields;

    fields["name"] = [&]() { addNetField("name", c_net.get_network_name()); };
    fields["type"] = [&]() { addNetField("type", c_net.get_network_type()); };
    fields["ip"]   = [&]() { addNetField("ip",   c_net.get_network_ip()); };

    // ---- Run fields in the order JSON specifies, with spacing controlled by trailing spaces in each entry ----
    static const std::vector<std::string> defaultOrder =
        {"name ", "type ", "ip"};
    auto order = config.getStringArray("compact_network_connection", "order", defaultOrder);

    runOrderedFields(order, fields, ss);

    lp.push(ss.str());
};

// ==================== COMPACT DISK ====================
sections["compact_disk_storage"] = [&]() {
    if (!config.isEnabled("compact_disk_storage")) return;

    // ---------- DISK USAGE ----------
    if (config.isFieldEnabled("compact_disk_storage", "usage.show")) {
        auto disks = disk.getAllDiskUsage();
        ostringstream ss;

        // line spacing json driven
        int spacing = config.getNestedInt("compact_disk_storage","top_line_spacing",0);
        for (int n = 0; n < spacing; n++) {lp.push("");}

        // Usage prefix - from JSON
        if (config.isFieldEnabled("compact_disk_storage", "usage.prefixes.show")) {
            ss << config.getColor("compact_disk_storage", "usage.prefixes.prefix_color", "")
               << config.getPrefix("compact_disk_storage", "usage.prefixes.prefix", "") << r;
        }

        // Usage label
        ss << config.getColor("compact_disk_storage", "usage.label.color", "")
           << config.getLabel("compact_disk_storage", "usage.label.text", "Disk Usage") << r;

        // Usage separator
        ss << config.getColor("compact_disk_storage", "usage.separator.color", "")
           << config.getPrefix("compact_disk_storage", "usage.separator.text", ":") << r
           << config.getColor("compact_disk_storage", "usage.separator.suffix_color", "")
           << config.getPrefix("compact_disk_storage", "usage.separator.suffix", " ") << r;

        // Per-disk entry, fully JSON-driven
        for (const auto& d : disks) {
            ss << config.getColor("compact_disk_storage", "usage.entry.bracket_color", "")
               << config.getPrefix("compact_disk_storage", "usage.entry.bracket_open", "(") << r

               << config.getColor("compact_disk_storage", "usage.entry.letter_color", "")
               << d.first[0] << r

               << config.getColor("compact_disk_storage", "usage.entry.letter_suffix_color", "")
               << config.getPrefix("compact_disk_storage", "usage.entry.letter_suffix", ":") << r
               << config.getColor("compact_disk_storage", "usage.entry.letter_suffix_color", "")
               << config.getPrefix("compact_disk_storage", "usage.entry.letter_suffix_space", " ") << r

               << config.getColor("compact_disk_storage", "usage.entry.value_color", "")
               << fixed << setprecision(1) << d.second << r

               << config.getColor("compact_disk_storage", "usage.entry.unit_color", "")
               << config.getLabel("compact_disk_storage", "usage.entry.unit", "%") << r

               << config.getColor("compact_disk_storage", "usage.entry.bracket_color", "")
               << config.getPrefix("compact_disk_storage", "usage.entry.bracket_close", ")") << r

               << config.getColor("compact_disk_storage", "usage.entry.suffix_color", "")
               << config.getPrefix("compact_disk_storage", "usage.entry.suffix", " ") << r;
        }
        lp.push(ss.str());
    }

    // ---------- DISK CAPACITY ----------
    if (config.isFieldEnabled("compact_disk_storage", "capacity.show")) {
        auto caps = disk.getDiskCapacity();
        ostringstream sc;

        // line spacing json driven
        int spacing = config.getNestedInt("compact_disk_storage","capacity.top_line_spacing",0);
        for (int n = 0; n < spacing; n++) {lp.push("");}

        // Capacity prefix - from JSON
        if (config.isFieldEnabled("compact_disk_storage", "capacity.prefixes.show")) {
            sc << config.getColor("compact_disk_storage", "capacity.prefixes.prefix_color", "")
               << config.getPrefix("compact_disk_storage", "capacity.prefixes.prefix", "") << r;
        }

        // Capacity label
        sc << config.getColor("compact_disk_storage", "capacity.label.color", "")
           << config.getLabel("compact_disk_storage", "capacity.label.text", "Disk Cap") << r;

        // Capacity separator
        sc << config.getColor("compact_disk_storage", "capacity.separator.color", "")
           << config.getPrefix("compact_disk_storage", "capacity.separator.text", ":") << r
           << config.getColor("compact_disk_storage", "capacity.separator.suffix_color", "")
           << config.getPrefix("compact_disk_storage", "capacity.separator.suffix", " ") << r;

        // Per-disk entry, fully JSON-driven
        for (const auto& c : caps) {
            sc << config.getColor("compact_disk_storage", "capacity.entry.bracket_color", "")
               << config.getPrefix("compact_disk_storage", "capacity.entry.bracket_open", "(") << r

               << config.getColor("compact_disk_storage", "capacity.entry.letter_color", "")
               << c.first[0] << r

               << config.getColor("compact_disk_storage", "capacity.entry.separator_color", "")
               << config.getPrefix("compact_disk_storage", "capacity.entry.separator", "-") << r

               << config.getColor("compact_disk_storage", "capacity.entry.value_color", "")
               << c.second << r

               << config.getColor("compact_disk_storage", "capacity.entry.unit_color", "")
               << config.getLabel("compact_disk_storage", "capacity.entry.unit", "GB") << r

               << config.getColor("compact_disk_storage", "capacity.entry.bracket_color", "")
               << config.getPrefix("compact_disk_storage", "capacity.entry.bracket_close", ")") << r

               << config.getColor("compact_disk_storage", "capacity.entry.suffix_color", "")
               << config.getPrefix("compact_disk_storage", "capacity.entry.suffix", " ") << r;
        }
        lp.push(sc.str());
    }
};

//  ██████╗ ███████╗████████╗ █████╗ ██╗██╗     ███████╗██████╗     ███╗   ███╗███████╗███╗   ███╗ ██████╗ ██████╗ ██╗   ██╗
//  ██╔══██╗██╔════╝╚══██╔══╝██╔══██╗██║██║     ██╔════╝██╔══██╗    ████╗ ████║██╔════╝████╗ ████║██╔═══██╗██╔══██╗╚██╗ ██╔╝
//  ██║  ██║█████╗     ██║   ███████║██║██║     █████╗  ██████╔╝    ██╔████╔██║█████╗  ██╔████╔██║██║   ██║██████╔╝ ╚████╔╝ 
//  ██║  ██║██╔══╝     ██║   ██╔══██║██║██║     ██╔══╝  ██╔══██╗    ██║╚██╔╝██║██╔══╝  ██║╚██╔╝██║██║   ██║██╔══██╗  ╚██╔╝  
//  ██████╔╝███████╗   ██║   ██║  ██║██║███████╗███████╗██████╔╝    ██║ ╚═╝ ██║███████╗██║ ╚═╝ ██║╚██████╔╝██║  ██║   ██║   
//  ╚═════╝ ╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝╚══════╝╚══════╝╚═════╝     ╚═╝     ╚═╝╚══════╝╚═╝     ╚═╝ ╚═════╝ ╚═╝  ╚═╝   ╚═╝   
//  This section displays comprehensive system memory information:
//  1. SUMMARY  - Total, free, and used percentage of system RAM
//  2. MODULES  - Per-stick capacity, type, and speed for each installed
//     memory module
//
//  Output Example:
//  #- Memory Info -------------------------#
//   (Total: 32 GB) (Free: 18 GB) (Used: 44%)
//   Memory 0 : (Used: 44%) 16GB DDR5 6000MHz
//   Memory 1 : (Used: 44%) 16GB DDR5 6000MHz

sections["detailed_system_memory"] = [&]() {
    if (!config.isEnabled("detailed_system_memory")) return;

    // line spacing json driven
    int spacing = config.getNestedInt("detailed_system_memory","top_line_spacing",0);
    for (int n = 0; n < spacing; n++) {lp.push("");}

    // ---------- HEADER ----------
    if (config.getNestedBool("detailed_system_memory", "sections.header", true)) {
        ostringstream ss;
        ss << config.getColor("detailed_system_memory", "header.prefix_color", "")
           << config.getPrefix("detailed_system_memory", "header.prefix", "") << r
           << config.getColor("detailed_system_memory", "header.text_color", "")
           << config.getLabel("detailed_system_memory", "header.text", "") << r
           << config.getColor("detailed_system_memory", "header.suffix_color", "")
           << config.getPrefix("detailed_system_memory", "header.suffix", "") << r;
        lp.push(ss.str());
    }

    // ---------- SUMMARY (TOTAL, FREE + bar, USED% + bar) ----------
    if (config.getNestedBool("detailed_system_memory", "sections.total", true) ||
        config.getNestedBool("detailed_system_memory", "sections.free", true) ||
        config.getNestedBool("detailed_system_memory", "sections.used_percentage", true)) {

        ostringstream ss;

        float usedPercent = ram.getUsedPercentage();
        float freePercent = 100.0f - usedPercent;

        // Generic helper: prints one bracketed "(Label [bar] value unit)" block, fully JSON-driven per field
        auto addMemField = [&](const string& field, auto val, bool hasVisual, float percent) {
            if (!config.getNestedBool("detailed_system_memory", "sections." + field, true)) return;

            ss << config.getColor("detailed_system_memory", "fields." + field + ".prefix_color", "")
               << config.getPrefix("detailed_system_memory", "fields." + field + ".prefix", "") << r

               << config.getColor("detailed_system_memory", "fields." + field + ".bracket_color", "")
               << config.getPrefix("detailed_system_memory", "fields." + field + ".bracket_open", "(") << r

               << config.getColor("detailed_system_memory", "fields." + field + ".label_color", "")
               << config.getLabel("detailed_system_memory", "fields." + field + ".label", "") << r;

            if (hasVisual) {
                std::string bar = makeVisualizer(percent, config, "detailed_system_memory", field);
                if (!bar.empty()) ss << bar << " ";
            }

            ss << config.getColor("detailed_system_memory", "fields." + field + ".value_color", "")
               << val << r

               << config.getColor("detailed_system_memory", "fields." + field + ".unit_color", "")
               << config.getLabel("detailed_system_memory", "fields." + field + ".unit", "") << r

               << config.getColor("detailed_system_memory", "fields." + field + ".bracket_color", "")
               << config.getPrefix("detailed_system_memory", "fields." + field + ".bracket_close", ")") << r;
        };

        // ---- Register each orderable field as a named lambda ----
        std::map<std::string, std::function<void()>> fields;

        fields["total"] = [&]() {
            addMemField("total", ram.getTotal(), false, 0.0f);
        };

        fields["free"] = [&]() {
            addMemField("free", ram.getFree(), true, freePercent);
        };

        fields["used_percentage"] = [&]() {
            addMemField("used_percentage", static_cast<int>(usedPercent), true, usedPercent);
        };

        // ---- Run fields in the order JSON specifies, with spacing controlled by trailing spaces in each entry ----
        static const std::vector<std::string> defaultOrder =
            {"total ", "free ", "used_percentage"};
        auto order = config.getStringArray("detailed_system_memory", "order", defaultOrder);

        runOrderedFields(order, fields, ss);

        lp.push(ss.str());
    }

    // ---------- MODULES (each module line is separately orderable) ----------
    if (config.getNestedBool("detailed_system_memory", "sections.modules", true)) {
        const auto& modules = ram.getModules();

        for (size_t i = 0; i < modules.size(); ++i) {
            string cap = modules[i].capacity;
            int num = 0;
            try { num = stoi(cap); }
            catch (...) { num = 0; }
            ostringstream capNum;
            capNum << setw(2) << setfill('0') << num;

            ostringstream ss;

            // ---- Register each orderable module sub-field as a named lambda ----
            std::map<std::string, std::function<void()>> moduleFields;

            moduleFields["label"] = [&]() {
                ss << config.getColor("detailed_system_memory", "modules.fields.label.prefix_color", "")
                   << config.getPrefix("detailed_system_memory", "modules.fields.label.prefix", "") << r
                   << config.getColor("detailed_system_memory", "modules.fields.label.text_color", "")
                   << config.getLabel("detailed_system_memory", "modules.fields.label.text", "Memory ") << i << r
                   << config.getColor("detailed_system_memory", "modules.fields.label.separator_color", "")
                   << config.getPrefix("detailed_system_memory", "modules.fields.label.separator", " : ") << r;
            };

            moduleFields["used"] = [&]() {
                ss << config.getColor("detailed_system_memory", "modules.fields.used.bracket_color", "")
                   << config.getPrefix("detailed_system_memory", "modules.fields.used.bracket_open", "(") << r
                   << config.getColor("detailed_system_memory", "modules.fields.used.label_color", "")
                   << config.getLabel("detailed_system_memory", "modules.fields.used.label", "Used: ") << r
                   << config.getColor("detailed_system_memory", "modules.fields.used.value_color", "")
                   << ram.getUsedPercentage()
                   << config.getColor("detailed_system_memory", "modules.fields.used.unit_color", "")
                   << config.getLabel("detailed_system_memory", "modules.fields.used.unit", "%") << r
                   << config.getColor("detailed_system_memory", "modules.fields.used.bracket_color", "")
                   << config.getPrefix("detailed_system_memory", "modules.fields.used.bracket_close", ")") << r;
            };

            moduleFields["capacity"] = [&]() {
                ss << config.getColor("detailed_system_memory", "modules.fields.capacity.value_color", "")
                   << capNum.str() << r
                   << config.getColor("detailed_system_memory", "modules.fields.capacity.unit_color", "")
                   << config.getLabel("detailed_system_memory", "modules.fields.capacity.unit", "GB") << r;
            };

            moduleFields["type"] = [&]() {
                ss << config.getColor("detailed_system_memory", "modules.fields.type.value_color", "")
                   << modules[i].type << r;
            };

            moduleFields["speed"] = [&]() {
                ss << config.getColor("detailed_system_memory", "modules.fields.speed.value_color", "")
                   << modules[i].speed << r;
            };

            // ---- Run module sub-fields in the order JSON specifies (independent order, separate from the summary line) ----
            static const std::vector<std::string> defaultModuleOrder =
                {"label", "used ", "capacity ", "type ", "speed"};
            auto moduleOrder = config.getStringArray("detailed_system_memory", "modules.order", defaultModuleOrder);

            runOrderedFields(moduleOrder, moduleFields, ss);

            lp.push(ss.str());
        }
    }
};


//  ██████╗ ███████╗████████╗ █████╗ ██╗██╗     ███████╗██████╗     ██████╗ ██╗███████╗██╗  ██╗
//  ██╔══██╗██╔════╝╚══██╔══╝██╔══██╗██║██║     ██╔════╝██╔══██╗    ██╔══██╗██║██╔════╝██║ ██╔╝
//  ██║  ██║█████╗     ██║   ███████║██║██║     █████╗  ██████╔╝    ██║  ██║██║███████╗█████╔╝ 
//  ██║  ██║██╔══╝     ██║   ██╔══██║██║██║     ██╔══╝  ██╔══██╗    ██║  ██║██║╚════██║██╔═██╗ 
//  ██████╔╝███████╗   ██║   ██║  ██║██║███████╗███████╗██████╔╝    ██████╔╝██║███████║██║  ██╗
//  ╚═════╝ ╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝╚══════╝╚══════╝╚═════╝     ╚═════╝ ╚═╝╚══════╝╚═╝  ╚═╝
//                         D E T A I L E D   S T O R A G E
//  This section displays comprehensive disk information in two main parts:
//  1. STORAGE SUMMARY - Shows each disk with capacity, usage, file system,
//     and external/internal status
//  2. DISK PERFORMANCE - Displays read/write speeds and serial numbers
//  3. PREDICTED PERFORMANCE - Estimated speeds (if enabled)
//
//  Output Example:
//  ------------------------- STORAGE SUMMARY --------------------------
//   SSD Disk (C:) [ (Used)  218.90 GiB / 237.10 GiB    92% - NTFS  Int ]
//   HDD Disk (D:) [ (Used)  189.10 GiB / 465.76 GiB    40% - NTFS  Int ]
//   USB Disk (G:) [ (Used)  104.02 GiB / 112.64 GiB    92% - NTFS  Ext ]
//
//   -------------------- DISK PERFORMANCE & DETAILS --------------------
//  Disk (C:) [ Read: 1225.44 MB/s | Write:  131.03 MB/s | SN-1000 Int ]
//  Disk (D:) [ Read:  128.76 MB/s | Write:  111.68 MB/s | SN-1001 Int ]
//  Disk (G:) [ Read:  151.20 MB/s | Write:    3.73 MB/s | SN-1002 Ext ]

// ----------------- DETAILED STORAGE SECTION -----------------

sections["detailed_disk_storage"] = [&]() {
    if (!config.isEnabled("detailed_disk_storage")) return;

    auto fmt_storage = [](const string& v) -> string {
        ostringstream o; double n = 0.0;
        try { n = stod(v); } catch (...) { n = 0.0; }
        o << fixed << setprecision(2) << setw(7) << right << setfill(' ') << n;
        return o.str();
    };

    auto fmt_speed = [](const string& v) -> string {
        ostringstream o; double n = 0.0;
        try { n = stod(v); } catch (...) { n = 0.0; }
        o << fixed << setprecision(2) << n;
        string s = o.str();
        int pad = 7 - static_cast<int>(s.size());
        if (pad < 0) pad = 0;
        return string(pad, ' ') + s;
    };

    auto fmt_percentage = [](int p) -> string {
        ostringstream o; o << right << setw(4) << p << "%";
        return o.str();
    };

    // zero-padded percentage for the used/free visualizer bars (e.g. 08 instead of 8)
    auto fmt_pct2 = [](float p) -> string {
        ostringstream o; o << setw(2) << setfill('0') << static_cast<int>(p);
        return o.str();
    };

    vector<storage_data> all_disks_captured;

    //  STORAGE SUMMARY 
    if (config.getNestedBool("detailed_disk_storage", "sections.storage_summary", true)) {
        int spacing = config.getNestedInt("detailed_disk_storage", "storage_summary.top_line_spacing", 0);
        for (int n = 0; n < spacing; n++) lp.push("");

        if (config.getNestedBool("detailed_disk_storage", "storage_summary.header.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("detailed_disk_storage", "storage_summary.header.prefix_color", "") << config.getPrefix("detailed_disk_storage", "storage_summary.header.prefix", "") << r
               << config.getNestedColor("detailed_disk_storage", "storage_summary.header.text_color", "") << config.getLabel("detailed_disk_storage", "storage_summary.header.text", "") << r
               << config.getNestedColor("detailed_disk_storage", "storage_summary.header.suffix_color", "") << config.getPrefix("detailed_disk_storage", "storage_summary.header.suffix", "") << r;
            lp.push(ss.str());
        }

        storage.process_storage_info([&](const storage_data& d) {
            all_disks_captured.push_back(d);
            ostringstream ss;
            std::map<std::string, std::function<void()>> fields;

            fields["storage_type"] = [&]() {
                if (!config.getNestedBool("detailed_disk_storage", "storage_summary.fields.storage_type.show", true)) return;
                ss << config.getNestedColor("detailed_disk_storage", "storage_summary.fields.storage_type.value_color", "") << d.storage_type << r;
            };

            fields["drive_letter"] = [&]() {
                if (!config.getNestedBool("detailed_disk_storage", "storage_summary.fields.drive_letter.show", true)) return;
                ss << config.getNestedColor("detailed_disk_storage", "storage_summary.fields.drive_letter.label_color", "") << config.getLabel("detailed_disk_storage", "storage_summary.fields.drive_letter.label", "Disk") << r
                   << config.getNestedColor("detailed_disk_storage", "storage_summary.fields.drive_letter.letter_prefix_color", "") << config.getPrefix("detailed_disk_storage", "storage_summary.fields.drive_letter.letter_prefix", "(") << r
                   << config.getNestedColor("detailed_disk_storage", "storage_summary.fields.drive_letter.letter_color", "") << d.drive_letter << r
                   << config.getNestedColor("detailed_disk_storage", "storage_summary.fields.drive_letter.letter_suffix_color", "") << config.getPrefix("detailed_disk_storage", "storage_summary.fields.drive_letter.letter_suffix", ")") << r;
            };

            fields["usage_block"] = [&]() {
                ss << config.getNestedColor("detailed_disk_storage", "storage_summary.brackets.square_color", "") << config.getPrefix("detailed_disk_storage", "storage_summary.brackets.square_open", "[") << r;

                if (config.getNestedBool("detailed_disk_storage", "storage_summary.fields.used_label.show", true)) {
                    ss << config.getNestedColor("detailed_disk_storage", "storage_summary.fields.used_label.prefix_color", "") << config.getPrefix("detailed_disk_storage", "storage_summary.fields.used_label.prefix", " ") << r
                       << config.getNestedColor("detailed_disk_storage", "storage_summary.brackets.round_color", "") << config.getPrefix("detailed_disk_storage", "storage_summary.brackets.round_open", "(") << r
                       << config.getNestedColor("detailed_disk_storage", "storage_summary.fields.used_label.value_color", "") << config.getLabel("detailed_disk_storage", "storage_summary.fields.used_label.text", "") << r
                       << config.getNestedColor("detailed_disk_storage", "storage_summary.brackets.round_color", "") << config.getPrefix("detailed_disk_storage", "storage_summary.brackets.round_close", ")") << r
                       << config.getNestedColor("detailed_disk_storage", "storage_summary.fields.used_label.suffix_color", "") << config.getPrefix("detailed_disk_storage", "storage_summary.fields.used_label.suffix", " ") << r;
                }

                if (config.getNestedBool("detailed_disk_storage", "storage_summary.fields.used_space.show", true)) {
                    ss << config.getNestedColor("detailed_disk_storage", "storage_summary.fields.used_space.value_color", "") << fmt_storage(d.used_space) << r
                       << config.getNestedColor("detailed_disk_storage", "storage_summary.fields.used_space.unit_color", "") << config.getLabel("detailed_disk_storage", "storage_summary.fields.used_space.unit", "") << r;
                }

                if (config.getNestedBool("detailed_disk_storage", "storage_summary.separator.show", true)) {
                    ss << config.getNestedColor("detailed_disk_storage", "storage_summary.separator.color", "") << config.getPrefix("detailed_disk_storage", "storage_summary.separator.text", "") << r;
                }

                if (config.getNestedBool("detailed_disk_storage", "storage_summary.fields.total_space.show", true)) {
                    ss << config.getNestedColor("detailed_disk_storage", "storage_summary.fields.total_space.value_color", "") << fmt_storage(d.total_space) << r
                       << config.getNestedColor("detailed_disk_storage", "storage_summary.fields.total_space.unit_color", "") << config.getLabel("detailed_disk_storage", "storage_summary.fields.total_space.unit", "") << r;
                }

                if (config.getNestedBool("detailed_disk_storage", "storage_summary.fields.used_percentage.show", true)) {
                    ss << config.getNestedColor("detailed_disk_storage", "storage_summary.fields.used_percentage.spacer_color", "") << config.getPrefix("detailed_disk_storage", "storage_summary.fields.used_percentage.spacer", " ") << r
                       << config.getNestedColor("detailed_disk_storage", "storage_summary.fields.used_percentage.prefix_color", "") << config.getPrefix("detailed_disk_storage", "storage_summary.fields.used_percentage.prefix", "") << r
                       << config.getNestedColor("detailed_disk_storage", "storage_summary.fields.used_percentage.value_color", "") << fmt_percentage(d.used_percentage) << r
                       << config.getNestedColor("detailed_disk_storage", "storage_summary.fields.used_percentage.suffix_color", "") << config.getPrefix("detailed_disk_storage", "storage_summary.fields.used_percentage.suffix", "") << r;
                }

                ss << config.getNestedColor("detailed_disk_storage", "storage_summary.dash.color", "") << config.getPrefix("detailed_disk_storage", "storage_summary.dash.text", "") << r
                   << config.getNestedColor("detailed_disk_storage", "storage_summary.dash.suffix_color", "") << config.getPrefix("detailed_disk_storage", "storage_summary.dash.suffix", " ") << r;

                if (config.getNestedBool("detailed_disk_storage", "storage_summary.fields.file_system.show", true)) {
                    ss << config.getNestedColor("detailed_disk_storage", "storage_summary.fields.file_system.value_color", "") << d.file_system << r
                       << config.getNestedColor("detailed_disk_storage", "storage_summary.fields.file_system.suffix_color", "") << config.getPrefix("detailed_disk_storage", "storage_summary.fields.file_system.suffix", " ") << r;
                }

                if (config.getNestedBool("detailed_disk_storage", "storage_summary.fields.external_status.show", true)) {
                    if (d.is_external)
                        ss << config.getNestedColor("detailed_disk_storage", "storage_summary.fields.external_status.external_color", "") << config.getLabel("detailed_disk_storage", "storage_summary.fields.external_status.external_text", "") << r;
                    else
                        ss << config.getNestedColor("detailed_disk_storage", "storage_summary.fields.external_status.internal_color", "") << config.getLabel("detailed_disk_storage", "storage_summary.fields.external_status.internal_text", "") << r;
                }

                ss << config.getNestedColor("detailed_disk_storage", "storage_summary.brackets.square_close_prefix_color", "") << config.getPrefix("detailed_disk_storage", "storage_summary.brackets.square_close_prefix", " ") << r
                   << config.getNestedColor("detailed_disk_storage", "storage_summary.brackets.square_color", "") << config.getPrefix("detailed_disk_storage", "storage_summary.brackets.square_close", "]") << r;
            };

            auto addUsageVisualizer = [&](const string& field, float percent) {
                if (!config.getNestedBool("detailed_disk_storage", "fields." + field + ".show", true)) return;

                ss << config.getNestedColor("detailed_disk_storage", "fields." + field + ".prefix_color", "") << config.getPrefix("detailed_disk_storage", "fields." + field + ".prefix", "") << r
                   << config.getNestedColor("detailed_disk_storage", "fields." + field + ".label_color", "") << config.getLabel("detailed_disk_storage", "fields." + field + ".label", "") << r;

                std::string bar = makeVisualizer(percent, config, "detailed_disk_storage", field);
                if (!bar.empty()) {
                    ss << bar << config.getNestedColor("detailed_disk_storage", "fields." + field + ".bar_gap_color", "") << config.getPrefix("detailed_disk_storage", "fields." + field + ".bar_gap", " ") << r;
                }

                ss << config.getNestedColor("detailed_disk_storage", "fields." + field + ".value_color", "") << fmt_pct2(percent) << r
                   << config.getNestedColor("detailed_disk_storage", "fields." + field + ".unit_color", "") << config.getLabel("detailed_disk_storage", "fields." + field + ".unit", "%") << r
                   << config.getNestedColor("detailed_disk_storage", "fields." + field + ".suffix_color", "") << config.getPrefix("detailed_disk_storage", "fields." + field + ".suffix", "") << r;
            };

            fields["used_visualizer"] = [&]() { addUsageVisualizer("used_visualizer", static_cast<float>(d.used_percentage)); };
            fields["free_visualizer"] = [&]() { addUsageVisualizer("free_visualizer", 100.0f - static_cast<float>(d.used_percentage)); };

            static const std::vector<std::string> defaultOrder = {"storage_type ", "drive_letter ", "usage_block ", "used_visualizer ", "free_visualizer"};
            auto order = config.getStringArray("detailed_disk_storage", "storage_summary.order", defaultOrder);
            runOrderedFields(order, fields, ss);

            lp.push(ss.str());
        });
    }

    //  DISK PERFORMANCE 
    if (!all_disks_captured.empty() && config.getNestedBool("detailed_disk_storage", "sections.disk_performance", true)) {
        int spacing = config.getNestedInt("detailed_disk_storage", "disk_performance.top_line_spacing", 0);
        for (int n = 0; n < spacing; n++) lp.push("");

        if (config.getNestedBool("detailed_disk_storage", "disk_performance.header.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("detailed_disk_storage", "disk_performance.header.prefix_color", "") << config.getPrefix("detailed_disk_storage", "disk_performance.header.prefix", "") << r
               << config.getNestedColor("detailed_disk_storage", "disk_performance.header.text_color", "") << config.getLabel("detailed_disk_storage", "disk_performance.header.text", "") << r
               << config.getNestedColor("detailed_disk_storage", "disk_performance.header.suffix_color", "") << config.getPrefix("detailed_disk_storage", "disk_performance.header.suffix", "") << r;
            lp.push(ss.str());
        }

        for (const auto& d : all_disks_captured) {
            ostringstream ss;

            if (config.getNestedBool("detailed_disk_storage", "disk_performance.fields.drive_letter.show", true)) {
                ss << config.getNestedColor("detailed_disk_storage", "disk_performance.fields.drive_letter.label_color", "") << config.getLabel("detailed_disk_storage", "disk_performance.fields.drive_letter.label", "Disk") << r
                   << config.getNestedColor("detailed_disk_storage", "disk_performance.fields.drive_letter.letter_prefix_color", "") << config.getPrefix("detailed_disk_storage", "disk_performance.fields.drive_letter.letter_prefix", "(") << r
                   << config.getNestedColor("detailed_disk_storage", "disk_performance.fields.drive_letter.letter_color", "") << d.drive_letter << r
                   << config.getNestedColor("detailed_disk_storage", "disk_performance.fields.drive_letter.letter_suffix_color", "") << config.getPrefix("detailed_disk_storage", "disk_performance.fields.drive_letter.letter_suffix", ")") << r
                   << " ";
            }

            ss << config.getNestedColor("detailed_disk_storage", "disk_performance.brackets.square_color", "") << config.getPrefix("detailed_disk_storage", "disk_performance.brackets.square_open", "[") << r << " ";

            if (config.getNestedBool("detailed_disk_storage", "disk_performance.fields.read_speed.show", true)) {
                ss << config.getNestedColor("detailed_disk_storage", "disk_performance.fields.read_speed.label_color", "") << config.getLabel("detailed_disk_storage", "disk_performance.fields.read_speed.label", "") << r
                   << config.getNestedColor("detailed_disk_storage", "disk_performance.fields.read_speed.value_color", "") << fmt_speed(d.read_speed) << r
                   << " " << config.getNestedColor("detailed_disk_storage", "disk_performance.fields.read_speed.unit_color", "") << config.getLabel("detailed_disk_storage", "disk_performance.fields.read_speed.unit", "") << r;
            }

            bool wantsWrite = config.getNestedBool("detailed_disk_storage", "disk_performance.fields.write_speed.show", true);
            bool wantsSerial = config.getNestedBool("detailed_disk_storage", "disk_performance.fields.serial_number.show", true);
            bool wantsStatus = config.getNestedBool("detailed_disk_storage", "disk_performance.fields.external_status.show", true);
            bool pipeEnabled = config.getNestedBool("detailed_disk_storage", "disk_performance.pipe.show", true);

            if (pipeEnabled && (wantsWrite || wantsSerial || wantsStatus)) {
                ss << " " << config.getNestedColor("detailed_disk_storage", "disk_performance.pipe.color", "") << config.getPrefix("detailed_disk_storage", "disk_performance.pipe.text", "|") << r << " ";
            }

            if (wantsWrite) {
                ss << config.getNestedColor("detailed_disk_storage", "disk_performance.fields.write_speed.label_color", "") << config.getLabel("detailed_disk_storage", "disk_performance.fields.write_speed.label", "") << r
                   << config.getNestedColor("detailed_disk_storage", "disk_performance.fields.write_speed.value_color", "") << fmt_speed(d.write_speed) << r
                   << " " << config.getNestedColor("detailed_disk_storage", "disk_performance.fields.write_speed.unit_color", "") << config.getLabel("detailed_disk_storage", "disk_performance.fields.write_speed.unit", "") << r;
            }

            if (pipeEnabled && (wantsSerial || wantsStatus)) {
                ss << " " << config.getNestedColor("detailed_disk_storage", "disk_performance.pipe.color", "") << config.getPrefix("detailed_disk_storage", "disk_performance.pipe.text", "|") << r << " ";
            }

            if (wantsSerial) {
                ss << config.getNestedColor("detailed_disk_storage", "disk_performance.fields.serial_number.value_color", "") << config.getPrefix("detailed_disk_storage", "disk_performance.fields.serial_number.prefix", "") << d.serial_number << r;
            }

            if (wantsStatus) {
                ss << " ";
                if (d.is_external)
                    ss << config.getNestedColor("detailed_disk_storage", "disk_performance.fields.external_status.external_color", "") << config.getLabel("detailed_disk_storage", "disk_performance.fields.external_status.external_text", "") << r;
                else
                    ss << config.getNestedColor("detailed_disk_storage", "disk_performance.fields.external_status.internal_color", "") << config.getLabel("detailed_disk_storage", "disk_performance.fields.external_status.internal_text", "") << r;
            }

            ss << " " << config.getNestedColor("detailed_disk_storage", "disk_performance.brackets.square_color", "") << config.getPrefix("detailed_disk_storage", "disk_performance.brackets.square_close", "]") << r;

            lp.push(ss.str());
        }
    }

    if (all_disks_captured.empty()) {
        lp.push(config.getLabel("detailed_disk_storage", "no_drives", ""));
    }
};



//  ███╗   ██╗███████╗████████╗██╗    ██╗ ██████╗ ██████╗ ██╗  ██╗
//  ████╗  ██║██╔════╝╚══██╔══╝██║    ██║██╔═══██╗██╔══██╗██║ ██╔╝
//  ██╔██╗ ██║█████╗     ██║   ██║ █╗ ██║██║   ██║██████╔╝█████╔╝ 
//  ██║╚██╗██║██╔══╝     ██║   ██║███╗██║██║   ██║██╔══██╗██╔═██╗ 
//  ██║ ╚████║███████╗   ██║   ╚███╔███╔╝╚██████╔╝██║  ██║██║  ██╗
//  ╚═╝  ╚═══╝╚══════╝   ╚═╝    ╚══╝╚══╝  ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝
//                      D E T A I L E D   N E T W O R K
//  This section displays comprehensive network information including:
//  1. Network Name      - The name of the active network connection
//  2. Network Type      - Type of network (Ethernet, Wi-Fi, etc.)
//  3. Local IP Address  - The local IPv4 address of the machine
//  4. Public IP Address - The external/public IP address
//  5. Locale            - The geographic location based on public IP
//  6. MAC Address       - The physical hardware address of the adapter
//  7. Upload Speed      - The average upload speed of the connection
//  8. Download Speed    - The average download speed of the connection
//
//  Output Example:
//  #- Network Info ---------------------------------------------------#
//  ~ Network Name            : Ethernet
//  ~ Network Type            : Ethernet
//  ~ Local IP                : 192.168.1.100
//  ~ Public IP:              : 203.0.113.42
//  ~ Locale                  : US, California
//  ~ Mac address             : 00:1A:2B:3C:4D:5E
//  ~ avg upload speed        : 10.5 Mbps
//  ~ avg download speed      : 85.2 Mbps
sections["detailed_network_connection"] = [&]() {
    if (!config.isEnabled("detailed_network_connection")) return;
    // line spacing json driven
    int spacing = config.getNestedInt("detailed_network_connection","top_line_spacing",0);
    for (int n = 0; n < spacing; n++) {lp.push("");}

    const string sec = "detailed_network_connection";

    // Network header
    if (config.getNestedBool(sec, "header.show", true)) {
        ostringstream ss;
        ss << config.getColor(sec, "header.prefix_color", "") << config.getPrefix(sec, "header.prefix", "") << r
           << config.getColor(sec, "header.text_color", "")   << config.getLabel(sec, "header.text", "")     << r
           << config.getColor(sec, "header.suffix_color", "") << config.getPrefix(sec, "header.suffix", "")  << r;
        lp.push(ss.str());
    }

    // Generic field printer: fields.<key>.{<pfx_key>_prefix, label, label_suffix, value_suffix} + colors
    auto field = [&](const string& key, const string& pfxKey, const string& value) {
        if (!config.getNestedBool(sec, "fields." + key + ".show", true)) return;

        ostringstream ss;
        ss << config.getColor(sec, "fields." + key + "." + pfxKey + "_prefix_color", "")
           << config.getPrefix(sec, "fields." + key + "." + pfxKey + "_prefix", "") << r

           << config.getColor(sec, "fields." + key + ".label_color", "")
           << config.getLabel(sec, "fields." + key + ".label", "") << r

           << config.getColor(sec, "fields." + key + ".label_suffix_color", "")
           << config.getPrefix(sec, "fields." + key + ".label_suffix", "") << r

           << config.getColor(sec, "fields." + key + ".value_color", "")
           << value << r

           << config.getColor(sec, "fields." + key + ".value_suffix_color", "")
           << config.getPrefix(sec, "fields." + key + ".value_suffix", "") << r;

        lp.push(ss.str());
    };

    // ---- Register each orderable field as a named lambda ----
    std::map<std::string, std::function<void()>> fields;

    fields["name"]      = [&]() { field("name",      "name",      net.get_network_name()); };
    fields["type"]      = [&]() { field("type",      "type",      c_net.get_network_type()); };
    fields["local_ip"]  = [&]() { field("local_ip",  "local_ip",  net.get_local_ip()); };
    fields["public_ip"] = [&]() { field("public_ip", "public_ip", net.get_public_ip()); };
    fields["locale"]    = [&]() { field("locale",    "locale",    net.get_locale()); };
    fields["mac"]       = [&]() { field("mac",       "mac",       net.get_mac_address()); };
    fields["upload"]    = [&]() { field("upload",    "upload",    net.get_network_upload_speed()); };
    fields["download"]  = [&]() { field("download",  "download",  net.get_network_download_speed()); };

    // ---- Run fields in the order JSON specifies ----
    // Each field prints its own line (unlike the compact sections), so no
    // trailing-space spacing trick is needed here — just sequence control.
    static const std::vector<std::string> defaultOrder =
        {"name", "type", "local_ip", "public_ip", "locale", "mac", "upload", "download"};
    auto order = config.getStringArray(sec, "order", defaultOrder);

    for (const auto& key : order) {
        auto it = fields.find(key);
        if (it != fields.end()) it->second();
    }
};

//  ██████╗ ██╗   ██╗███╗   ███╗███╗   ███╗██╗   ██╗
//  ██╔══██╗██║   ██║████╗ ████║████╗ ████║╚██╗ ██╔╝
//  ██║  ██║██║   ██║██╔████╔██║██╔████╔██║ ╚████╔╝ 
//  ██║  ██║██║   ██║██║╚██╔╝██║██║╚██╔╝██║  ╚██╔╝  
//  ██████╔╝╚██████╔╝██║ ╚═╝ ██║██║ ╚═╝ ██║   ██║   
//  ╚═════╝  ╚═════╝ ╚═╝     ╚═╝╚═╝     ╚═╝   ╚═╝   
//                   D E T A I L E D   D U M M Y   N E T W O R K
//  This section displays dummy/example network information for testing:
//  1. Network Name      - Example: "InterCentury"
//  2. Network Type      - Example: "Ethernet"
//  3. Local IP Address  - Example: "192.168.1.42"
//  4. Read Speed         - Example: "812.45 Mbps"
//  5. Write Speed        - Example: "634.10 Mbps"
//  All values, labels, colors, prefixes, and units are fully JSON-driven.
//
//  Output Example:
//  #- Network Info ---------------------------------------------------#
//  ~ Network Name            : InterCentury
//  ~ Network Type            : Ethernet
//  ~ Local IP                : 192.168.1.42
//  ~ Read Speed              : 812.45 Mbps
//  ~ Write Speed             : 634.10 Mbps

sections["dummy_network_info"] = [&]() {
    if (!config.isEnabled("dummy_network_info")) return;
    
    // line spacing json driven
    int spacing = config.getNestedInt("dummy_network_info","top_line_spacing",0);
    for (int n = 0; n < spacing; n++) {lp.push("");}

    const string sec = "dummy_network_info";

    // ---------- HEADER ----------
    if (config.getNestedBool(sec, "header.show", true)) {
        ostringstream ss;
        ss << config.getNestedColor(sec, "header.prefix_color", "") << config.getPrefix(sec, "header.prefix", "") << r
           << config.getNestedColor(sec, "header.text_color", "")   << config.getLabel(sec, "header.text", "")     << r
           << config.getNestedColor(sec, "header.suffix_color", "") << config.getPrefix(sec, "header.suffix", "")  << r;
        lp.push(ss.str());
    }

    // Generic field printer: fields.<key>.{prefix, label, label_suffix, value, value_suffix} + colors
    auto field = [&](const string& key) {
        if (!config.getNestedBool(sec, "fields." + key + ".show", true)) return;

        ostringstream ss;
        ss << config.getNestedColor(sec, "fields." + key + ".prefix_color", "")
           << config.getPrefix(sec, "fields." + key + ".prefix", "") << r

           << config.getNestedColor(sec, "fields." + key + ".label_color", "")
           << config.getLabel(sec, "fields." + key + ".label", "") << r

           << config.getNestedColor(sec, "fields." + key + ".label_suffix_color", "")
           << config.getPrefix(sec, "fields." + key + ".label_suffix", "") << r

           << config.getNestedColor(sec, "fields." + key + ".value_color", "")
           << config.getLabel(sec, "fields." + key + ".value", "") << r

           << config.getNestedColor(sec, "fields." + key + ".value_suffix_color", "")
           << config.getLabel(sec, "fields." + key + ".value_suffix", "") << r;

        lp.push(ss.str());
    };

    // ---- Register each orderable field as a named lambda ----
    std::map<std::string, std::function<void()>> fields;

    fields["name"]        = [&]() { field("name"); };
    fields["type"]        = [&]() { field("type"); };
    fields["local_ip"]    = [&]() { field("local_ip"); };
    fields["read_speed"]  = [&]() { field("read_speed"); };
    fields["write_speed"] = [&]() { field("write_speed"); };

    // ---- Run fields in the order JSON specifies ----
    // Each field prints its own line, so this only controls sequence,
    // not inter-field spacing (unlike the compact single-line sections).
    static const std::vector<std::string> defaultOrder =
        {"name", "type", "local_ip", "read_speed", "write_speed"};
    auto order = config.getStringArray(sec, "order", defaultOrder);

    for (const auto& key : order) {
        auto it = fields.find(key);
        if (it != fields.end()) it->second();
    }
};

//   ██████╗ ███████╗    ██╗███╗   ██╗███████╗ ██████╗ 
//  ██╔═══██╗██╔════╝    ██║████╗  ██║██╔════╝██╔═══██╗
//  ██║   ██║███████╗    ██║██╔██╗ ██║█████╗  ██║   ██║
//  ██║   ██║╚════██║    ██║██║╚██╗██║██╔══╝  ██║   ██║
//  ╚██████╔╝███████║    ██║██║ ╚████║██║     ╚██████╔╝
//   ╚═════╝ ╚══════╝    ╚═╝╚═╝  ╚═══╝╚═╝      ╚═════╝ 
//                    D E T A I L E D   O P E R A T I N G   S Y S T E M
//  This section displays comprehensive OS information including:
//  1. Name          - OS name (e.g., "Windows 11 Pro")
//  2. Build         - OS build/version number
//  3. Architecture  - System architecture (e.g., "64-bit")
//  4. Kernel        - Kernel version info
//  5. Uptime        - System uptime since last boot
//  6. Install Date  - OS installation date
//  7. Serial        - OS serial number
//  All labels, values, colors, prefixes, and toggles are fully JSON-driven
//  via the "detailed_operating_system" config block (aliased as "detailed_operating_system").
//
//  Output Example:
//  #- Operating System -----------------------------------------#
//  ~ Name                    : Windows 11 Pro
//  ~ Build                   : 22631.3737
//  ~ Architecture             : 64-bit
//  ~ Kernel                  : 10.0.22631
//  ~ Uptime                  : 3d 4h 12m
//  ~ Install Date            : 2024-01-15
//  ~ Serial                  : XXXXX-XXXXX-XXXXX-XXXXX
sections["detailed_operating_system"] = [&]() {
    if (!config.isEnabled("detailed_operating_system")) return;
        
        // line spacing json driven
        int spacing = config.getNestedInt("detailed_operating_system","top_line_spacing",0);
        for (int n = 0; n < spacing; n++) {lp.push("");}

        const string sec = "detailed_operating_system";

        // Header
        if (config.getNestedBool(sec, "header.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor(sec, "header.prefix_color", "") << config.getPrefix(sec, "header.prefix", "") << r
               << config.getNestedColor(sec, "header.text_color", "")   << config.getLabel(sec, "header.text", "")     << r
               << config.getNestedColor(sec, "header.suffix_color", "") << config.getPrefix(sec, "header.suffix", "")  << r;
            lp.push(ss.str());
        }

        // Generic field printer: fields.<key>.{<pfx>_prefix, label, label_suffix, value_suffix} + colors
        auto field = [&](const string& key, const string& pfxKey, const string& value) {
            if (!config.getNestedBool(sec, "fields." + key + ".show", true)) return;

            ostringstream ss;
            ss << config.getNestedColor(sec, "fields." + key + "." + pfxKey + "_prefix_color", "")
               << config.getPrefix(sec, "fields." + key + "." + pfxKey + "_prefix", "") << r

               << config.getNestedColor(sec, "fields." + key + ".label_color", "")
               << config.getLabel(sec, "fields." + key + ".label", "") << r

               << config.getNestedColor(sec, "fields." + key + ".label_suffix_color", "")
               << config.getPrefix(sec, "fields." + key + ".label_suffix", "") << r

               << config.getNestedColor(sec, "fields." + key + ".value_color", "")
               << value << r

               << config.getNestedColor(sec, "fields." + key + ".value_suffix_color", "")
               << config.getPrefix(sec, "fields." + key + ".value_suffix", "") << r;

            lp.push(ss.str());
        };

        // ---- Register each orderable field as a named lambda ----
        std::map<std::string, std::function<void()>> fields;

        fields["name"]         = [&]() { field("name",         "name",         os.GetOSName()); };
        fields["build"]        = [&]() { field("build",        "build",        os.GetOSVersion()); };
        fields["architecture"] = [&]() { field("architecture", "architecture", os.GetOSArchitecture()); };
        fields["kernel"]       = [&]() { field("kernel",       "kernel",       os.get_os_kernel_info()); };
        fields["uptime"]       = [&]() { field("uptime",       "uptime",       os.get_os_uptime()); };
        fields["install_date"] = [&]() { field("install_date", "install_date", os.get_os_install_date()); };
        fields["serial"]       = [&]() { field("serial",       "serial",       os.get_os_serial_number()); };

        // ---- Run fields in the order JSON specifies ----
        static const std::vector<std::string> defaultOrder =
            {"name", "build", "architecture", "kernel", "uptime", "install_date", "serial"};
        auto order = config.getStringArray(sec, "order", defaultOrder);

        for (const auto& key : order) {
            auto it = fields.find(key);
            if (it != fields.end()) it->second();
        }
    };
//   ██████╗██████╗ ██╗   ██╗    ██╗███╗   ██╗███████╗ ██████╗ 
//  ██╔════╝██╔══██╗██║   ██║    ██║████╗  ██║██╔════╝██╔═══██╗
//  ██║     ██████╔╝██║   ██║    ██║██╔██╗ ██║█████╗  ██║   ██║
//  ██║     ██╔═══╝ ██║   ██║    ██║██║╚██╗██║██╔══╝  ██║   ██║
//  ╚██████╗██║     ╚██████╔╝    ██║██║ ╚████║██║     ╚██████╔╝
//   ╚═════╝╚═╝      ╚═════╝     ╚═╝╚═╝  ╚═══╝╚═╝      ╚═════╝ 
//                       D E T A I L E D   P R O C E S S O R

sections["detailed_processor"] = [&]() {
    if (!config.isEnabled("detailed_processor")) return;

    // line spacing json driven
    int spacing = config.getNestedInt("detailed_processor","top_line_spacing",0);
    for (int n = 0; n < spacing; n++) {lp.push("");}

    const string sec = "detailed_processor";

    // Header
    if (config.getNestedBool(sec, "header.show", true)) {
        ostringstream ss;
        ss << config.getNestedColor(sec, "header.prefix_color", "")
           << config.getPrefix(sec, "header.prefix", "") << r
           << config.getNestedColor(sec, "header.text_color", "")
           << config.getLabel(sec, "header.text", "") << r
           << config.getNestedColor(sec, "header.suffix_color", "")
           << config.getPrefix(sec, "header.suffix", "") << r;
        lp.push(ss.str());
    }

    // Generic field printer: fields.<key>.{<pfxKey>_prefix, label, label_suffix, value_suffix} + colors
    auto field = [&](const string& key, const string& pfxKey, const string& value, const string& defaultSuffix = "") {
        if (!config.getNestedBool(sec, "fields." + key + ".show", true)) return;

        ostringstream ss;
        ss << config.getNestedColor(sec, "fields." + key + "." + pfxKey + "_prefix_color", "")
           << config.getPrefix(sec, "fields." + key + "." + pfxKey + "_prefix", "") << r

           << config.getNestedColor(sec, "fields." + key + ".label_color", "")
           << config.getLabel(sec, "fields." + key + ".label", "") << r

           << config.getNestedColor(sec, "fields." + key + ".label_suffix_color", "")
           << config.getPrefix(sec, "fields." + key + ".label_suffix", "") << r

           << config.getNestedColor(sec, "fields." + key + ".value_color", "")
           << value << r

           << config.getNestedColor(sec, "fields." + key + ".value_suffix_color", "")
           << config.getPrefix(sec, "fields." + key + ".value_suffix", defaultSuffix) << r;

        lp.push(ss.str());
    };

    // ---- Register each orderable field as a named lambda ----
    std::map<std::string, std::function<void()>> fields;

    fields["brand"]              = [&]() { field("brand",              "brand",              cpu.get_cpu_info()); };
    fields["utilization"]        = [&]() { field("utilization",        "utilization",        std::to_string(cpu.get_cpu_utilization()), "%"); };
    fields["speed"]              = [&]() { field("speed",              "speed",              cpu.get_cpu_speed()); };
    fields["base_speed"]         = [&]() { field("base_speed",         "base_speed",         cpu.get_cpu_base_speed()); };
    fields["cores"]              = [&]() { field("cores",              "cores",              std::to_string(cpu.get_cpu_cores())); };
    fields["logical_processors"] = [&]() { field("logical_processors", "logical_processors", std::to_string(cpu.get_cpu_logical_processors())); };
    fields["sockets"]            = [&]() { field("sockets",            "sockets",            std::to_string(cpu.get_cpu_sockets())); };
    fields["virtualization"]     = [&]() { field("virtualization",     "virtualization",     cpu.get_cpu_virtualization()); };
    fields["l1_cache"]           = [&]() { field("l1_cache",           "l1_cache",           cpu.get_cpu_l1_cache()); };
    fields["l2_cache"]           = [&]() { field("l2_cache",           "l2_cache",           cpu.get_cpu_l2_cache()); };
    fields["l3_cache"]           = [&]() { field("l3_cache",           "l3_cache",           cpu.get_cpu_l3_cache()); };


    // ---- Run fields in the order JSON specifies ----
    static const std::vector<std::string> defaultOrder =
        {"brand", "utilization", "speed", "base_speed", "cores", "logical_processors", "sockets", "virtualization", "l1_cache", "l2_cache", "l3_cache"};
    auto order = config.getStringArray(sec, "order", defaultOrder);

    for (const auto& key : order) {
        auto it = fields.find(key);
        if (it != fields.end()) it->second();
    }
};

//   ██████╗██████╗ ██╗   ██╗    ██╗███╗   ██╗███████╗ ██████╗ 
//  ██╔════╝██╔══██╗██║   ██║    ██║████╗  ██║██╔════╝██╔═══██╗
//  ██║  ███╗██████╔╝██║   ██║    ██║██╔██╗ ██║█████╗  ██║   ██║
//  ██║   ██║██╔═══╝ ██║   ██║    ██║██║╚██╗██║██╔══╝  ██║   ██║
//  ╚██████╔╝██║     ╚██████╔╝    ██║██║ ╚████║██║     ╚██████╔╝
//   ╚═════╝ ╚═╝      ╚═════╝     ╚═╝╚═╝  ╚═══╝╚═╝      ╚═════╝ 
//                    D E T A I L E D   G R A P H I C S   C A R D
sections["detailed_graphics_card"] = [&]() {
    if (!config.isEnabled("detailed_graphics_card")) return;

    // line spacing json driven
    int spacing = config.getNestedInt("detailed_graphics_card","top_line_spacing",0);
    for (int n = 0; n < spacing; n++) {lp.push("");}

    auto all_gpu_info = obj_gpu.get_all_gpu_info();

    if (all_gpu_info.empty()) {

        // Header
        if (config.getNestedBool("detailed_graphics_card", "header.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("detailed_graphics_card", "header.prefix_color", "")
               << config.getPrefix("detailed_graphics_card", "header.prefix", "") << r
               << config.getNestedColor("detailed_graphics_card", "header.text_color", "")
               << config.getLabel("detailed_graphics_card", "header.text", "") << r
               << config.getNestedColor("detailed_graphics_card", "header.suffix_color", "")
               << config.getPrefix("detailed_graphics_card", "header.suffix", "") << r;
            lp.push(ss.str());
        }

        lp.push(
            config.getColor("detailed_graphics_card", "error_color", "")
            + config.getLabel("detailed_graphics_card", "error_text", "No GPU detected.")
            + r
        );
    }
    else {

        // Main Header
        if (config.getNestedBool("detailed_graphics_card", "header.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("detailed_graphics_card", "header.prefix_color", "")
               << config.getPrefix("detailed_graphics_card", "header.prefix", "") << r
               << config.getNestedColor("detailed_graphics_card", "header.text_color", "")
               << config.getLabel("detailed_graphics_card", "header.text", "") << r
               << config.getNestedColor("detailed_graphics_card", "header.suffix_color", "")
               << config.getPrefix("detailed_graphics_card", "header.suffix", "") << r;
            lp.push(ss.str());
        }

        //   ALL GPUs LOOP  (toggle via "show_gpu_list": true/false)
        //   true  -> loop every detected GPU (original behaviour)
        //   false -> skip the loop entirely
        if (config.getNestedBool("detailed_graphics_card", "show_gpu_list", true)) {

            for (size_t i = 0; i < all_gpu_info.size(); ++i) {
                auto& g = all_gpu_info[i];

                // GPU index line
                if (config.getNestedBool("detailed_graphics_card", "gpu_header.show", true)) {
                    ostringstream label;

                    if (i == 0) {
                        label << config.getNestedColor("detailed_graphics_card", "gpu_header.index_color", "")
                              << config.getLabel("detailed_graphics_card", "gpu_header.text", "GPU ")
                              << (i + 1) << r;
                    }
                    else {
                        label << config.getNestedColor("detailed_graphics_card", "gpu_header.prefix_color", "")
                              << config.getPrefix("detailed_graphics_card", "gpu_header.prefix", "") << r
                              << config.getNestedColor("detailed_graphics_card", "gpu_header.index_color", "")
                              << config.getLabel("detailed_graphics_card", "gpu_header.text", "GPU ")
                              << (i + 1) << r
                              << config.getNestedColor("detailed_graphics_card", "gpu_header.suffix_color", "")
                              << config.getPrefix("detailed_graphics_card", "gpu_header.suffix", "") << r;
                    }

                    lp.push(label.str());
                }

                // ---- Register each orderable field as a named lambda ----
                std::map<std::string, std::function<void()>> fields;

                fields["name"] = [&]() {
                    if (!config.getNestedBool("detailed_graphics_card", "fields.name.show", true)) return;
                    ostringstream ss;
                    ss << config.getNestedColor("detailed_graphics_card", "fields.name.name_prefix_color", "")
                       << config.getPrefix("detailed_graphics_card", "fields.name.name_prefix", "") << r
                       << config.getNestedColor("detailed_graphics_card", "fields.name.label_color", "")
                       << config.getLabel("detailed_graphics_card", "fields.name.label", "") << r
                       << config.getNestedColor("detailed_graphics_card", "fields.name.label_suffix_color", "")
                       << config.getPrefix("detailed_graphics_card", "fields.name.label_suffix", "") << r
                       << config.getNestedColor("detailed_graphics_card", "fields.name.value_color", "")
                       << g.gpu_name << r
                       << config.getNestedColor("detailed_graphics_card", "fields.name.value_suffix_color", "")
                       << config.getPrefix("detailed_graphics_card", "fields.name.value_suffix", "") << r;
                    lp.push(ss.str());
                };

                fields["memory"] = [&]() {
                    if (!config.getNestedBool("detailed_graphics_card", "fields.memory.show", true)) return;
                    ostringstream ss;
                    ss << config.getNestedColor("detailed_graphics_card", "fields.memory.memory_prefix_color", "")
                       << config.getPrefix("detailed_graphics_card", "fields.memory.memory_prefix", "") << r
                       << config.getNestedColor("detailed_graphics_card", "fields.memory.label_color", "")
                       << config.getLabel("detailed_graphics_card", "fields.memory.label", "") << r
                       << config.getNestedColor("detailed_graphics_card", "fields.memory.label_suffix_color", "")
                       << config.getPrefix("detailed_graphics_card", "fields.memory.label_suffix", "") << r
                       << config.getNestedColor("detailed_graphics_card", "fields.memory.value_color", "")
                       << g.gpu_memory << r
                       << config.getNestedColor("detailed_graphics_card", "fields.memory.value_suffix_color", "")
                       << config.getPrefix("detailed_graphics_card", "fields.memory.value_suffix", "") << r;
                    lp.push(ss.str());
                };

                fields["usage"] = [&]() {
                    if (!config.getNestedBool("detailed_graphics_card", "fields.usage.show", true)) return;
                    ostringstream ss;
                    ss << config.getNestedColor("detailed_graphics_card", "fields.usage.usage_prefix_color", "")
                       << config.getPrefix("detailed_graphics_card", "fields.usage.usage_prefix", "") << r
                       << config.getNestedColor("detailed_graphics_card", "fields.usage.label_color", "")
                       << config.getLabel("detailed_graphics_card", "fields.usage.label", "") << r
                       << config.getNestedColor("detailed_graphics_card", "fields.usage.label_suffix_color", "")
                       << config.getPrefix("detailed_graphics_card", "fields.usage.label_suffix", "") << r
                       << config.getNestedColor("detailed_graphics_card", "fields.usage.value_color", "")
                       << g.gpu_usage << r
                       << config.getNestedColor("detailed_graphics_card", "fields.usage.value_suffix_color", "")
                       << config.getPrefix("detailed_graphics_card", "fields.usage.value_suffix", "%") << r;
                    lp.push(ss.str());
                };

                fields["vendor"] = [&]() {
                    if (!config.getNestedBool("detailed_graphics_card", "fields.vendor.show", true)) return;
                    ostringstream ss;
                    ss << config.getNestedColor("detailed_graphics_card", "fields.vendor.vendor_prefix_color", "")
                       << config.getPrefix("detailed_graphics_card", "fields.vendor.vendor_prefix", "") << r
                       << config.getNestedColor("detailed_graphics_card", "fields.vendor.label_color", "")
                       << config.getLabel("detailed_graphics_card", "fields.vendor.label", "") << r
                       << config.getNestedColor("detailed_graphics_card", "fields.vendor.label_suffix_color", "")
                       << config.getPrefix("detailed_graphics_card", "fields.vendor.label_suffix", "") << r
                       << config.getNestedColor("detailed_graphics_card", "fields.vendor.value_color", "")
                       << g.gpu_vendor << r
                       << config.getNestedColor("detailed_graphics_card", "fields.vendor.value_suffix_color", "")
                       << config.getPrefix("detailed_graphics_card", "fields.vendor.value_suffix", "") << r;
                    lp.push(ss.str());
                };

                fields["driver"] = [&]() {
                    if (!config.getNestedBool("detailed_graphics_card", "fields.driver.show", true)) return;
                    ostringstream ss;
                    ss << config.getNestedColor("detailed_graphics_card", "fields.driver.driver_prefix_color", "")
                       << config.getPrefix("detailed_graphics_card", "fields.driver.driver_prefix", "") << r
                       << config.getNestedColor("detailed_graphics_card", "fields.driver.label_color", "")
                       << config.getLabel("detailed_graphics_card", "fields.driver.label", "") << r
                       << config.getNestedColor("detailed_graphics_card", "fields.driver.label_suffix_color", "")
                       << config.getPrefix("detailed_graphics_card", "fields.driver.label_suffix", "") << r
                       << config.getNestedColor("detailed_graphics_card", "fields.driver.value_color", "")
                       << g.gpu_driver_version << r
                       << config.getNestedColor("detailed_graphics_card", "fields.driver.value_suffix_color", "")
                       << config.getPrefix("detailed_graphics_card", "fields.driver.value_suffix", "") << r;
                    lp.push(ss.str());
                };

                fields["temperature"] = [&]() {
                    if (!config.getNestedBool("detailed_graphics_card", "fields.temperature.show", true)) return;
                    ostringstream ss;
                    ss << config.getNestedColor("detailed_graphics_card", "fields.temperature.temperature_prefix_color", "")
                       << config.getPrefix("detailed_graphics_card", "fields.temperature.temperature_prefix", "") << r
                       << config.getNestedColor("detailed_graphics_card", "fields.temperature.label_color", "")
                       << config.getLabel("detailed_graphics_card", "fields.temperature.label", "") << r
                       << config.getNestedColor("detailed_graphics_card", "fields.temperature.label_suffix_color", "")
                       << config.getPrefix("detailed_graphics_card", "fields.temperature.label_suffix", "") << r
                       << config.getNestedColor("detailed_graphics_card", "fields.temperature.value_color", "")
                       << g.gpu_temperature << r
                       << config.getNestedColor("detailed_graphics_card", "fields.temperature.value_suffix_color", "")
                       << config.getPrefix("detailed_graphics_card", "fields.temperature.value_suffix", " C") << r;
                    lp.push(ss.str());
                };

                fields["cores"] = [&]() {
                    if (!config.getNestedBool("detailed_graphics_card", "fields.cores.show", true)) return;
                    ostringstream ss;
                    ss << config.getNestedColor("detailed_graphics_card", "fields.cores.cores_prefix_color", "")
                       << config.getPrefix("detailed_graphics_card", "fields.cores.cores_prefix", "") << r
                       << config.getNestedColor("detailed_graphics_card", "fields.cores.label_color", "")
                       << config.getLabel("detailed_graphics_card", "fields.cores.label", "") << r
                       << config.getNestedColor("detailed_graphics_card", "fields.cores.label_suffix_color", "")
                       << config.getPrefix("detailed_graphics_card", "fields.cores.label_suffix", "") << r
                       << config.getNestedColor("detailed_graphics_card", "fields.cores.value_color", "")
                       << g.gpu_core_count << r
                       << config.getNestedColor("detailed_graphics_card", "fields.cores.value_suffix_color", "")
                       << config.getPrefix("detailed_graphics_card", "fields.cores.value_suffix", "") << r;
                    lp.push(ss.str());
                };

                // ---- Run fields in the order JSON specifies ----
                static const std::vector<std::string> defaultGpuFieldOrder =
                    {"name", "memory", "usage", "vendor", "driver", "temperature", "cores"};
                auto gpuFieldOrder = config.getStringArray("detailed_graphics_card", "order", defaultGpuFieldOrder);

                for (const auto& key : gpuFieldOrder) {
                    auto it = fields.find(key);
                    if (it != fields.end()) it->second();
                }
            }
        }

        //   PRIMARY GPU DETAILS  (toggle via "show_primary_gpu")
        //   true  -> print Primary GPU Details (original behaviour)
        //   false -> skip the primary block entirely
        if (config.getNestedBool("detailed_graphics_card", "show_primary_gpu", true)) {

            auto primary = detailed_gpu_info.primary_gpu_info();

            if (config.getNestedBool("detailed_graphics_card", "primary_header.show", true)) {

                ostringstream ss;
                ss << config.getNestedColor("detailed_graphics_card", "primary_header.prefix_color", "")
                   << config.getPrefix("detailed_graphics_card", "primary_header.prefix", "") << r
                   << config.getNestedColor("detailed_graphics_card", "primary_header.text_color", "")
                   << config.getLabel("detailed_graphics_card", "primary_header.text", "") << r
                   << config.getNestedColor("detailed_graphics_card", "primary_header.suffix_color", "")
                   << config.getPrefix("detailed_graphics_card", "primary_header.suffix", "") << r;
                lp.push(ss.str());

                // ---- Register each orderable primary field as a named lambda ----
                std::map<std::string, std::function<void()>> primaryFields;

                primaryFields["name"] = [&]() {
                    if (!config.getNestedBool("detailed_graphics_card", "primary_fields.name.show", true)) return;
                    ostringstream ss;
                    ss << config.getNestedColor("detailed_graphics_card", "primary_fields.name.name_prefix_color", "")
                       << config.getPrefix("detailed_graphics_card", "primary_fields.name.name_prefix", "") << r
                       << config.getNestedColor("detailed_graphics_card", "primary_fields.name.label_color", "")
                       << config.getLabel("detailed_graphics_card", "primary_fields.name.label", "") << r
                       << config.getNestedColor("detailed_graphics_card", "primary_fields.name.label_suffix_color", "")
                       << config.getPrefix("detailed_graphics_card", "primary_fields.name.label_suffix", "") << r
                       << config.getNestedColor("detailed_graphics_card", "primary_fields.name.value_color", "")
                       << primary.name << r
                       << config.getNestedColor("detailed_graphics_card", "primary_fields.name.value_suffix_color", "")
                       << config.getPrefix("detailed_graphics_card", "primary_fields.name.value_suffix", "") << r;
                    lp.push(ss.str());
                };

                primaryFields["vram"] = [&]() {
                    if (!config.getNestedBool("detailed_graphics_card", "primary_fields.vram.show", true)) return;
                    ostringstream ss;
                    ss << config.getNestedColor("detailed_graphics_card", "primary_fields.vram.vram_prefix_color", "")
                       << config.getPrefix("detailed_graphics_card", "primary_fields.vram.vram_prefix", "") << r
                       << config.getNestedColor("detailed_graphics_card", "primary_fields.vram.label_color", "")
                       << config.getLabel("detailed_graphics_card", "primary_fields.vram.label", "") << r
                       << config.getNestedColor("detailed_graphics_card", "primary_fields.vram.label_suffix_color", "")
                       << config.getPrefix("detailed_graphics_card", "primary_fields.vram.label_suffix", "") << r
                       << config.getNestedColor("detailed_graphics_card", "primary_fields.vram.value_color", "")
                       << primary.vram_gb << r
                       << config.getNestedColor("detailed_graphics_card", "primary_fields.vram.value_suffix_color", "")
                       << config.getPrefix("detailed_graphics_card", "primary_fields.vram.value_suffix", " GiB") << r;
                    lp.push(ss.str());
                };

                primaryFields["freq"] = [&]() {
                    if (!config.getNestedBool("detailed_graphics_card", "primary_fields.freq.show", true)) return;
                    ostringstream ss;
                    ss << config.getNestedColor("detailed_graphics_card", "primary_fields.freq.freq_prefix_color", "")
                       << config.getPrefix("detailed_graphics_card", "primary_fields.freq.freq_prefix", "") << r
                       << config.getNestedColor("detailed_graphics_card", "primary_fields.freq.label_color", "")
                       << config.getLabel("detailed_graphics_card", "primary_fields.freq.label", "") << r
                       << config.getNestedColor("detailed_graphics_card", "primary_fields.freq.label_suffix_color", "")
                       << config.getPrefix("detailed_graphics_card", "primary_fields.freq.label_suffix", "") << r
                       << config.getNestedColor("detailed_graphics_card", "primary_fields.freq.value_color", "")
                       << primary.frequency_ghz << r
                       << config.getNestedColor("detailed_graphics_card", "primary_fields.freq.value_suffix_color", "")
                       << config.getPrefix("detailed_graphics_card", "primary_fields.freq.value_suffix", " GHz") << r;
                    lp.push(ss.str());
                };

                // ---- Run primary fields in the order JSON specifies ----
                static const std::vector<std::string> defaultPrimaryOrder =
                    {"name", "vram", "freq"};
                auto primaryOrder = config.getStringArray("detailed_graphics_card", "primary_order", defaultPrimaryOrder);

                for (const auto& key : primaryOrder) {
                    auto it = primaryFields.find(key);
                    if (it != primaryFields.end()) it->second();
                }
            }
        }
    }
};


//   ██████╗ ██╗███████╗██████╗ ██╗      █████╗ ██╗   ██╗
//   ██╔══██╗██║██╔════╝██╔══██╗██║     ██╔══██╗╚██╗ ██╔╝
//   ██║  ██║██║███████╗██████╔╝██║     ███████║ ╚████╔╝ 
//   ██║  ██║██║     ██ ██╗     ██║     ██╔══██║  ╚██╔╝  
//   ██████╔╝██║███████╗██║     ███████╗██║  ██║   ██║   
//   ╚═════╝ ╚═╝╚══════╝╚═╝     ╚══════╝╚═╝  ╚═╝   ╚═╝   
//                      D E T A I L E D   D I S P L A Y
//  Displays comprehensive monitor information:
//  • Display Banner      - Index number with formatted header
//  • Display Name        - Manufacturer and model
//  • Applied Resolution  - Current resolution @ refresh rate
//  • Native Resolution   - Maximum native resolution
//  • Aspect Ratio        - Width-to-height ratio
//  • Scaling             - DPI scaling percentage
//  • Upscale             - Upscaling multiplier
//  • DSR / VSR           - Dynamic Super Resolution status

sections["detailed_display_monitor"] = [&]() {
    if (!config.isEnabled("detailed_display_monitor")) return;
    
    // line spacing json driven
    int spacing = config.getNestedInt("detailed_display_monitor","top_line_spacing",0);
    for (int n = 0; n < spacing; n++) {lp.push("");}

    const auto& screens = di.getScreens();

    for (size_t i = 0; i < screens.size(); ++i) {
        const auto& s = screens[i];

        // ---------- Display Banner (per-display header, stays outside the order) ----------
        if (config.getNestedBool("detailed_display_monitor", "banner.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("detailed_display_monitor", "banner.prefix_color", "")
               << config.getPrefix("detailed_display_monitor", "banner.prefix", "") << r
               << config.getNestedColor("detailed_display_monitor", "banner.text_color", "")
               << config.getLabel("detailed_display_monitor", "banner.text", "")
               << config.getNestedColor("detailed_display_monitor", "banner.index_color", "")
               << (i + 1) << " " << r
               << config.getNestedColor("detailed_display_monitor", "banner.suffix_color", "")
               << config.getPrefix("detailed_display_monitor", "banner.suffix", "") << r;
            lp.push(ss.str());
        }

        // ---- Register each orderable field as a named lambda ----
        std::map<std::string, std::function<void()>> fields;

        // ---------- Name ----------
        fields["name"] = [&]() {
            if (!config.getNestedBool("detailed_display_monitor", "fields.name.show", true)) return;
            ostringstream ss;
            ss << config.getNestedColor("detailed_display_monitor", "fields.name.name_prefix_color", "")
               << config.getPrefix("detailed_display_monitor", "fields.name.name_prefix", "") << r
               << config.getNestedColor("detailed_display_monitor", "fields.name.label_color", "")
               << config.getLabel("detailed_display_monitor", "fields.name.label", "") << r
               << config.getNestedColor("detailed_display_monitor", "fields.name.label_suffix_color", "")
               << config.getLabel("detailed_display_monitor", "fields.name.label_suffix", "") << r
               << config.getNestedColor("detailed_display_monitor", "fields.name.value_color", "")
               << s.name << r
               << config.getNestedColor("detailed_display_monitor", "fields.name.value_suffix_color", "")
               << config.getLabel("detailed_display_monitor", "fields.name.value_suffix", "") << r;
            lp.push(ss.str());
        };

        // ---------- Applied Resolution ----------
        fields["applied_resolution"] = [&]() {
            if (!config.getNestedBool("detailed_display_monitor", "fields.applied_resolution.show", true)) return;
            ostringstream ss;
            ss << config.getNestedColor("detailed_display_monitor", "fields.applied_resolution.applied_resolution_prefix_color", "")
               << config.getPrefix("detailed_display_monitor", "fields.applied_resolution.applied_resolution_prefix", "") << r
               << config.getNestedColor("detailed_display_monitor", "fields.applied_resolution.label_color", "")
               << config.getLabel("detailed_display_monitor", "fields.applied_resolution.label", "") << r
               << config.getNestedColor("detailed_display_monitor", "fields.applied_resolution.label_suffix_color", "")
               << config.getLabel("detailed_display_monitor", "fields.applied_resolution.label_suffix", "") << r
               << config.getNestedColor("detailed_display_monitor", "fields.applied_resolution.value_color", "")
               << s.current_width
               << config.getNestedColor("detailed_display_monitor", "fields.applied_resolution.x_color", "")
               << config.getLabel("detailed_display_monitor", "fields.applied_resolution.x_symbol", "x") << r
               << config.getNestedColor("detailed_display_monitor", "fields.applied_resolution.value_color", "")
               << s.current_height
               << config.getNestedColor("detailed_display_monitor", "fields.applied_resolution.at_symbol_color", "")
               << config.getLabel("detailed_display_monitor", "fields.applied_resolution.at_symbol", " @") << r
               << config.getNestedColor("detailed_display_monitor", "fields.applied_resolution.value_color", "")
               << s.refresh_rate
               << config.getNestedColor("detailed_display_monitor", "fields.applied_resolution.hz_color", "")
               << config.getLabel("detailed_display_monitor", "fields.applied_resolution.hz_symbol", "Hz") << r
               << config.getNestedColor("detailed_display_monitor", "fields.applied_resolution.value_suffix_color", "")
               << config.getLabel("detailed_display_monitor", "fields.applied_resolution.value_suffix", "") << r;
            lp.push(ss.str());
        };

        // ---------- Native Resolution ----------
        fields["native_resolution"] = [&]() {
            if (!config.getNestedBool("detailed_display_monitor", "fields.native_resolution.show", true)) return;
            ostringstream ss;
            ss << config.getNestedColor("detailed_display_monitor", "fields.native_resolution.native_resolution_prefix_color", "")
               << config.getPrefix("detailed_display_monitor", "fields.native_resolution.native_resolution_prefix", "") << r
               << config.getNestedColor("detailed_display_monitor", "fields.native_resolution.label_color", "")
               << config.getLabel("detailed_display_monitor", "fields.native_resolution.label", "") << r
               << config.getNestedColor("detailed_display_monitor", "fields.native_resolution.label_suffix_color", "")
               << config.getLabel("detailed_display_monitor", "fields.native_resolution.label_suffix", "") << r
               << config.getNestedColor("detailed_display_monitor", "fields.native_resolution.value_color", "")
               << s.native_resolution << r
               << config.getNestedColor("detailed_display_monitor", "fields.native_resolution.value_suffix_color", "")
               << config.getLabel("detailed_display_monitor", "fields.native_resolution.value_suffix", "") << r;
            lp.push(ss.str());
        };

        // ---------- Aspect Ratio ----------
        fields["aspect_ratio"] = [&]() {
            if (!config.getNestedBool("detailed_display_monitor", "fields.aspect_ratio.show", true)) return;
            ostringstream ss;
            ss << config.getNestedColor("detailed_display_monitor", "fields.aspect_ratio.aspect_ratio_prefix_color", "")
               << config.getPrefix("detailed_display_monitor", "fields.aspect_ratio.aspect_ratio_prefix", "") << r
               << config.getNestedColor("detailed_display_monitor", "fields.aspect_ratio.label_color", "")
               << config.getLabel("detailed_display_monitor", "fields.aspect_ratio.label", "") << r
               << config.getNestedColor("detailed_display_monitor", "fields.aspect_ratio.label_suffix_color", "")
               << config.getLabel("detailed_display_monitor", "fields.aspect_ratio.label_suffix", "") << r
               << config.getNestedColor("detailed_display_monitor", "fields.aspect_ratio.value_color", "")
               << s.aspect_ratio << r
               << config.getNestedColor("detailed_display_monitor", "fields.aspect_ratio.value_suffix_color", "")
               << config.getLabel("detailed_display_monitor", "fields.aspect_ratio.value_suffix", "") << r;
            lp.push(ss.str());
        };

        // ---------- Scaling ----------
        fields["scaling"] = [&]() {
            if (!config.getNestedBool("detailed_display_monitor", "fields.scaling.show", true)) return;
            ostringstream ss;
            ss << config.getNestedColor("detailed_display_monitor", "fields.scaling.scaling_prefix_color", "")
               << config.getPrefix("detailed_display_monitor", "fields.scaling.scaling_prefix", "") << r
               << config.getNestedColor("detailed_display_monitor", "fields.scaling.label_color", "")
               << config.getLabel("detailed_display_monitor", "fields.scaling.label", "") << r
               << config.getNestedColor("detailed_display_monitor", "fields.scaling.label_suffix_color", "")
               << config.getLabel("detailed_display_monitor", "fields.scaling.label_suffix", "") << r
               << config.getNestedColor("detailed_display_monitor", "fields.scaling.value_color", "")
               << s.scale_percent
               << config.getNestedColor("detailed_display_monitor", "fields.scaling.percent_color", "")
               << config.getLabel("detailed_display_monitor", "fields.scaling.percent_symbol", "%") << r
               << config.getNestedColor("detailed_display_monitor", "fields.scaling.value_suffix_color", "")
               << config.getLabel("detailed_display_monitor", "fields.scaling.value_suffix", "") << r;
            lp.push(ss.str());
        };

        // ---------- Upscale ----------
        fields["upscale"] = [&]() {
            if (!config.getNestedBool("detailed_display_monitor", "fields.upscale.show", true)) return;
            ostringstream ss;
            ss << config.getNestedColor("detailed_display_monitor", "fields.upscale.upscale_prefix_color", "")
               << config.getPrefix("detailed_display_monitor", "fields.upscale.upscale_prefix", "") << r
               << config.getNestedColor("detailed_display_monitor", "fields.upscale.label_color", "")
               << config.getLabel("detailed_display_monitor", "fields.upscale.label", "") << r
               << config.getNestedColor("detailed_display_monitor", "fields.upscale.label_suffix_color", "")
               << config.getLabel("detailed_display_monitor", "fields.upscale.label_suffix", "") << r
               << config.getNestedColor("detailed_display_monitor", "fields.upscale.value_color", "")
               << s.upscale << r
               << config.getNestedColor("detailed_display_monitor", "fields.upscale.value_suffix_color", "")
               << config.getLabel("detailed_display_monitor", "fields.upscale.value_suffix", "") << r;
            lp.push(ss.str());
        };

        // ---------- DSR / VSR ----------
        fields["dsr"] = [&]() {
            if (!config.getNestedBool("detailed_display_monitor", "fields.dsr.show", true)) return;
            ostringstream ss;
            ss << config.getNestedColor("detailed_display_monitor", "fields.dsr.dsr_prefix_color", "")
               << config.getPrefix("detailed_display_monitor", "fields.dsr.dsr_prefix", "") << r
               << config.getNestedColor("detailed_display_monitor", "fields.dsr.label_color", "")
               << config.getLabel("detailed_display_monitor", "fields.dsr.label", "") << r
               << config.getNestedColor("detailed_display_monitor", "fields.dsr.label_suffix_color", "")
               << config.getLabel("detailed_display_monitor", "fields.dsr.label_suffix", "") << r
               << config.getNestedColor(
                    "detailed_display_monitor",
                    s.dsr_enabled ? "fields.dsr.enabled_color" : "fields.dsr.disabled_color",
                    ""
                  )
               << config.getLabel(
                    "detailed_display_monitor",
                    s.dsr_enabled ? "fields.dsr.enabled_text" : "fields.dsr.disabled_text",
                    s.dsr_enabled ? "Enabled" : "Disabled"
                  ) << r
               << config.getNestedColor("detailed_display_monitor", "fields.dsr.brackets_color", "")
               << config.getLabel("detailed_display_monitor", "fields.dsr.bracket_open", " (") << r
               << config.getNestedColor("detailed_display_monitor", "fields.dsr.type_color", "")
               << s.dsr_type
               << config.getNestedColor("detailed_display_monitor", "fields.dsr.brackets_color", "")
               << config.getLabel("detailed_display_monitor", "fields.dsr.bracket_close", ")") << r
               << config.getNestedColor("detailed_display_monitor", "fields.dsr.value_suffix_color", "")
               << config.getLabel("detailed_display_monitor", "fields.dsr.value_suffix", "") << r;
            lp.push(ss.str());
        };

        // ---- Run fields in the order JSON specifies ----
        // Each field prints its own line, so this only controls sequence,
        // not inter-field spacing (matching the pattern used by
        // detailed_processor, detailed_operating_system, etc.).
        static const std::vector<std::string> defaultOrder =
            {"name", "applied_resolution", "native_resolution", "aspect_ratio", "scaling", "upscale", "dsr"};
        auto order = config.getStringArray("detailed_display_monitor", "order", defaultOrder);

        for (const auto& key : order) {
            auto it = fields.find(key);
            if (it != fields.end()) it->second();
        }
    }
};


//  ██████╗ ██╗ ██████╗ ███████╗    ██╗███╗   ██╗███████╗ ██████╗ 
//  ██╔══██╗██║██╔═══██╗██╔════╝    ██║████╗  ██║██╔════╝██╔═══██╗
//  ██████╔╝██║██║   ██║███████╗    ██║██╔██╗ ██║█████╗  ██║   ██║
//  ██╔══██╗██║██║   ██║╚════██║    ██║██║╚██╗██║██╔══╝  ██║   ██║
//  ██████╔╝██║╚██████╔╝███████║    ██║██║ ╚████║██║     ╚██████╔╝
//  ╚═════╝ ╚═╝ ╚═════╝ ╚══════╝    ╚═╝╚═╝  ╚═══╝╚═╝      ╚═════╝ 
//              D E T A I L E D   B I O S   &   M O T H E R B O A R D
//  This section displays comprehensive BIOS and motherboard information:
//  1. Bios Vendor           - Manufacturer of the system BIOS/UEFI
//  2. Bios Version          - Installed BIOS/UEFI version string
//  3. Bios Date             - Release date of the installed BIOS
//  4. Motherboard Model     - Model identifier of the motherboard
//  5. Motherboard Manufacturer - Motherboard vendor/brand
//  All labels, values, colors, prefixes, and toggles are fully JSON-driven
//  via the "detailed_bios_and_motherboard" config block (aliased as "detailed_bios_and_motherboard").
//
//  Output Example:
//  #- BIOS & Motherboard Info ----------------------------------------#
//  ~ Bios Vendor             : American Megatrends Inc.
//  ~ Bios Version            : F5
//  ~ Bios Date               : 2024-03-12
//  ~ Motherboard Model       : ROG STRIX B650E-F
//  ~ Motherboard Manufacturer: ASUSTeK COMPUTER INC.
sections["detailed_bios_and_motherboard"] = [&]() {
    if (!config.isEnabled("detailed_bios_and_motherboard")) return;

    // line spacing json driven
    int spacing = config.getNestedInt("detailed_bios_and_motherboard","top_line_spacing",0);
    for (int n = 0; n < spacing; n++) {lp.push("");}

    // ---------- HEADER (stays outside the order) ----------
    if (config.getNestedBool("detailed_bios_and_motherboard", "header.show", true)) {
        ostringstream ss;
        ss << config.getNestedColor("detailed_bios_and_motherboard", "header.prefix_color", "")
           << config.getPrefix("detailed_bios_and_motherboard", "header.prefix", "") << r
           << config.getNestedColor("detailed_bios_and_motherboard", "header.text_color", "")
           << config.getLabel("detailed_bios_and_motherboard", "header.text", "") << r
           << config.getNestedColor("detailed_bios_and_motherboard", "header.suffix_color", "")
           << config.getPrefix("detailed_bios_and_motherboard", "header.suffix", "") << r;
        lp.push(ss.str());
    }

    // ---- Register each orderable field as a named lambda ----
    std::map<std::string, std::function<void()>> fields;

    // ---------- BIOS VENDOR ----------
    fields["bios_vendor"] = [&]() {
        if (!config.getNestedBool("detailed_bios_and_motherboard", "fields.bios_vendor.show", true)) return;
        ostringstream ss;
        ss << config.getNestedColor("detailed_bios_and_motherboard", "fields.bios_vendor.bios_vendor_prefix_color", "")
           << config.getPrefix("detailed_bios_and_motherboard", "fields.bios_vendor.bios_vendor_prefix", "") << r
           << config.getNestedColor("detailed_bios_and_motherboard", "fields.bios_vendor.label_color", "")
           << config.getLabel("detailed_bios_and_motherboard", "fields.bios_vendor.label", "") << r
           << config.getNestedColor("detailed_bios_and_motherboard", "fields.bios_vendor.label_suffix_color", "")
           << config.getLabel("detailed_bios_and_motherboard", "fields.bios_vendor.label_suffix", "") << r
           << config.getNestedColor("detailed_bios_and_motherboard", "fields.bios_vendor.value_color", "")
           << sys.get_bios_vendor() << r
           << config.getNestedColor("detailed_bios_and_motherboard", "fields.bios_vendor.value_suffix_color", "")
           << config.getLabel("detailed_bios_and_motherboard", "fields.bios_vendor.value_suffix", "") << r;
        lp.push(ss.str());
    };

    // ---------- BIOS VERSION ----------
    fields["bios_version"] = [&]() {
        if (!config.getNestedBool("detailed_bios_and_motherboard", "fields.bios_version.show", true)) return;
        ostringstream ss;
        ss << config.getNestedColor("detailed_bios_and_motherboard", "fields.bios_version.bios_version_prefix_color", "")
           << config.getPrefix("detailed_bios_and_motherboard", "fields.bios_version.bios_version_prefix", "") << r
           << config.getNestedColor("detailed_bios_and_motherboard", "fields.bios_version.label_color", "")
           << config.getLabel("detailed_bios_and_motherboard", "fields.bios_version.label", "") << r
           << config.getNestedColor("detailed_bios_and_motherboard", "fields.bios_version.label_suffix_color", "")
           << config.getLabel("detailed_bios_and_motherboard", "fields.bios_version.label_suffix", "") << r
           << config.getNestedColor("detailed_bios_and_motherboard", "fields.bios_version.value_color", "")
           << sys.get_bios_version() << r
           << config.getNestedColor("detailed_bios_and_motherboard", "fields.bios_version.value_suffix_color", "")
           << config.getLabel("detailed_bios_and_motherboard", "fields.bios_version.value_suffix", "") << r;
        lp.push(ss.str());
    };

    // ---------- BIOS DATE ----------
    fields["bios_date"] = [&]() {
        if (!config.getNestedBool("detailed_bios_and_motherboard", "fields.bios_date.show", true)) return;
        ostringstream ss;
        ss << config.getNestedColor("detailed_bios_and_motherboard", "fields.bios_date.bios_date_prefix_color", "")
           << config.getPrefix("detailed_bios_and_motherboard", "fields.bios_date.bios_date_prefix", "") << r
           << config.getNestedColor("detailed_bios_and_motherboard", "fields.bios_date.label_color", "")
           << config.getLabel("detailed_bios_and_motherboard", "fields.bios_date.label", "") << r
           << config.getNestedColor("detailed_bios_and_motherboard", "fields.bios_date.label_suffix_color", "")
           << config.getLabel("detailed_bios_and_motherboard", "fields.bios_date.label_suffix", "") << r
           << config.getNestedColor("detailed_bios_and_motherboard", "fields.bios_date.value_color", "")
           << sys.get_bios_date() << r
           << config.getNestedColor("detailed_bios_and_motherboard", "fields.bios_date.value_suffix_color", "")
           << config.getLabel("detailed_bios_and_motherboard", "fields.bios_date.value_suffix", "") << r;
        lp.push(ss.str());
    };

    // ---------- MOTHERBOARD MODEL ----------
    fields["mb_model"] = [&]() {
        if (!config.getNestedBool("detailed_bios_and_motherboard", "fields.mb_model.show", true)) return;
        ostringstream ss;
        ss << config.getNestedColor("detailed_bios_and_motherboard", "fields.mb_model.mb_model_prefix_color", "")
           << config.getPrefix("detailed_bios_and_motherboard", "fields.mb_model.mb_model_prefix", "") << r
           << config.getNestedColor("detailed_bios_and_motherboard", "fields.mb_model.label_color", "")
           << config.getLabel("detailed_bios_and_motherboard", "fields.mb_model.label", "") << r
           << config.getNestedColor("detailed_bios_and_motherboard", "fields.mb_model.label_suffix_color", "")
           << config.getLabel("detailed_bios_and_motherboard", "fields.mb_model.label_suffix", "") << r
           << config.getNestedColor("detailed_bios_and_motherboard", "fields.mb_model.value_color", "")
           << sys.get_motherboard_model() << r
           << config.getNestedColor("detailed_bios_and_motherboard", "fields.mb_model.value_suffix_color", "")
           << config.getLabel("detailed_bios_and_motherboard", "fields.mb_model.value_suffix", "") << r;
        lp.push(ss.str());
    };

    // ---------- MOTHERBOARD MANUFACTURER ----------
    fields["mb_manufacturer"] = [&]() {
        if (!config.getNestedBool("detailed_bios_and_motherboard", "fields.mb_manufacturer.show", true)) return;
        ostringstream ss;
        ss << config.getNestedColor("detailed_bios_and_motherboard", "fields.mb_manufacturer.mb_manufacturer_prefix_color", "")
           << config.getPrefix("detailed_bios_and_motherboard", "fields.mb_manufacturer.mb_manufacturer_prefix", "") << r
           << config.getNestedColor("detailed_bios_and_motherboard", "fields.mb_manufacturer.label_color", "")
           << config.getLabel("detailed_bios_and_motherboard", "fields.mb_manufacturer.label", "") << r
           << config.getNestedColor("detailed_bios_and_motherboard", "fields.mb_manufacturer.label_suffix_color", "")
           << config.getLabel("detailed_bios_and_motherboard", "fields.mb_manufacturer.label_suffix", "") << r
           << config.getNestedColor("detailed_bios_and_motherboard", "fields.mb_manufacturer.value_color", "")
           << sys.get_motherboard_manufacturer() << r
           << config.getNestedColor("detailed_bios_and_motherboard", "fields.mb_manufacturer.value_suffix_color", "")
           << config.getLabel("detailed_bios_and_motherboard", "fields.mb_manufacturer.value_suffix", "") << r;
        lp.push(ss.str());
    };

    // ---- Run fields in the order JSON specifies ----
    // Each field prints its own line via lp.push(...), so this only
    // controls sequence — matching the pattern used by
    // detailed_processor, detailed_operating_system, etc.
    static const std::vector<std::string> defaultOrder =
        {"bios_vendor", "bios_version", "bios_date", "mb_model", "mb_manufacturer"};
    auto order = config.getStringArray("detailed_bios_and_motherboard", "order", defaultOrder);

    for (const auto& key : order) {
        auto it = fields.find(key);
        if (it != fields.end()) it->second();
    }
};

//  ██╗   ██╗███████╗███████╗██████╗     ██╗███╗   ██╗███████╗ ██████╗ 
//  ██║   ██║██╔════╝██╔════╝██╔══██╗    ██║████╗  ██║██╔════╝██╔═══██╗
//  ██║   ██║███████╗█████╗  ██████╔╝    ██║██╔██╗ ██║█████╗  ██║   ██║
//  ██║   ██║╚════██║██╔══╝  ██╔══██╗    ██║██║╚██╗██║██╔══╝  ██║   ██║
//  ╚██████╔╝███████║███████╗██║  ██║    ██║██║ ╚████║██║     ╚██████╔╝
//   ╚═════╝ ╚══════╝╚══════╝╚═╝  ╚═╝    ╚═╝╚═╝  ╚═══╝╚═╝      ╚═════╝ 
//                      D E T A I L E D   U S E R   A C C O U N T
//  This section displays comprehensive user account information:
//  1. Username           - The currently logged-in user's account name
//  2. Computer Name      - The hostname of the machine
//  3. Domain             - The Windows domain or workgroup the PC belongs to
//  All labels, values, colors, prefixes, and toggles are fully JSON-driven
//  via the "detailed_user_account" config block (aliased as "detailed_user_account").
//
//  Output Example:
//  #- User Info ------------------------------------------------------#
//  ~ Username              : JohnDoe
//  ~ Computer Name         : DESKTOP-4X9K2P1
//  ~ Domain                : WORKGROUP

sections["detailed_user_account"] = [&]() {
    if (!config.isEnabled("detailed_user_account")) return;
    
    // line spacing json driven
    int spacing = config.getNestedInt("detailed_user_account","top_line_spacing",0);
    for (int n = 0; n < spacing; n++) {lp.push("");}

    // ---------- HEADER (stays outside the order) ----------
    if (config.getNestedBool("detailed_user_account", "header.show", true)) {
        ostringstream ss;
        ss << config.getNestedColor("detailed_user_account", "header.prefix_color", "")
           << config.getPrefix("detailed_user_account", "header.prefix", "") << r
           << config.getNestedColor("detailed_user_account", "header.text_color", "")
           << config.getLabel("detailed_user_account", "header.text", "") << r
           << config.getNestedColor("detailed_user_account", "header.suffix_color", "")
           << config.getPrefix("detailed_user_account", "header.suffix", "") << r;
        lp.push(ss.str());
    }

    // ---- Register each orderable field as a named lambda ----
    std::map<std::string, std::function<void()>> fields;

    // ---------- USERNAME ----------
    fields["username"] = [&]() {
        if (!config.getNestedBool("detailed_user_account", "fields.username.show", true)) return;
        ostringstream ss;
        ss << config.getNestedColor("detailed_user_account", "fields.username.username_prefix_color", "")
           << config.getPrefix("detailed_user_account", "fields.username.username_prefix", "") << r
           << config.getNestedColor("detailed_user_account", "fields.username.label_color", "")
           << config.getLabel("detailed_user_account", "fields.username.label", "") << r
           << config.getNestedColor("detailed_user_account", "fields.username.label_suffix_color", "")
           << config.getLabel("detailed_user_account", "fields.username.label_suffix", "") << r
           << config.getNestedColor("detailed_user_account", "fields.username.value_color", "")
           << user.get_username() << r
           << config.getNestedColor("detailed_user_account", "fields.username.value_suffix_color", "")
           << config.getLabel("detailed_user_account", "fields.username.value_suffix", "") << r;
        lp.push(ss.str());
    };

    // ---------- COMPUTER NAME ----------
    fields["computer_name"] = [&]() {
        if (!config.getNestedBool("detailed_user_account", "fields.computer_name.show", true)) return;
        ostringstream ss;
        ss << config.getNestedColor("detailed_user_account", "fields.computer_name.computer_name_prefix_color", "")
           << config.getPrefix("detailed_user_account", "fields.computer_name.computer_name_prefix", "") << r
           << config.getNestedColor("detailed_user_account", "fields.computer_name.label_color", "")
           << config.getLabel("detailed_user_account", "fields.computer_name.label", "") << r
           << config.getNestedColor("detailed_user_account", "fields.computer_name.label_suffix_color", "")
           << config.getLabel("detailed_user_account", "fields.computer_name.label_suffix", "") << r
           << config.getNestedColor("detailed_user_account", "fields.computer_name.value_color", "")
           << user.get_computer_name() << r
           << config.getNestedColor("detailed_user_account", "fields.computer_name.value_suffix_color", "")
           << config.getLabel("detailed_user_account", "fields.computer_name.value_suffix", "") << r;
        lp.push(ss.str());
    };

    // ---------- DOMAIN ----------
    fields["domain"] = [&]() {
        if (!config.getNestedBool("detailed_user_account", "fields.domain.show", true)) return;
        ostringstream ss;
        ss << config.getNestedColor("detailed_user_account", "fields.domain.domain_prefix_color", "")
           << config.getPrefix("detailed_user_account", "fields.domain.domain_prefix", "") << r
           << config.getNestedColor("detailed_user_account", "fields.domain.label_color", "")
           << config.getLabel("detailed_user_account", "fields.domain.label", "") << r
           << config.getNestedColor("detailed_user_account", "fields.domain.label_suffix_color", "")
           << config.getLabel("detailed_user_account", "fields.domain.label_suffix", "") << r
           << config.getNestedColor("detailed_user_account", "fields.domain.value_color", "")
           << user.get_domain_name() << r
           << config.getNestedColor("detailed_user_account", "fields.domain.value_suffix_color", "")
           << config.getLabel("detailed_user_account", "fields.domain.value_suffix", "") << r;
        lp.push(ss.str());
    };

    // ---- Run fields in the order JSON specifies ----
    // Each field prints its own line via lp.push(...), so this only
    // controls sequence — matching the pattern used by
    // detailed_processor, detailed_operating_system, etc.
    static const std::vector<std::string> defaultOrder =
        {"username", "computer_name", "domain"};
    auto order = config.getStringArray("detailed_user_account", "order", defaultOrder);

    for (const auto& key : order) {
        auto it = fields.find(key);
        if (it != fields.end()) it->second();
    }
};

//  ██████╗ ███████╗██████╗ ███████╗ ██████╗ ██████╗ ███╗   ███╗ █████╗ ███╗   ██╗ ██████╗███████╗
//  ██╔══██╗██╔════╝██╔══██╗██╔════╝██╔═══██╗██╔══██╗████╗ ████║██╔══██╗████╗  ██║██╔════╝██╔════╝
//  ██████╔╝█████╗  ██████╔╝█████╗  ██║   ██║██████╔╝██╔████╔██║███████║██╔██╗ ██║██║     █████╗  
//  ██╔═══╝ ██╔══╝  ██╔══██╗██╔══╝  ██║   ██║██╔══██╗██║╚██╔╝██║██╔══██║██║╚██╗██║██║     ██╔══╝  
//  ██║     ███████╗██║  ██║██║     ╚██████╔╝██║  ██║██║ ╚═╝ ██║██║  ██║██║ ╚████║╚██████╗███████╗
//  ╚═╝     ╚══════╝╚═╝  ╚═╝╚═╝      ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝ ╚═════╝╚══════╝
//                       D E T A I L E D   P E R F O R M A N C E
//  This section displays real-time system performance metrics:
//  1. System Uptime      - Time elapsed since the last system boot
//  2. CPU Usage          - Current processor utilization percentage
//  3. RAM Usage           - Current memory utilization percentage
//  4. Disk Usage          - Current storage utilization percentage
//  5. GPU Usage           - Current graphics card utilization percentage
//  All labels, values, colors, prefixes, and toggles are fully JSON-driven
//  via the "detailed_resource_usage" config block (aliased as "detailed_resource_usage").
//
//  Output Example:
//  #- Performance Info -----------------------------------------------#
//  ~ System Uptime          : 3d 4h 12m
//  ~ CPU Usage              : 12%
//  ~ RAM Usage              : 47%
//  ~ Disk Usage             : 68%
//  ~ GPU Usage              : 8%
// Performance Info (JSON Driven)
sections["detailed_resource_usage"] = [&]() {
    if (!config.isEnabled("detailed_resource_usage")) return;
    
    // line spacing json driven
    int spacing = config.getNestedInt("detailed_resource_usage","top_line_spacing",0);
    for (int n = 0; n < spacing; n++) {lp.push("");}

    // ---------- HEADER (stays outside the order) ----------
    if (config.getNestedBool("detailed_resource_usage", "header.show", true)) {
        ostringstream ss;
        ss << config.getNestedColor("detailed_resource_usage", "header.prefix_color", "")
           << config.getLabel("detailed_resource_usage", "header.prefix", "") << r
           << config.getNestedColor("detailed_resource_usage", "header.text_color", "")
           << config.getLabel("detailed_resource_usage", "header.text", "") << r
           << config.getNestedColor("detailed_resource_usage", "header.suffix_color", "")
           << config.getLabel("detailed_resource_usage", "header.suffix", "") << r;
        lp.push(ss.str());
    }

    // ---- Register each orderable field as a named lambda ----
    std::map<std::string, std::function<void()>> fields;

    // ---------- SYSTEM UPTIME ----------
    fields["uptime"] = [&]() {
        if (!config.getNestedBool("detailed_resource_usage", "fields.uptime.show", true)) return;
        ostringstream ss;
        ss << config.getNestedColor("detailed_resource_usage", "fields.uptime.uptime_prefix_color", "")
           << config.getLabel("detailed_resource_usage", "fields.uptime.uptime_prefix", "") << r
           << config.getNestedColor("detailed_resource_usage", "fields.uptime.label_color", "")
           << config.getLabel("detailed_resource_usage", "fields.uptime.label", "") << r
           << config.getNestedColor("detailed_resource_usage", "fields.uptime.label_suffix_color", "")
           << config.getLabel("detailed_resource_usage", "fields.uptime.label_suffix", "") << r
           << config.getNestedColor("detailed_resource_usage", "fields.uptime.value_color", "")
           << perf.get_system_uptime() << r
           << config.getNestedColor("detailed_resource_usage", "fields.uptime.value_suffix_color", "")
           << config.getLabel("detailed_resource_usage", "fields.uptime.value_suffix", "") << r;
        lp.push(ss.str());
    };

    // ---------- CPU USAGE ----------
    fields["cpu_usage"] = [&]() {
        if (!config.getNestedBool("detailed_resource_usage", "fields.cpu_usage.show", true)) return;

        float cpu = perf.get_cpu_usage_percent();

        // call the visualizer funtion
        std::string cpuVisualizer = makeVisualizer(cpu,config,"detailed_resource_usage","cpu_usage");

        ostringstream cpuSs;

        cpuSs << config.getNestedColor("detailed_resource_usage", "fields.cpu_usage.cpu_usage_prefix_color", "")
              << config.getLabel("detailed_resource_usage", "fields.cpu_usage.cpu_usage_prefix", "") << r

              << config.getNestedColor("detailed_resource_usage", "fields.cpu_usage.label_color", "")
              << config.getLabel("detailed_resource_usage", "fields.cpu_usage.label", "") << r

              << config.getNestedColor("detailed_resource_usage", "fields.cpu_usage.label_suffix_color", "")
              << config.getLabel("detailed_resource_usage", "fields.cpu_usage.label_suffix", "") << r;

        // Add the visualizer.
        if (!cpuVisualizer.empty())
            cpuSs << cpuVisualizer << " ";

        // Apply the normal value color again after the visualizer reset.
        cpuSs << config.getNestedColor("detailed_resource_usage", "fields.cpu_usage.value_color", "")
              << static_cast<int>(cpu)
              << config.getNestedColor("detailed_resource_usage", "fields.cpu_usage.value_suffix_color", "")
              << config.getLabel("detailed_resource_usage", "fields.cpu_usage.value_suffix", "") << r;

        lp.push(cpuSs.str());
    };

    // ---------- RAM USAGE ----------
    fields["ram_usage"] = [&]() {
        if (!config.getNestedBool("detailed_resource_usage", "fields.ram_usage.show", true)) return;

        float ram = perf.get_ram_usage_percent();

        // call the visualizer function
        std::string ramVisualizer = makeVisualizer(ram,config,"detailed_resource_usage","ram_usage");

        ostringstream ramSs;

        ramSs << config.getNestedColor("detailed_resource_usage", "fields.ram_usage.ram_usage_prefix_color", "")
              << config.getLabel("detailed_resource_usage", "fields.ram_usage.ram_usage_prefix", "") << r
              << config.getNestedColor("detailed_resource_usage", "fields.ram_usage.label_color", "")
              << config.getLabel("detailed_resource_usage", "fields.ram_usage.label", "") << r
              << config.getNestedColor("detailed_resource_usage", "fields.ram_usage.label_suffix_color", "")
              << config.getLabel("detailed_resource_usage", "fields.ram_usage.label_suffix", "") << r;

        // Add the visualizer.
        if (!ramVisualizer.empty())
            ramSs << ramVisualizer << " ";

        // Apply the normal value color again after the visualizer reset.
        ramSs << config.getNestedColor("detailed_resource_usage", "fields.ram_usage.value_color", "")
              << static_cast<int>(ram)
              << config.getNestedColor("detailed_resource_usage", "fields.ram_usage.value_suffix_color", "")
              << config.getLabel("detailed_resource_usage", "fields.ram_usage.value_suffix", "") << r;

        lp.push(ramSs.str());
    };

    // ---------- DISK USAGE ----------
    fields["disk_usage"] = [&]() {
        if (!config.getNestedBool("detailed_resource_usage", "fields.disk_usage.show", true)) return;

        float disk = perf.get_disk_usage_percent();

        // call the visualizer function
        std::string diskVisualizer = makeVisualizer(disk,config,"detailed_resource_usage","disk_usage");

        ostringstream diskSs;

        diskSs << config.getNestedColor("detailed_resource_usage", "fields.disk_usage.disk_usage_prefix_color", "")
               << config.getLabel("detailed_resource_usage", "fields.disk_usage.disk_usage_prefix", "") << r
               << config.getNestedColor("detailed_resource_usage", "fields.disk_usage.label_color", "")
               << config.getLabel("detailed_resource_usage", "fields.disk_usage.label", "") << r
               << config.getNestedColor("detailed_resource_usage", "fields.disk_usage.label_suffix_color", "")
               << config.getLabel("detailed_resource_usage", "fields.disk_usage.label_suffix", "") << r;

        // Add the visualizer.
        if (!diskVisualizer.empty())
            diskSs << diskVisualizer << " ";

        // Apply the normal value color again after the visualizer reset.
        diskSs << config.getNestedColor("detailed_resource_usage", "fields.disk_usage.value_color", "")
               << static_cast<int>(disk)
               << config.getNestedColor("detailed_resource_usage", "fields.disk_usage.value_suffix_color", "")
               << config.getLabel("detailed_resource_usage", "fields.disk_usage.value_suffix", "") << r;

        lp.push(diskSs.str());
    };

    // ---------- GPU USAGE ----------
    fields["gpu_usage"] = [&]() {
        if (!config.getNestedBool("detailed_resource_usage", "fields.gpu_usage.show", true)) return;

        float gpu = perf.get_gpu_usage_percent();

        // call the visualizer function
        std::string gpuVisualizer = makeVisualizer(gpu,config,"detailed_resource_usage","gpu_usage");

        ostringstream gpuSs;

        gpuSs << config.getNestedColor("detailed_resource_usage", "fields.gpu_usage.gpu_usage_prefix_color", "")
              << config.getLabel("detailed_resource_usage", "fields.gpu_usage.gpu_usage_prefix", "") << r
              << config.getNestedColor("detailed_resource_usage", "fields.gpu_usage.label_color", "")
              << config.getLabel("detailed_resource_usage", "fields.gpu_usage.label", "") << r
              << config.getNestedColor("detailed_resource_usage", "fields.gpu_usage.label_suffix_color", "")
              << config.getLabel("detailed_resource_usage", "fields.gpu_usage.label_suffix", "") << r;

        // Add the visualizer.
        if (!gpuVisualizer.empty())
            gpuSs << gpuVisualizer << " ";

        // Apply the normal value color again after the visualizer reset.
        gpuSs << config.getNestedColor("detailed_resource_usage", "fields.gpu_usage.value_color", "")
              << static_cast<int>(gpu)
              << config.getNestedColor("detailed_resource_usage", "fields.gpu_usage.value_suffix_color", "")
              << config.getLabel("detailed_resource_usage", "fields.gpu_usage.value_suffix", "") << r;

        lp.push(gpuSs.str());
    };

    // ---- Run fields in the order JSON specifies ----
    // Each field prints its own line via lp.push(...), so this only
    // controls sequence — matching the pattern used by
    // detailed_processor, detailed_operating_system, etc.
    static const std::vector<std::string> defaultOrder =
        {"uptime", "cpu_usage", "ram_usage", "disk_usage", "gpu_usage"};
    auto order = config.getStringArray("detailed_resource_usage", "order", defaultOrder);

    for (const auto& key : order) {
        auto it = fields.find(key);
        if (it != fields.end()) it->second();
    }
};

//   █████╗ ██╗   ██╗██████╗ ██╗ ██████╗     █████╗     ██████╗  ██████╗ ██╗    ██╗███████╗██████╗ 
//  ██╔══██╗██║   ██║██╔══██╗██║██╔═══██╗   ██╔══██╗    ██╔══██╗██╔═══██╗██║    ██║██╔════╝██╔══██╗
//  ███████║██║   ██║██║  ██║██║██║   ██║   ███████║    ██████╔╝██║   ██║██║ █╗ ██║█████╗  ██████╔╝
//  ██╔══██║██║   ██║██║  ██║██║██║   ██║   ██╔══██║    ██╔═══╝ ██║   ██║██║███╗██║██╔══╝  ██╔══██╗
//  ██║  ██║╚██████╔╝██████╔╝██║╚██████╔╝   ██║  ██║    ██║     ╚██████╔╝╚███╔███╔╝███████╗██║  ██║
//  ╚═╝  ╚═╝ ╚═════╝ ╚═════╝ ╚═╝ ╚═════╝    ╚═╝  ╚═╝    ╚═╝      ╚═════╝  ╚══╝╚══╝ ╚══════╝╚═╝  ╚═╝
//                   D E T A I L E D   A U D I O   &   P O W E R
//  This section displays audio device and power/battery information:
//  1. Audio Output Devices - List of active/available playback devices
//  2. Audio Input Devices  - List of active/available recording devices
//  3. Power Status         - Wired connection or battery percentage/charging state
//  All labels, values, colors, prefixes, and toggles are fully JSON-driven
//  via the "detailed_audio_and_power" config block (aliased as "detailed_audio_and_power").
//
//  Output Example:
//  #- Audio Output -----------------------------------------------------#
//  ~ 1 Speakers (Realtek High Definition Audio) (active)
//  #- Audio Input ------------------------------------------------------#
//  ~ 1 Microphone Array (Realtek High Definition Audio) (active)
//  #- Power  -------------------------------------------------------------#
//  ~ Battery powered (87%) (Charging)

// Audio & Power Info (JSON Driven)
sections["detailed_audio_and_power"] = [&]() {
    if (!config.isEnabled("detailed_audio_and_power")) return;

    // line spacing json driven
    int spacing = config.getNestedInt("detailed_audio_and_power","top_line_spacing",0);
    for (int n = 0; n < spacing; n++) {lp.push("");}

    ExtraInfo audio;

    // ---- Register each orderable subsection as a named lambda ----
    std::map<std::string, std::function<void()>> fields;

    // ---------- OUTPUT DEVICES ----------
    fields["output"] = [&]() {
        if (!config.getNestedBool("detailed_audio_and_power", "output.show", true)) return;

        vector<AudioDevice> outputDevices = audio.get_output_devices();

        // line spacing json driven
        int spacing = config.getNestedInt("detailed_audio_and_power","output.top_line_spacing",0);
        for (int n = 0; n < spacing; n++) {lp.push("");}

        ostringstream ss;
        ss << config.getNestedColor("detailed_audio_and_power", "output.header.prefix_color", "")
           << config.getLabel("detailed_audio_and_power", "output.header.prefix", "") << r
           << config.getNestedColor("detailed_audio_and_power", "output.header.text_color", "")
           << config.getLabel("detailed_audio_and_power", "output.header.text", "") << r
           << config.getNestedColor("detailed_audio_and_power", "output.header.suffix_color", "")
           << config.getLabel("detailed_audio_and_power", "output.header.suffix", "") << r;
        lp.push(ss.str());

        int audio_output_device_count = 0;
        for (const auto& device : outputDevices) {
            audio_output_device_count++;
            ostringstream oss;
            oss << config.getNestedColor("detailed_audio_and_power", "output.fields.device.prefix_color", "")
                << config.getLabel("detailed_audio_and_power", "output.fields.device.prefix", "") << r
                << config.getNestedColor("detailed_audio_and_power", "output.fields.device.index_color", "")
                << audio_output_device_count << r << " "
                << config.getNestedColor("detailed_audio_and_power", "output.fields.device.name_color", "")
                << device.name << r
                << config.getNestedColor("detailed_audio_and_power", "output.fields.device.value_suffix_color", "")
                << config.getLabel("detailed_audio_and_power", "output.fields.device.value_suffix", "") << r;

            if (device.isActive) {
                oss << " " << config.getNestedColor("detailed_audio_and_power", "output.fields.device.active_label_color", "")
                    << config.getLabel("detailed_audio_and_power", "output.fields.device.active_label", "") << r;
            }
            lp.push(oss.str());
        }
    };

    // ---------- INPUT DEVICES ----------
    fields["input"] = [&]() {
        if (!config.getNestedBool("detailed_audio_and_power", "input.show", true)) return;

        vector<AudioDevice> inputDevices = audio.get_input_devices();

        // line spacing json driven
        int spacing = config.getNestedInt("detailed_audio_and_power","input.top_line_spacing",0);
        for (int n = 0; n < spacing; n++) {lp.push("");}

        ostringstream ss;
        ss << config.getNestedColor("detailed_audio_and_power", "input.header.prefix_color", "")
           << config.getLabel("detailed_audio_and_power", "input.header.prefix", "") << r
           << config.getNestedColor("detailed_audio_and_power", "input.header.text_color", "")
           << config.getLabel("detailed_audio_and_power", "input.header.text", "") << r
           << config.getNestedColor("detailed_audio_and_power", "input.header.suffix_color", "")
           << config.getLabel("detailed_audio_and_power", "input.header.suffix", "") << r;
        lp.push(ss.str());

        int audio_input_device_count = 0;
        for (const auto& device : inputDevices) {
            audio_input_device_count++;
            ostringstream oss;
            oss << config.getNestedColor("detailed_audio_and_power", "input.fields.device.prefix_color", "")
                << config.getLabel("detailed_audio_and_power", "input.fields.device.prefix", "") << r
                << config.getNestedColor("detailed_audio_and_power", "input.fields.device.index_color", "")
                << audio_input_device_count << r << " "
                << config.getNestedColor("detailed_audio_and_power", "input.fields.device.name_color", "")
                << device.name << r
                << config.getNestedColor("detailed_audio_and_power", "input.fields.device.value_suffix_color", "")
                << config.getLabel("detailed_audio_and_power", "input.fields.device.value_suffix", "") << r;

            if (device.isActive) {
                oss << " " << config.getNestedColor("detailed_audio_and_power", "input.fields.device.active_label_color", "")
                    << config.getLabel("detailed_audio_and_power", "input.fields.device.active_label", "") << r;
            }
            lp.push(oss.str());
        }
    };

    // ---------- POWER STATUS ----------
    fields["power"] = [&]() {
        if (!config.getNestedBool("detailed_audio_and_power", "power.show", true)) return;

        // line spacing json driven
        int spacing = config.getNestedInt("detailed_audio_and_power","power.top_line_spacing",0);
        for (int n = 0; n < spacing; n++) {lp.push("");}

        PowerStatus power = audio.get_power_status();

        ostringstream ss;
        ss << config.getNestedColor("detailed_audio_and_power", "power.header.prefix_color", "")
           << config.getLabel("detailed_audio_and_power", "power.header.prefix", "") << r
           << config.getNestedColor("detailed_audio_and_power", "power.header.text_color", "")
           << config.getLabel("detailed_audio_and_power", "power.header.text", "") << r
           << config.getNestedColor("detailed_audio_and_power", "power.header.suffix_color", "")
           << config.getLabel("detailed_audio_and_power", "power.header.suffix", "") << r;
        lp.push(ss.str());

        ostringstream ossPower;
        if (!power.hasBattery) {
            ossPower << config.getNestedColor("detailed_audio_and_power", "power.fields.wired.brackets_color", "")
                      << config.getLabel("detailed_audio_and_power", "power.fields.wired.bracket_open", "") << r
                      << config.getNestedColor("detailed_audio_and_power", "power.fields.wired.text_color", "")
                      << config.getLabel("detailed_audio_and_power", "power.fields.wired.text", "") << r
                      << config.getNestedColor("detailed_audio_and_power", "power.fields.wired.value_suffix_color", "")
                      << config.getLabel("detailed_audio_and_power", "power.fields.wired.value_suffix", "") << r
                      << config.getNestedColor("detailed_audio_and_power", "power.fields.wired.brackets_color", "")
                      << config.getLabel("detailed_audio_and_power", "power.fields.wired.bracket_close", "") << r;
        }
        else {
            ossPower << config.getNestedColor("detailed_audio_and_power", "power.fields.battery.prefix_color", "")
                      << config.getLabel("detailed_audio_and_power", "power.fields.battery.prefix", "") << r
                      << config.getNestedColor("detailed_audio_and_power", "power.fields.battery.label_color", "")
                      << config.getLabel("detailed_audio_and_power", "power.fields.battery.label", "") << r
                      << config.getNestedColor("detailed_audio_and_power", "power.fields.battery.label_suffix_color", "")
                      << config.getLabel("detailed_audio_and_power", "power.fields.battery.label_suffix", "") << r
                      << config.getNestedColor("detailed_audio_and_power", "power.fields.battery.brackets_color", "")
                      << config.getLabel("detailed_audio_and_power", "power.fields.battery.bracket_open", "") << r
                      << config.getNestedColor("detailed_audio_and_power", "power.fields.battery.value_color", "")
                      << power.batteryPercent << r
                      << config.getNestedColor("detailed_audio_and_power", "power.fields.battery.value_suffix_color", "")
                      << config.getLabel("detailed_audio_and_power", "power.fields.battery.value_suffix", "") << r
                      << config.getNestedColor("detailed_audio_and_power", "power.fields.battery.brackets_color", "")
                      << config.getLabel("detailed_audio_and_power", "power.fields.battery.bracket_close", "") << r;

            if (power.isCharging) {
                ossPower << " " << config.getNestedColor("detailed_audio_and_power", "power.fields.battery.charging_color", "")
                          << config.getLabel("detailed_audio_and_power", "power.fields.battery.charging_text", "") << r;
            }
            else {
                ossPower << " " << config.getNestedColor("detailed_audio_and_power", "power.fields.battery.not_charging_color", "")
                          << config.getLabel("detailed_audio_and_power", "power.fields.battery.not_charging_text", "") << r;
            }
        }
        lp.push(ossPower.str());
    };

    // ---- Run subsections in the order JSON specifies ----
    // Each subsection prints its own header + device/status lines via
    // lp.push(...), so this only controls sequence — matching the
    // pattern used by detailed_processor, detailed_operating_system, etc.
    static const std::vector<std::string> defaultOrder =
        {"output", "input", "power"};
    auto order = config.getStringArray("detailed_audio_and_power", "order", defaultOrder);

    for (const auto& key : order) {
        auto it = fields.find(key);
        if (it != fields.end()) it->second();
    }
};

// Walk through the section names in the order the JSON "layout" array
// specifies (or the hardcoded default order, if "layout" is missing).
for (const auto& key : config.getLayoutOrder()) {

    // Look up whether a section with this exact name was registered
    // earlier in main() via sections["some_name"] = [&]() { ... };
    auto it = sections.find(key);

    if (it != sections.end()) {
        // Found a match — run that section's lambda now.
        // This is what actually prints the section (header, fields,
        // colors, etc.) to the LivePrinter (lp).
        it->second();
    }
#ifdef _DEBUG
    else {
        // No section is registered under this name — likely a typo
        // in the JSON "layout" array, or a section that was removed
        // from the sections map but left in the JSON.
        // Only warns in debug builds; release builds skip silently
        // so a bad config entry never crashes or interrupts output.
        std::cerr << "Warning: layout entry '" << key
                   << "' does not match any registered section.\n";
    }
#endif
}







    // Print remaining ASCII art lines (if art is taller than info)
    lp.finish();

    cout << endl;






    return 0;
}


