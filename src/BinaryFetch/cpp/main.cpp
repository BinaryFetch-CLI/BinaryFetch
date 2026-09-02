

//Welcome to BinaryFetch entry point (main.cpp) 

#include <iostream>       // Standard input/output stream (cin, cout) 
#include <iomanip>        // Formatting utilities (setw, precision, setfill) 
#include <vector>         // Dynamic array container 
#include <functional>     // Function objects and wrappers (function) 
#include <sstream>        // String stream operations for parsing/conversion 
#include <fstream>        // File stream operations (reading/writing files) 
#include <string>         // Standard string class and methods 
#include <regex>          // Regular expressions for pattern matching 
#include <windows.h>      // Core Windows API functions (handles, processes) 
#include <shlobj.h>       // Shell object functions (folder paths, UI) 
#include <direct.h>       // Directory and file handling functions (_mkdir, _chdir) 
#include <comdef.h>       // Native C++ compiler COM support 
#include <Wbemidl.h>      // WMI (Windows Management Instrumentation) interfaces 


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
//since we've decleared std, we may no longer need it 

// (start) - place holder for global
int global_memory_capacity = 0;

// (end) - place holder for global varaibles





int main(){

    
	


    
    // ========== SIMPLIFIED ASCII ART LOADING ==========
        // Just call loadFromFile() - it handles everything automatically!
        // - Checks C:\Users\<User>\AppData\BinaryFetch\BinaryArt.txt
        // - If missing, copies from Default_Ascii_Art.txt and creates it
        // - User can modify their art anytime in AppData folder

	SetConsoleOutputCP(CP_UTF8); // UTF-8 output on Windows console (for emoji printing)
    AsciiArt art;
    if (!art.loadFromFile()) {
        cout << "Warning: ASCII art could not be loaded. Continuing without art.\n";
        // Program continues even if art fails to load
    }

    // ========== CONFIG MANAGEMENT ==========
    // DEV_MODE = true  → load default JSON directly from project folder (fast iteration 🧪)
    // DEV_MODE = false → production: read/create C:\Users\Public\BinaryFetch\BinaryFetch_Config.json 🛰️
    //                    (self-heals from embedded EXE resource if the file is missing)
    //                    NEVER overwrites an existing user config.
    bool DEV_MODE = true; // ← set to true while developing, false before shipping
    ConfigManager config(DEV_MODE);
    string r = config.getResetColor();

	// Anyway....this is how we're allowed to print emojis in C++ console
    // :cout << u8"😄 ❤️ 🎉 🚀 ⭐ 🐱 🍕 🎮 😭 🌈\n"; 


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




    

    
//-----------------------------testing site start-------------------------
// here, we can test new features before adding them to the main codebase
    
   
    
//-----------------------------testing site end-------------------------


// json based printing workfolow starts here...........................
        




// ==================== HEADER BANNER ====================
if (config.isEnabled("header_settings")) {
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
}




// ============================================================================
//   ██████╗ ██████╗ ███╗   ███╗██████╗  █████╗  ██████╗████████╗
//  ██╔════╝██╔═══██╗████╗ ████║██╔══██╗██╔══██╗██╔════╝╚══██╔══╝
//  ██║     ██║   ██║██╔████╔██║██████╔╝███████║██║        ██║   
//  ██║     ██║   ██║██║╚██╔╝██║██╔═══╝ ██╔══██║██║        ██║   
//  ╚██████╗╚██████╔╝██║ ╚═╝ ██║██║     ██║  ██║╚██████╗   ██║   
//   ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚═╝     ╚═╝  ╚═╝ ╚═════╝   ╚═╝   
// ============================================================================
//                       C O M P A C T   M O D U L E S
// ============================================================================



// ==================== COMPACT TIME ====================
if (config.isEnabled("compact_date_and_time")) {
    TimeInfo time;
    ostringstream ss;

    // Prefix - comes entirely from JSON (can be emoji, text, or empty)
    if (config.isFieldEnabled("compact_date_and_time", "prefixes.show")) {
        ss << config.getColor("compact_date_and_time", "prefixes.prefix_color", "")
           << config.getPrefix("compact_date_and_time", "prefixes.prefix", "") << r;
    }

    // ---------- TIME SECTION ----------
    if (config.isNestedEnabled("compact_date_and_time", "time", "enabled")) {
        ss << config.getNestedColor("compact_date_and_time", "time", "bracket", "") << "(" << r;

        if (config.isNestedEnabled("compact_date_and_time", "time", "show_label")) {
            ss << config.getNestedColor("compact_date_and_time", "time", "label", "") << "Time: " << r;
        }

        bool wrote = false;

        if (config.isNestedEnabled("compact_date_and_time", "time", "show_hour")) {
            ss << config.getNestedColor("compact_date_and_time", "time", "hour", "")
               << setw(2) << setfill('0') << time.getHour() << r;
            wrote = true;
        }

        if (config.isNestedEnabled("compact_date_and_time", "time", "show_minute")) {
            if (wrote) ss << config.getNestedColor("compact_date_and_time", "time", "sep", "") << ":" << r;
            ss << config.getNestedColor("compact_date_and_time", "time", "minute", "")
               << setw(2) << setfill('0') << time.getMinute() << r;
            wrote = true;
        }

        if (config.isNestedEnabled("compact_date_and_time", "time", "show_second")) {
            if (wrote) ss << config.getNestedColor("compact_date_and_time", "time", "sep", "") << ":" << r;
            ss << config.getNestedColor("compact_date_and_time", "time", "second", "")
               << setw(2) << setfill('0') << time.getSecond() << r;
        }

        ss << config.getNestedColor("compact_date_and_time", "time", "bracket", "") << ") " << r;
    }

    // ---------- DATE SECTION ----------
    if (config.isNestedEnabled("compact_date_and_time", "date", "enabled")) {
        ss << config.getNestedColor("compact_date_and_time", "date", "bracket", "") << "(" << r;

        if (config.isNestedEnabled("compact_date_and_time", "date", "show_label")) {
            ss << config.getNestedColor("compact_date_and_time", "date", "label", "") << "Date: " << r;
        }

        bool wrote = false;

        if (config.isNestedEnabled("compact_date_and_time", "date", "show_day")) {
            ss << config.getNestedColor("compact_date_and_time", "date", "day", "")
               << setw(2) << setfill('0') << time.getDay() << r;
            wrote = true;
        }

        if (config.isNestedEnabled("compact_date_and_time", "date", "show_month_name")) {
            if (wrote) ss << config.getNestedColor("compact_date_and_time", "date", "sep", "") << " : " << r;
            ss << config.getNestedColor("compact_date_and_time", "date", "month_name", "")
               << time.getMonthName() << r;
            wrote = true;
        }

        if (config.isNestedEnabled("compact_date_and_time", "date", "show_month_num")) {
            if (wrote) ss << " ";
            ss << config.getNestedColor("compact_date_and_time", "date", "month_num", "")
               << setw(2) << setfill('0') << time.getMonthNumber() << r;
            wrote = true;
        }

        if (config.isNestedEnabled("compact_date_and_time", "date", "show_year")) {
            if (wrote) ss << config.getNestedColor("compact_date_and_time", "date", "sep", "") << " : " << r;
            ss << config.getNestedColor("compact_date_and_time", "date", "year", "")
               << time.getYearNumber() << r;
        }

        ss << config.getNestedColor("compact_date_and_time", "date", "bracket", "") << ") " << r;
    }

    // ---------- WEEK SECTION ----------
    if (config.isNestedEnabled("compact_date_and_time", "week", "enabled")) {
        ss << config.getNestedColor("compact_date_and_time", "week", "bracket", "") << "(" << r;

        if (config.isNestedEnabled("compact_date_and_time", "week", "show_label")) {
            ss << config.getNestedColor("compact_date_and_time", "week", "label", "") << "Week: " << r;
        }

        bool wrote = false;

        if (config.isNestedEnabled("compact_date_and_time", "week", "show_num")) {
            ss << config.getNestedColor("compact_date_and_time", "week", "num", "")
               << time.getWeekNumber() << r;
            wrote = true;
        }

        if (config.isNestedEnabled("compact_date_and_time", "week", "show_day_name")) {
            if (wrote) ss << config.getNestedColor("compact_date_and_time", "week", "sep", "") << " - " << r;
            ss << config.getNestedColor("compact_date_and_time", "week", "day_name", "")
               << time.getDayName() << r;
        }

        ss << config.getNestedColor("compact_date_and_time", "week", "bracket", "") << ") " << r;
    }

    // ---------- LEAP YEAR SECTION ----------
    if (config.isNestedEnabled("compact_date_and_time", "leap_year", "enabled")) {
        ss << config.getNestedColor("compact_date_and_time", "leap_year", "bracket", "") << "(" << r;

        if (config.isNestedEnabled("compact_date_and_time", "leap_year", "show_label")) {
            ss << config.getNestedColor("compact_date_and_time", "leap_year", "label", "") << "Leap Year: " << r;
        }

        if (config.isNestedEnabled("compact_date_and_time", "leap_year", "show_val")) {
            ss << config.getNestedColor("compact_date_and_time", "leap_year", "val", "")
               << time.getLeapYear() << r;
        }

        ss << config.getNestedColor("compact_date_and_time", "leap_year", "bracket", "") << ") " << r;
    }

    lp.push(ss.str());
}

// ==================== COMPACT OPERATING SYSTEM ====================
if (config.isEnabled("compact_operating_system")) {
    ostringstream ss;

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

    // Name field
    if (config.isFieldEnabled("compact_operating_system", "fields.name.show")) {
        ss << config.getColor("compact_operating_system", "fields.name.value_color", "")
           << c_os.getOSName() << r << " ";
    }

    // Build field
    if (config.isFieldEnabled("compact_operating_system", "fields.build.show")) {
        ss << config.getColor("compact_operating_system", "fields.build.value_color", "")
           << c_os.getOSBuild() << r;
    }

    // Architecture (with brackets)
    if (config.isFieldEnabled("compact_operating_system", "fields.arch.show")) {
        ss << config.getColor("compact_operating_system", "brackets.color", "") << "(" << r
           << config.getColor("compact_operating_system", "fields.arch.value_color", "")
           << c_os.getArchitecture() << r
           << config.getColor("compact_operating_system", "brackets.color", "") << ")" << r;
    }

    // Uptime (with brackets)
    if (config.isFieldEnabled("compact_operating_system", "fields.uptime.show")) {
        ss << config.getColor("compact_operating_system", "brackets.color", "") << "(" << r
           << config.getColor("compact_operating_system", "fields.uptime.label_color", "") << "uptime: " << r
           << config.getColor("compact_operating_system", "fields.uptime.value_color", "")
           << c_os.getUptime() << r
           << config.getColor("compact_operating_system", "brackets.color", "") << ")" << r;
    }

    lp.push(ss.str());
}

// ==================== COMPACT CPU ====================
if (config.isEnabled("compact_processor")) {
    ostringstream ss;

    // Prefix - comes entirely from JSON (can be emoji, text, or empty)
    if (config.isFieldEnabled("compact_processor", "prefixes.show")) {
        ss << config.getColor("compact_processor", "prefixes.prefix_color", "")
           << config.getPrefix("compact_processor", "prefixes.prefix", "") << r;
    }

    // Label
    ss << config.getColor("compact_processor", "label.color", "")
       << config.getLabel("compact_processor", "label.text", "CPU") << r;

    // Separator
    ss << config.getColor("compact_processor", "separator.color", "")
       << config.getPrefix("compact_processor", "separator.text", ":") << " " << r;

    // Name field
    if (config.isFieldEnabled("compact_processor", "fields.name.show")) {
        ss << config.getColor("compact_processor", "fields.name.value_color", "")
           << c_cpu.getCPUName() << r;
    }

    // Cores and Threads (with brackets)
    if (config.isFieldEnabled("compact_processor", "fields.cores.show") ||
        config.isFieldEnabled("compact_processor", "fields.threads.show")) {
        ss << config.getColor("compact_processor", "brackets.color", "") << "(" << r;

        if (config.isFieldEnabled("compact_processor", "fields.cores.show")) {
            ss << config.getColor("compact_processor", "fields.cores.value_color", "")
               << c_cpu.getCPUCores() << r
               << config.getColor("compact_processor", "text_color", "") << "C" << r;
        }

        if (config.isFieldEnabled("compact_processor", "fields.cores.show") &&
            config.isFieldEnabled("compact_processor", "fields.threads.show")) {
            ss << "/";
        }

        if (config.isFieldEnabled("compact_processor", "fields.threads.show")) {
            ss << config.getColor("compact_processor", "fields.threads.value_color", "")
               << c_cpu.getCPUThreads() << r
               << config.getColor("compact_processor", "text_color", "") << "T" << r;
        }

        ss << config.getColor("compact_processor", "brackets.color", "") << ")" << r;
    }

    // Clock speed
    if (config.isFieldEnabled("compact_processor", "fields.clock.show")) {
        ss << fixed << setprecision(2)
           << config.getColor("compact_processor", "fields.clock.at_symbol_color", "") << "@" << r
           << config.getColor("compact_processor", "fields.clock.value_color", "") << " "
           << c_cpu.getClockSpeed() << " GHz" << r;
    }

    lp.push(ss.str());
}

// ==================== COMPACT GPU ====================
if (config.isEnabled("compact_graphics_card")) {
    ostringstream ss;

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

    // Name field
    if (config.isFieldEnabled("compact_graphics_card", "fields.name.show")) {
        ss << config.getColor("compact_graphics_card", "fields.name.value_color", "")
           << c_gpu.getGPUName() << r;
    }

    // Usage (with brackets)
    if (config.isFieldEnabled("compact_graphics_card", "fields.usage.show")) {
        ss << config.getColor("compact_graphics_card", "brackets.color", "") << "(" << r
           << config.getColor("compact_graphics_card", "fields.usage.value_color", "")
           << c_gpu.getGPUUsagePercent() << "%" << r
           << config.getColor("compact_graphics_card", "brackets.color", "") << ")" << r;
    }

    // VRAM (with brackets)
    if (config.isFieldEnabled("compact_graphics_card", "fields.vram.show")) {
        ss << config.getColor("compact_graphics_card", "brackets.color", "") << "(" << r
           << config.getColor("compact_graphics_card", "fields.vram.value_color", "")
           << c_gpu.getVRAMGB() << " GB" << r
           << config.getColor("compact_graphics_card", "brackets.color", "") << ")" << r;
    }

    // Frequency (with brackets)
    if (config.isFieldEnabled("compact_graphics_card", "fields.freq.show")) {
        ss << config.getColor("compact_graphics_card", "brackets.color", "") << "(" << r
           << config.getColor("compact_graphics_card", "fields.freq.at_symbol_color", "") << "@" << r
           << config.getColor("compact_graphics_card", "fields.freq.value_color", "")
           << c_gpu.getGPUFrequency() << r
           << config.getColor("compact_graphics_card", "brackets.color", "") << ") " << r;
    }

    lp.push(ss.str());
}

// ==================== COMPACT DISPLAY ====================
if (config.isEnabled("compact_display_monitor")) {
    CompactScreen screenDetector;
    auto screens = screenDetector.getScreens();

    if (screens.empty()) {
        ostringstream ss;
        ss << config.getColor("compact_display_monitor", "header.text_color", "")
           << config.getLabel("compact_display_monitor", "header.text", "Display") << r
           << config.getColor("compact_display_monitor", "header.separator_color", "")
           << config.getPrefix("compact_display_monitor", "header.separator", ":") << " " << r
           << config.getColor("compact_display_monitor", "fields.name.value_color", "")
           << "No displays detected" << r;
        lp.push(ss.str());
    } else {
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

            // Display name
            if (config.isFieldEnabled("compact_display_monitor", "fields.name.show")) {
                ss << config.getColor("compact_display_monitor", "fields.name.value_color", "")
                   << screen.name << r << " ";
            }

            // Resolution: (3840 x 2160)
            if (config.isFieldEnabled("compact_display_monitor", "fields.resolution.show")) {
                ss << config.getColor("compact_display_monitor", "brackets.color", "") << "(" << r
                   << config.getColor("compact_display_monitor", "fields.resolution.value_color", "")
                   << screen.native_width << r
                   << config.getColor("compact_display_monitor", "fields.resolution.x_color", "") << " x " << r
                   << config.getColor("compact_display_monitor", "fields.resolution.value_color", "")
                   << screen.native_height << r
                   << config.getColor("compact_display_monitor", "brackets.color", "") << ") " << r;
            }

            // Scale: (Scale: 175%)
            if (config.isFieldEnabled("compact_display_monitor", "fields.scale.show")) {
                ss << config.getColor("compact_display_monitor", "brackets.color", "") << "(" << r
                   << config.getColor("compact_display_monitor", "fields.scale.label_color", "") << "Scale: " << r
                   << config.getColor("compact_display_monitor", "fields.scale.value_color", "")
                   << screen.scale_percent << "%" << r
                   << config.getColor("compact_display_monitor", "brackets.color", "") << ") " << r;
            }

            // Upscale: (upscale: 4x)
            if (config.isFieldEnabled("compact_display_monitor", "fields.upscale.show")) {
                ss << config.getColor("compact_display_monitor", "brackets.color", "") << "(" << r
                   << config.getColor("compact_display_monitor", "fields.upscale.label_color", "") << "upscale: " << r
                   << config.getColor("compact_display_monitor", "fields.upscale.value_color", "")
                   << screen.upscale << r
                   << config.getColor("compact_display_monitor", "brackets.color", "") << ") " << r;
            }

            // Refresh rate: (@60Hz)
            if (config.isFieldEnabled("compact_display_monitor", "fields.refresh.show")) {
                ss << config.getColor("compact_display_monitor", "brackets.color", "") << "(" << r
                   << config.getColor("compact_display_monitor", "fields.refresh.at_symbol_color", "") << "@" << r
                   << config.getColor("compact_display_monitor", "fields.refresh.value_color", "")
                   << screen.refresh_rate << "Hz" << r
                   << config.getColor("compact_display_monitor", "brackets.color", "") << ")" << r;
            }

            lp.push(ss.str());
        }
    }
}

// ==================== COMPACT MEMORY ====================
if (config.isEnabled("compact_system_memory")) {
    ostringstream ss;

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

    // Total memory (with brackets)
    if (config.isFieldEnabled("compact_system_memory", "fields.total.show")) {
        ss << config.getColor("compact_system_memory", "brackets.color", "") << "(" << r
           << config.getColor("compact_system_memory", "fields.total.label_color", "") << "total: " << r
           << config.getColor("compact_system_memory", "fields.total.value_color", "")
           << c_memory.get_total_memory() << " GB" << r
           << config.getColor("compact_system_memory", "brackets.color", "") << ")" << r;
    }

    // Free memory (with brackets)
    if (config.isFieldEnabled("compact_system_memory", "fields.free.show")) {
        ss << " " << config.getColor("compact_system_memory", "brackets.color", "") << "(" << r
           << config.getColor("compact_system_memory", "fields.total.label_color", "") << "free: " << r
           << config.getColor("compact_system_memory", "fields.free.value_color", "")
           << c_memory.get_free_memory() << " GB" << r
           << config.getColor("compact_system_memory", "brackets.color", "") << ")" << r;
    }

    // Used percentage (with brackets)
    if (config.isFieldEnabled("compact_system_memory", "fields.percent.show")) {
        ss << " " << config.getColor("compact_system_memory", "brackets.color", "") << "(" << r
           << config.getColor("compact_system_memory", "fields.percent.value_color", "")
           << c_memory.get_used_memory_percent() << "%" << r
           << config.getColor("compact_system_memory", "brackets.color", "") << ")" << r;
    }

    lp.push(ss.str());
}

// ==================== COMPACT AUDIO ====================
if (config.isEnabled("compact_audio_devices")) {
    // Input device
    if (config.isFieldEnabled("compact_audio_devices", "input.show")) {
        ostringstream ss;

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
        ss << config.getColor("compact_audio_devices", "brackets.color", "") << "[" << r
           << config.getColor("compact_audio_devices", "input.status_color", "")
           << c_audio.active_audio_input_status() << r
           << config.getColor("compact_audio_devices", "brackets.color", "") << "]" << r;

        lp.push(ss.str());
    }

    // Output device
    if (config.isFieldEnabled("compact_audio_devices", "output.show")) {
        ostringstream ss;

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
        ss << config.getColor("compact_audio_devices", "brackets.color", "") << "[" << r
           << config.getColor("compact_audio_devices", "output.status_color", "")
           << c_audio.active_audio_output_status() << r
           << config.getColor("compact_audio_devices", "brackets.color", "") << "]" << r;

        lp.push(ss.str());
    }
}

// ==================== COMPACT PERFORMANCE ====================
if (config.isEnabled("compact_resource_usage")) {
    ostringstream ss;

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
       << config.getPrefix("compact_resource_usage", "separator.text", ":") << " " << r;

    // Helper lambda for adding performance stats
    auto addPerf = [&](const string& field, const string& label, const string& colorKey, auto val) {
        if (config.isFieldEnabled("compact_resource_usage", "fields." + field + ".show")) {
            ss << config.getColor("compact_resource_usage", "brackets.color", "") << "(" << r
               << config.getColor("compact_resource_usage", "fields." + field + ".label_color", "")
               << label << ": " << r
               << config.getColor("compact_resource_usage", "fields." + field + ".value_color", "")
               << val << "%" << r
               << config.getColor("compact_resource_usage", "brackets.color", "") << ") " << r;
        }
    };

    addPerf("cpu", "CPU", "cpu_color", c_perf.getCPUUsage());
    addPerf("gpu", "GPU", "gpu_color", c_perf.getGPUUsage());
    addPerf("ram", "RAM", "ram_color", c_perf.getRAMUsage());
    addPerf("disk", "Disk", "disk_color", c_perf.getDiskUsage());

    lp.push(ss.str());
}

// ==================== COMPACT USER ====================
if (config.isEnabled("compact_user_account")) {
    ostringstream ss;

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

    // Username
    if (config.isFieldEnabled("compact_user_account", "fields.username.show")) {
        ss << config.getColor("compact_user_account", "fields.username.value_color", "")
           << "@" << c_user.getUsername() << r;
    }

    // Domain (with brackets)
    if (config.isFieldEnabled("compact_user_account", "fields.domain.show")) {
        ss << " " << config.getColor("compact_user_account", "brackets.color", "") << "(" << r
           << config.getColor("compact_user_account", "label_color", "") << "Domain: " << r
           << config.getColor("compact_user_account", "fields.domain.value_color", "")
           << c_user.getDomain() << r
           << config.getColor("compact_user_account", "brackets.color", "") << ")" << r;
    }

    // Type (with brackets)
    if (config.isFieldEnabled("compact_user_account", "fields.type.show")) {
        ss << " " << config.getColor("compact_user_account", "brackets.color", "") << "(" << r
           << config.getColor("compact_user_account", "label_color", "") << "Type: " << r
           << config.getColor("compact_user_account", "fields.type.value_color", "")
           << c_user.isAdmin() << r
           << config.getColor("compact_user_account", "brackets.color", "") << ")" << r;
    }

    lp.push(ss.str());
}

// ==================== COMPACT NETWORK ====================
if (config.isEnabled("compact_network_connection")) {
    if (config.isFieldEnabled("compact_network_connection", "fields.name.show") ||
        config.isFieldEnabled("compact_network_connection", "fields.type.show") ||
        config.isFieldEnabled("compact_network_connection", "fields.ip.show")) {
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

        // Network Name (with brackets)
        if (config.isFieldEnabled("compact_network_connection", "fields.name.show")) {
            ss << config.getColor("compact_network_connection", "brackets.color", "") << "(" << r
               << config.getColor("compact_network_connection", "label_color", "") << "Name: " << r
               << config.getColor("compact_network_connection", "fields.name.value_color", "")
               << c_net.get_network_name() << r
               << config.getColor("compact_network_connection", "brackets.color", "") << ") " << r;
        }

        // Network Type (with brackets)
        if (config.isFieldEnabled("compact_network_connection", "fields.type.show")) {
            ss << config.getColor("compact_network_connection", "brackets.color", "") << "(" << r
               << config.getColor("compact_network_connection", "label_color", "") << "Type: " << r
               << config.getColor("compact_network_connection", "fields.type.value_color", "")
               << c_net.get_network_type() << r
               << config.getColor("compact_network_connection", "brackets.color", "") << ") " << r;
        }

        // IP Address (with brackets)
        if (config.isFieldEnabled("compact_network_connection", "fields.ip.show")) {
            ss << config.getColor("compact_network_connection", "brackets.color", "") << "(" << r
               << config.getColor("compact_network_connection", "label_color", "") << "ip: " << r
               << config.getColor("compact_network_connection", "fields.ip.value_color", "")
               << c_net.get_network_ip() << r
               << config.getColor("compact_network_connection", "brackets.color", "") << ")" << r;
        }

        lp.push(ss.str());
    }
}

// ==================== COMPACT DISK ====================
if (config.isEnabled("compact_disk_storage")) {
    // Disk Usage
    if (config.isFieldEnabled("compact_disk_storage", "usage.show")) {
        auto disks = disk.getAllDiskUsage();
        ostringstream ss;

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
           << config.getPrefix("compact_disk_storage", "usage.separator.text", ":") << " " << r;

        for (const auto& d : disks) {
            ss << config.getColor("compact_disk_storage", "brackets.color", "") << "(" << r
               << config.getColor("compact_disk_storage", "fields.letter_color", "")
               << d.first[0] << ":" << r << " "
               << config.getColor("compact_disk_storage", "fields.percent_color", "")
               << fixed << setprecision(1) << d.second << "%" << r
               << config.getColor("compact_disk_storage", "brackets.color", "") << ") " << r;
        }
        lp.push(ss.str());
    }

    // Disk Capacity
    if (config.isFieldEnabled("compact_disk_storage", "capacity.show")) {
        auto caps = disk.getDiskCapacity();
        ostringstream sc;

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
           << config.getPrefix("compact_disk_storage", "capacity.separator.text", ":") << " " << r;

        for (const auto& c : caps) {
            sc << config.getColor("compact_disk_storage", "brackets.color", "") << "(" << r
               << config.getColor("compact_disk_storage", "fields.letter_color", "")
               << c.first[0] << r
               << config.getColor("compact_disk_storage", "fields.separator_color", "") << "-" << r
               << config.getColor("compact_disk_storage", "fields.capacity_color", "")
               << c.second << "GB" << r
               << config.getColor("compact_disk_storage", "brackets.color", "") << ")" << r;
        }
        lp.push(sc.str());
    }
}



// ============================================================================
//  ██████╗ ███████╗████████╗ █████╗ ██╗██╗     ███████╗██████╗ 
//  ██╔══██╗██╔════╝╚══██╔══╝██╔══██╗██║██║     ██╔════╝██╔══██╗
//  ██║  ██║█████╗     ██║   ███████║██║██║     █████╗  ██   ██╔
//  ██║  ██║██╔══╝     ██║   ██╔══██║██║██║     ██╔══╝  ██╔══██╗
//  ██████╔╝███████╗   ██║   ██║  ██║██║███████╗███████╗██████╔╝
//  ╚═════╝ ╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝╚══════╝╚══════╝╚═════╝ 
// ============================================================================
//                      D E T A I L E D   M O D U L E S
// ============================================================================



// ============================================================================
//                         DETAILED SYSTEM MEMORY
// ============================================================================

if (config.isEnabled("detailed_system_memory")) {
    lp.push("");

    // ---------- HEADER ----------
    if (config.isSectionEnabled("detailed_system_memory", "header")) {
        ostringstream ss;
        ss << config.getColor("detailed_system_memory", "header.prefix_color", "")
           << config.getPrefix("detailed_system_memory", "header.prefix", "") << r
           << config.getColor("detailed_system_memory", "header.text_color", "")
           << config.getLabel("detailed_system_memory", "header.text", "") << r
           << config.getColor("detailed_system_memory", "header.suffix_color", "")
           << config.getPrefix("detailed_system_memory", "header.suffix", "") << r;
        lp.push(ss.str());
    }

    // ---------- SUMMARY (TOTAL, FREE, USED) ----------
    if (config.isSectionEnabled("detailed_system_memory", "total") ||
        config.isSectionEnabled("detailed_system_memory", "free") ||
        config.isSectionEnabled("detailed_system_memory", "used_percentage")) {
        ostringstream ss;

        // ---------- TOTAL ----------
        if (config.isSectionEnabled("detailed_system_memory", "total")) {
            ss << config.getColor("detailed_system_memory", "values.total_prefix_color", "")
               << config.getPrefix("detailed_system_memory", "values.total_prefix", "") << r
               << config.getColor("detailed_system_memory", "brackets.color", "") << "(" << r
               << config.getColor("detailed_system_memory", "labels.total.color", "")
               << config.getLabel("detailed_system_memory", "labels.total.text", "Total: ") << r
               << config.getColor("detailed_system_memory", "values.total_color", "")
               << ram.getTotal() << " GB" << r
               << config.getColor("detailed_system_memory", "brackets.color", "") << ") " << r;
        }

        // ---------- FREE ----------
        if (config.isSectionEnabled("detailed_system_memory", "free")) {
            ss << config.getColor("detailed_system_memory", "values.free_prefix_color", "")
               << config.getPrefix("detailed_system_memory", "values.free_prefix", "") << r
               << config.getColor("detailed_system_memory", "brackets.color", "") << "(" << r
               << config.getColor("detailed_system_memory", "labels.free.color", "")
               << config.getLabel("detailed_system_memory", "labels.free.text", "Free: ") << r
               << config.getColor("detailed_system_memory", "values.free_color", "")
               << ram.getFree() << " GB" << r
               << config.getColor("detailed_system_memory", "brackets.color", "") << ") " << r;
        }

        // ---------- USED PERCENTAGE ----------
        if (config.isSectionEnabled("detailed_system_memory", "used_percentage")) {
            ss << config.getColor("detailed_system_memory", "values.used_prefix_color", "")
               << config.getPrefix("detailed_system_memory", "values.used_prefix", "") << r
               << config.getColor("detailed_system_memory", "brackets.color", "") << "(" << r
               << config.getColor("detailed_system_memory", "labels.used.color", "")
               << config.getLabel("detailed_system_memory", "labels.used.text", "Used: ") << r
               << config.getColor("detailed_system_memory", "values.used_color", "")
               << ram.getUsedPercentage() << "%" << r
               << config.getColor("detailed_system_memory", "brackets.color", "") << ")" << r;
        }

        lp.push(ss.str());
    }

    // ---------- MODULES ----------
    if (config.isSectionEnabled("detailed_system_memory", "modules")) {
        const auto& modules = ram.getModules();
        for (size_t i = 0; i < modules.size(); ++i) {
            string cap = modules[i].capacity;
            int num = 0;
            try { num = stoi(cap); }
            catch (...) { num = 0; }
            ostringstream capOut;
            capOut << setw(2) << setfill('0') << num << "GB";

            ostringstream ss;
            // Memory 0
            ss << config.getColor("detailed_system_memory", "modules.prefix_color", "")
               << config.getPrefix("detailed_system_memory", "modules.prefix", "") << "" << r
               << config.getColor("detailed_system_memory", "modules.label_color", "")
               << config.getLabel("detailed_system_memory", "modules.label", "Memory ") << i << r
               << config.getColor("detailed_system_memory", "modules.separator_color", "")
               << config.getPrefix("detailed_system_memory", "modules.separator", " : ") << r;

            ss << config.getColor("detailed_system_memory", "brackets.color", "") << "(" << r
               << config.getColor("detailed_system_memory", "labels.used.color", "")
               << config.getLabel("detailed_system_memory", "labels.used.text", "Used: ") << r
               << config.getColor("detailed_system_memory", "values.used_color", "")
               << ram.getUsedPercentage() << "%" << r
               << config.getColor("detailed_system_memory", "brackets.color", "") << ") " << r;

            ss << config.getColor("detailed_system_memory", "modules.capacity_color", "")
               << capOut.str() << r << " "
               << config.getColor("detailed_system_memory", "modules.type_color", "")
               << modules[i].type << r << " "
               << config.getColor("detailed_system_memory", "modules.speed_color", "")
               << modules[i].speed << r;

            lp.push(ss.str());
        }
    }
}
    

 



// ============================================================================
//  ██████╗ ███████╗████████╗ █████╗ ██╗██╗     ███████╗██████╗     ██████╗ ██╗███████╗██╗  ██╗
//  ██╔══██╗██╔════╝╚══██╔══╝██╔══██╗██║██║     ██╔════╝██╔══██╗    ██╔══██╗██║██╔════╝██║ ██╔╝
//  ██║  ██║█████╗     ██║   ███████║██║██║     █████╗  ██████╔╝    ██║  ██║██║███████╗█████╔╝ 
//  ██║  ██║██╔══╝     ██║   ██╔══██║██║██║     ██╔══╝  ██╔══██╗    ██║  ██║██║╚════██║██╔═██╗ 
//  ██████╔╝███████╗   ██║   ██║  ██║██║███████╗███████╗██████╔╝    ██████╔╝██║███████║██║  ██╗
//  ╚═════╝ ╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝╚══════╝╚══════╝╚═════╝     ╚═════╝ ╚═╝╚══════╝╚═╝  ╚═╝
// ============================================================================
//                         D E T A I L E D   S T O R A G E
// ============================================================================
//  This section displays comprehensive disk information in two main parts:
//  1. STORAGE SUMMARY - Shows each disk with capacity, usage, file system,
//     and external/internal status
//  2. DISK PERFORMANCE - Displays read/write speeds and serial numbers
//  3. PREDICTED PERFORMANCE - Estimated speeds (if enabled)
// ============================================================================
//
//  Output Example:
//  ------------------------- STORAGE SUMMARY --------------------------
//   SSD Disk (C:) [ (Used)  218.90 GiB / 237.10 GiB    92% - NTFS  Int ]
//  HDD Disk (D:) [ (Used)  189.10 GiB / 465.76 GiB    40% - NTFS  Int ]
//   USB Disk (G:) [ (Used)  104.02 GiB / 112.64 GiB    92% - NTFS  Ext ]
//
//   -------------------- DISK PERFORMANCE & DETAILS --------------------
//  Disk (C:) [ Read: 1225.44 MB/s | Write:  131.03 MB/s | SN-1000 Int ]
//  Disk (D:) [ Read:  128.76 MB/s | Write:  111.68 MB/s | SN-1001 Int ]
//  Disk (G:) [ Read:  151.20 MB/s | Write:    3.73 MB/s | SN-1002 Ext ]
// ============================================================================

// ----------------- DETAILED STORAGE SECTION -----------------

if (config.isEnabled("detailed_disk_storage")) {

    lp.push("");

    // Format storage values
    auto fmt_storage = [](const string& value) -> string {
        ostringstream oss;
        double number = 0.0;

        try {
            number = stod(value);
        }
        catch (...) {
            number = 0.0;
        }

        oss << fixed
            << setprecision(2)
            << setw(7)
            << right
            << setfill(' ')
            << number;

        return oss.str();
    };

    // Format speed values
    auto fmt_speed = [](const string& value) -> string {
        ostringstream oss;
        double number = 0.0;

        try {
            number = stod(value);
        }
        catch (...) {
            number = 0.0;
        }

        oss << fixed
            << setprecision(2)
            << number;

        string result = oss.str();

        int padding =
            7 - static_cast<int>(result.size());

        if (padding < 0)
            padding = 0;

        return string(padding, ' ') + result;
    };

    // Format percentage
    auto fmt_percentage = [](int percentage) -> string {
        ostringstream oss;

        oss << right
            << setw(4)
            << percentage
            << "%";

        return oss.str();
    };

    vector<storage_data> all_disks_captured;


    // ----------------- STORAGE SUMMARY -----------------

    if (config.getNestedBool(
            "detailed_disk_storage",
            "sections.storage_summary",
            true))
    {
        lp.push("");

        // Header
        if (config.getNestedBool(
                "detailed_disk_storage",
                "storage_summary.header.show",
                true))
        {
            ostringstream ss;

            ss << config.getNestedColor(
                    "detailed_disk_storage",
                    "storage_summary.header.prefix_color",
                    "")
               << config.getPrefix(
                    "detailed_disk_storage",
                    "storage_summary.header.prefix",
                    "")
               << r

               << config.getNestedColor(
                    "detailed_disk_storage",
                    "storage_summary.header.text_color",
                    "")
               << config.getLabel(
                    "detailed_disk_storage",
                    "storage_summary.header.text",
                    "")
               << r

               << config.getNestedColor(
                    "detailed_disk_storage",
                    "storage_summary.header.suffix_color",
                    "")
               << config.getPrefix(
                    "detailed_disk_storage",
                    "storage_summary.header.suffix",
                    "")
               << r;

            lp.push(ss.str());
        }


        // Collect disk information
        storage.process_storage_info(
            [&](const storage_data& d)
            {
                all_disks_captured.push_back(d);

                ostringstream ss;


                // Storage type
                if (config.getNestedBool(
                        "detailed_disk_storage",
                        "storage_summary.fields.storage_type.show",
                        true))
                {
                    ss << config.getNestedColor(
                            "detailed_disk_storage",
                            "storage_summary.fields.storage_type.value_color",
                            "")
                       << d.storage_type
                       << r
                       << " ";
                }


                // Drive label and drive letter
                if (config.getNestedBool(
                        "detailed_disk_storage",
                        "storage_summary.fields.drive_letter.show",
                        true))
                {
                    // Label
                    ss << config.getNestedColor(
                            "detailed_disk_storage",
                            "storage_summary.fields.drive_letter.label_color",
                            "")
                       << config.getLabel(
                            "detailed_disk_storage",
                            "storage_summary.fields.drive_letter.label",
                            "Disk")
                       << r;

                    // Opening parenthesis
                    ss << config.getNestedColor(
                            "detailed_disk_storage",
                            "storage_summary.fields.drive_letter.letter_prefix_color",
                            "")
                       << config.getPrefix(
                            "detailed_disk_storage",
                            "storage_summary.fields.drive_letter.letter_prefix",
                            "(")
                       << r;

                    // Actual drive letter
                    // The letter still comes from storage_data
                    ss << config.getNestedColor(
                            "detailed_disk_storage",
                            "storage_summary.fields.drive_letter.letter_color",
                            "")
                       << d.drive_letter
                       << r;

                    // Closing parenthesis
                    ss << config.getNestedColor(
                            "detailed_disk_storage",
                            "storage_summary.fields.drive_letter.letter_suffix_color",
                            "")
                       << config.getPrefix(
                            "detailed_disk_storage",
                            "storage_summary.fields.drive_letter.letter_suffix",
                            ")")
                       << r
                       << " ";
                }


                // Opening square bracket
                ss << config.getNestedColor(
                        "detailed_disk_storage",
                        "storage_summary.brackets.square_color",
                        "")
                   << config.getPrefix(
                        "detailed_disk_storage",
                        "storage_summary.brackets.square_open",
                        "[")
                   << r;


                // Used label
                if (config.getNestedBool(
                        "detailed_disk_storage",
                        "storage_summary.fields.used_label.show",
                        true))
                {
                    ss << " "
                       << config.getNestedColor(
                            "detailed_disk_storage",
                            "storage_summary.brackets.round_color",
                            "")
                       << config.getPrefix(
                            "detailed_disk_storage",
                            "storage_summary.brackets.round_open",
                            "(")
                       << r

                       << config.getNestedColor(
                            "detailed_disk_storage",
                            "storage_summary.fields.used_label.value_color",
                            "")
                       << config.getLabel(
                            "detailed_disk_storage",
                            "storage_summary.fields.used_label.text",
                            "")
                       << r

                       << config.getNestedColor(
                            "detailed_disk_storage",
                            "storage_summary.brackets.round_color",
                            "")
                       << config.getPrefix(
                            "detailed_disk_storage",
                            "storage_summary.brackets.round_close",
                            ")")
                       << r

                       << " ";
                }


                // Used space
                if (config.getNestedBool(
                        "detailed_disk_storage",
                        "storage_summary.fields.used_space.show",
                        true))
                {
                    ss << config.getNestedColor(
                            "detailed_disk_storage",
                            "storage_summary.fields.used_space.value_color",
                            "")
                       << fmt_storage(d.used_space)
                       << r

                       << config.getNestedColor(
                            "detailed_disk_storage",
                            "storage_summary.fields.used_space.unit_color",
                            "")
                       << config.getLabel(
                            "detailed_disk_storage",
                            "storage_summary.fields.used_space.unit",
                            "")
                       << r;
                }


                // Separator
                if (config.getNestedBool(
                        "detailed_disk_storage",
                        "storage_summary.separator.show",
                        true))
                {
                    ss << config.getNestedColor(
                            "detailed_disk_storage",
                            "storage_summary.separator.color",
                            "")
                       << config.getPrefix(
                            "detailed_disk_storage",
                            "storage_summary.separator.text",
                            "")
                       << r;
                }


                // Total space
                if (config.getNestedBool(
                        "detailed_disk_storage",
                        "storage_summary.fields.total_space.show",
                        true))
                {
                    ss << config.getNestedColor(
                            "detailed_disk_storage",
                            "storage_summary.fields.total_space.value_color",
                            "")
                       << fmt_storage(d.total_space)
                       << r

                       << config.getNestedColor(
                            "detailed_disk_storage",
                            "storage_summary.fields.total_space.unit_color",
                            "")
                       << config.getLabel(
                            "detailed_disk_storage",
                            "storage_summary.fields.total_space.unit",
                            "")
                       << r;
                }


                // Used percentage
                if (config.getNestedBool(
                        "detailed_disk_storage",
                        "storage_summary.fields.used_percentage.show",
                        true))
                {
                    ss << " ";

                    // Percentage prefix
                    ss << config.getNestedColor(
                            "detailed_disk_storage",
                            "storage_summary.fields.used_percentage.prefix_color",
                            "")
                       << config.getPrefix(
                            "detailed_disk_storage",
                            "storage_summary.fields.used_percentage.prefix",
                            "")
                       << r;

                    // Percentage value
                    ss << config.getNestedColor(
                            "detailed_disk_storage",
                            "storage_summary.fields.used_percentage.value_color",
                            "")
                       << fmt_percentage(d.used_percentage)
                       << r;

                    // Percentage suffix
                    ss << config.getNestedColor(
                            "detailed_disk_storage",
                            "storage_summary.fields.used_percentage.suffix_color",
                            "")
                       << config.getPrefix(
                            "detailed_disk_storage",
                            "storage_summary.fields.used_percentage.suffix",
                            "")
                       << r;
                }


                // Dash separator
                ss << config.getNestedColor(
                        "detailed_disk_storage",
                        "storage_summary.dash.color",
                        "")
                   << config.getPrefix(
                        "detailed_disk_storage",
                        "storage_summary.dash.text",
                        "")
                   << r
                   << " ";


                // File system
                if (config.getNestedBool(
                        "detailed_disk_storage",
                        "storage_summary.fields.file_system.show",
                        true))
                {
                    ss << config.getNestedColor(
                            "detailed_disk_storage",
                            "storage_summary.fields.file_system.value_color",
                            "")
                       << d.file_system
                       << r
                       << " ";
                }


                // External or internal status
                if (config.getNestedBool(
                        "detailed_disk_storage",
                        "storage_summary.fields.external_status.show",
                        true))
                {
                    if (d.is_external)
                    {
                        ss << config.getNestedColor(
                                "detailed_disk_storage",
                                "storage_summary.fields.external_status.external_color",
                                "")
                           << config.getLabel(
                                "detailed_disk_storage",
                                "storage_summary.fields.external_status.external_text",
                                "")
                           << r;
                    }
                    else
                    {
                        ss << config.getNestedColor(
                                "detailed_disk_storage",
                                "storage_summary.fields.external_status.internal_color",
                                "")
                           << config.getLabel(
                                "detailed_disk_storage",
                                "storage_summary.fields.external_status.internal_text",
                                "")
                           << r;
                    }
                }


                // Closing square bracket
                ss << " "
                   << config.getNestedColor(
                        "detailed_disk_storage",
                        "storage_summary.brackets.square_color",
                        "")
                   << config.getPrefix(
                        "detailed_disk_storage",
                        "storage_summary.brackets.square_close",
                        "]")
                   << r;

                lp.push(ss.str());
            }
        );
    }


    // ----------------- DISK PERFORMANCE -----------------

    if (!all_disks_captured.empty() &&
        config.getNestedBool(
            "detailed_disk_storage",
            "sections.disk_performance",
            true))
    {
        lp.push("");

        // Header
        if (config.getNestedBool(
                "detailed_disk_storage",
                "disk_performance.header.show",
                true))
        {
            ostringstream ss;

            ss << config.getNestedColor(
                    "detailed_disk_storage",
                    "disk_performance.header.prefix_color",
                    "")
               << config.getPrefix(
                    "detailed_disk_storage",
                    "disk_performance.header.prefix",
                    "")
               << r

               << config.getNestedColor(
                    "detailed_disk_storage",
                    "disk_performance.header.text_color",
                    "")
               << config.getLabel(
                    "detailed_disk_storage",
                    "disk_performance.header.text",
                    "")
               << r

               << config.getNestedColor(
                    "detailed_disk_storage",
                    "disk_performance.header.suffix_color",
                    "")
               << config.getPrefix(
                    "detailed_disk_storage",
                    "disk_performance.header.suffix",
                    "")
               << r;

            lp.push(ss.str());
        }


        // Print each disk
        for (const auto& d : all_disks_captured)
        {
            ostringstream ss;


            // Drive label and drive letter
            if (config.getNestedBool(
                    "detailed_disk_storage",
                    "disk_performance.fields.drive_letter.show",
                    true))
            {
                // Label
                ss << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance.fields.drive_letter.label_color",
                        "")
                   << config.getLabel(
                        "detailed_disk_storage",
                        "disk_performance.fields.drive_letter.label",
                        "Disk")
                   << r;

                // Opening parenthesis
                ss << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance.fields.drive_letter.letter_prefix_color",
                        "")
                   << config.getPrefix(
                        "detailed_disk_storage",
                        "disk_performance.fields.drive_letter.letter_prefix",
                        "(")
                   << r;

                // Actual drive letter
                ss << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance.fields.drive_letter.letter_color",
                        "")
                   << d.drive_letter
                   << r;

                // Closing parenthesis
                ss << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance.fields.drive_letter.letter_suffix_color",
                        "")
                   << config.getPrefix(
                        "detailed_disk_storage",
                        "disk_performance.fields.drive_letter.letter_suffix",
                        ")")
                   << r
                   << " ";
            }


            // Opening square bracket
            ss << config.getNestedColor(
                    "detailed_disk_storage",
                    "disk_performance.brackets.square_color",
                    "")
               << config.getPrefix(
                    "detailed_disk_storage",
                    "disk_performance.brackets.square_open",
                    "[")
               << r
               << " ";


            // Read speed
            if (config.getNestedBool(
                    "detailed_disk_storage",
                    "disk_performance.fields.read_speed.show",
                    true))
            {
                ss << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance.fields.read_speed.label_color",
                        "")
                   << config.getLabel(
                        "detailed_disk_storage",
                        "disk_performance.fields.read_speed.label",
                        "")
                   << r

                   << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance.fields.read_speed.value_color",
                        "")
                   << fmt_speed(d.read_speed)
                   << r

                   << " "

                   << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance.fields.read_speed.unit_color",
                        "")
                   << config.getLabel(
                        "detailed_disk_storage",
                        "disk_performance.fields.read_speed.unit",
                        "")
                   << r;
            }


            // Pipe before write speed
            if (config.getNestedBool(
                    "detailed_disk_storage",
                    "disk_performance.pipe.show",
                    true))
            {
                bool has_following_field =
                    config.getNestedBool(
                        "detailed_disk_storage",
                        "disk_performance.fields.write_speed.show",
                        true)
                    ||
                    config.getNestedBool(
                        "detailed_disk_storage",
                        "disk_performance.fields.serial_number.show",
                        true)
                    ||
                    config.getNestedBool(
                        "detailed_disk_storage",
                        "disk_performance.fields.external_status.show",
                        true);

                if (has_following_field)
                {
                    ss << " "
                       << config.getNestedColor(
                            "detailed_disk_storage",
                            "disk_performance.pipe.color",
                            "")
                       << config.getPrefix(
                            "detailed_disk_storage",
                            "disk_performance.pipe.text",
                            "|")
                       << r
                       << " ";
                }
            }


            // Write speed
            if (config.getNestedBool(
                    "detailed_disk_storage",
                    "disk_performance.fields.write_speed.show",
                    true))
            {
                ss << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance.fields.write_speed.label_color",
                        "")
                   << config.getLabel(
                        "detailed_disk_storage",
                        "disk_performance.fields.write_speed.label",
                        "")
                   << r

                   << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance.fields.write_speed.value_color",
                        "")
                   << fmt_speed(d.write_speed)
                   << r

                   << " "

                   << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance.fields.write_speed.unit_color",
                        "")
                   << config.getLabel(
                        "detailed_disk_storage",
                        "disk_performance.fields.write_speed.unit",
                        "")
                   << r;
            }


            // Pipe before serial number or status
            if (config.getNestedBool(
                    "detailed_disk_storage",
                    "disk_performance.pipe.show",
                    true))
            {
                bool has_following_field =
                    config.getNestedBool(
                        "detailed_disk_storage",
                        "disk_performance.fields.serial_number.show",
                        true)
                    ||
                    config.getNestedBool(
                        "detailed_disk_storage",
                        "disk_performance.fields.external_status.show",
                        true);

                if (has_following_field)
                {
                    ss << " "
                       << config.getNestedColor(
                            "detailed_disk_storage",
                            "disk_performance.pipe.color",
                            "")
                       << config.getPrefix(
                            "detailed_disk_storage",
                            "disk_performance.pipe.text",
                            "|")
                       << r
                       << " ";
                }
            }


            // Serial number
            if (config.getNestedBool(
                    "detailed_disk_storage",
                    "disk_performance.fields.serial_number.show",
                    true))
            {
                ss << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance.fields.serial_number.value_color",
                        "")
                   << config.getPrefix(
                        "detailed_disk_storage",
                        "disk_performance.fields.serial_number.prefix",
                        "")
                   << d.serial_number
                   << r;
            }


            // External or internal status
            if (config.getNestedBool(
                    "detailed_disk_storage",
                    "disk_performance.fields.external_status.show",
                    true))
            {
                ss << " ";

                if (d.is_external)
                {
                    ss << config.getNestedColor(
                            "detailed_disk_storage",
                            "disk_performance.fields.external_status.external_color",
                            "")
                       << config.getLabel(
                            "detailed_disk_storage",
                            "disk_performance.fields.external_status.external_text",
                            "")
                       << r;
                }
                else
                {
                    ss << config.getNestedColor(
                            "detailed_disk_storage",
                            "disk_performance.fields.external_status.internal_color",
                            "")
                       << config.getLabel(
                            "detailed_disk_storage",
                            "disk_performance.fields.external_status.internal_text",
                            "")
                       << r;
                }
            }


            // Closing square bracket
            ss << " "
               << config.getNestedColor(
                    "detailed_disk_storage",
                    "disk_performance.brackets.square_color",
                    "")
               << config.getPrefix(
                    "detailed_disk_storage",
                    "disk_performance.brackets.square_close",
                    "]")
               << r;

            lp.push(ss.str());
        }
    }


    // ----------------- PREDICTED DISK PERFORMANCE -----------------

    if (!all_disks_captured.empty() &&
        config.getNestedBool(
            "detailed_disk_storage",
            "sections.disk_performance_predicted",
            true))
    {
        lp.push("");

        // Header
        if (config.getNestedBool(
                "detailed_disk_storage",
                "disk_performance_predicted.header.show",
                false))
        {
            ostringstream ss;

            ss << config.getNestedColor(
                    "detailed_disk_storage",
                    "disk_performance_predicted.header.prefix_color",
                    "")
               << config.getPrefix(
                    "detailed_disk_storage",
                    "disk_performance_predicted.header.prefix",
                    "")
               << r

               << config.getNestedColor(
                    "detailed_disk_storage",
                    "disk_performance_predicted.header.text_color",
                    "")
               << config.getLabel(
                    "detailed_disk_storage",
                    "disk_performance_predicted.header.text",
                    "")
               << r

               << config.getNestedColor(
                    "detailed_disk_storage",
                    "disk_performance_predicted.header.suffix_color",
                    "")
               << config.getPrefix(
                    "detailed_disk_storage",
                    "disk_performance_predicted.header.suffix",
                    "")
               << r;

            lp.push(ss.str());
        }


        // Print predicted values
        for (const auto& d : all_disks_captured)
        {
            ostringstream ss;


            // Drive label and drive letter
            if (config.getNestedBool(
                    "detailed_disk_storage",
                    "disk_performance_predicted.fields.drive_letter.show",
                    true))
            {
                // Label
                ss << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.drive_letter.label_color",
                        "")
                   << config.getLabel(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.drive_letter.label",
                        "Disk")
                   << r;

                // Opening parenthesis
                ss << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.drive_letter.letter_prefix_color",
                        "")
                   << config.getPrefix(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.drive_letter.letter_prefix",
                        "(")
                   << r;

                // Actual drive letter
                ss << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.drive_letter.letter_color",
                        "")
                   << d.drive_letter
                   << r;

                // Closing parenthesis
                ss << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.drive_letter.letter_suffix_color",
                        "")
                   << config.getPrefix(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.drive_letter.letter_suffix",
                        ")")
                   << r
                   << " ";
            }


            // Opening square bracket
            ss << config.getNestedColor(
                    "detailed_disk_storage",
                    "disk_performance_predicted.brackets.square_color",
                    "")
               << config.getPrefix(
                    "detailed_disk_storage",
                    "disk_performance_predicted.brackets.square_open",
                    "[")
               << r
               << " ";


            // Predicted read speed
            if (config.getNestedBool(
                    "detailed_disk_storage",
                    "disk_performance_predicted.fields.read_speed.show",
                    true))
            {
                ss << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.read_speed.label_color",
                        "")
                   << config.getLabel(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.read_speed.label",
                        "")
                   << r

                   << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.read_speed.value_color",
                        "")
                   << fmt_speed(d.predicted_read_speed)
                   << r

                   << " "

                   << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.read_speed.unit_color",
                        "")
                   << config.getLabel(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.read_speed.unit",
                        "")
                   << r;
            }


            // Pipe before write speed
            if (config.getNestedBool(
                    "detailed_disk_storage",
                    "disk_performance_predicted.pipe.show",
                    true))
            {
                bool has_following_field =
                    config.getNestedBool(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.write_speed.show",
                        true)
                    ||
                    config.getNestedBool(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.serial_number.show",
                        true)
                    ||
                    config.getNestedBool(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.external_status.show",
                        true);

                if (has_following_field)
                {
                    ss << " "
                       << config.getNestedColor(
                            "detailed_disk_storage",
                            "disk_performance_predicted.pipe.color",
                            "")
                       << config.getPrefix(
                            "detailed_disk_storage",
                            "disk_performance_predicted.pipe.text",
                            "|")
                       << r
                       << " ";
                }
            }


            // Predicted write speed
            if (config.getNestedBool(
                    "detailed_disk_storage",
                    "disk_performance_predicted.fields.write_speed.show",
                    true))
            {
                ss << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.write_speed.label_color",
                        "")
                   << config.getLabel(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.write_speed.label",
                        "")
                   << r

                   << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.write_speed.value_color",
                        "")
                   << fmt_speed(d.predicted_write_speed)
                   << r

                   << " "

                   << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.write_speed.unit_color",
                        "")
                   << config.getLabel(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.write_speed.unit",
                        "")
                   << r;
            }


            // Pipe before serial number or status
            if (config.getNestedBool(
                    "detailed_disk_storage",
                    "disk_performance_predicted.pipe.show",
                    true))
            {
                bool has_following_field =
                    config.getNestedBool(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.serial_number.show",
                        true)
                    ||
                    config.getNestedBool(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.external_status.show",
                        true);

                if (has_following_field)
                {
                    ss << " "
                       << config.getNestedColor(
                            "detailed_disk_storage",
                            "disk_performance_predicted.pipe.color",
                            "")
                       << config.getPrefix(
                            "detailed_disk_storage",
                            "disk_performance_predicted.pipe.text",
                            "|")
                       << r
                       << " ";
                }
            }


            // Serial number
            if (config.getNestedBool(
                    "detailed_disk_storage",
                    "disk_performance_predicted.fields.serial_number.show",
                    true))
            {
                ss << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.serial_number.value_color",
                        "")
                   << config.getPrefix(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.serial_number.prefix",
                        "")
                   << d.serial_number
                   << r;
            }


            // External or internal status
            if (config.getNestedBool(
                    "detailed_disk_storage",
                    "disk_performance_predicted.fields.external_status.show",
                    true))
            {
                ss << " ";

                if (d.is_external)
                {
                    ss << config.getNestedColor(
                            "detailed_disk_storage",
                            "disk_performance_predicted.fields.external_status.external_color",
                            "")
                       << config.getLabel(
                            "detailed_disk_storage",
                            "disk_performance_predicted.fields.external_status.external_text",
                            "")
                       << r;
                }
                else
                {
                    ss << config.getNestedColor(
                            "detailed_disk_storage",
                            "disk_performance_predicted.fields.external_status.internal_color",
                            "")
                       << config.getLabel(
                            "detailed_disk_storage",
                            "disk_performance_predicted.fields.external_status.internal_text",
                            "")
                       << r;
                }
            }


            // Closing square bracket
            ss << " "
               << config.getNestedColor(
                    "detailed_disk_storage",
                    "disk_performance_predicted.brackets.square_color",
                    "")
               << config.getPrefix(
                    "detailed_disk_storage",
                    "disk_performance_predicted.brackets.square_close",
                    "]")
               << r;

            lp.push(ss.str());
        }
    }


    // No drives were found
    if (all_disks_captured.empty())
    {
        lp.push(
            config.getLabel(
                "detailed_disk_storage",
                "no_drives",
                "")
        );
    }
}

// ----------------- END DETAILED STORAGE SECTION -----------------

// ============================================================================
//  ███╗   ██╗███████╗████████╗██╗    ██╗ ██████╗ ██████╗ ██╗  ██╗
//  ████╗  ██║██╔════╝╚══██╔══╝██║    ██║██╔═══██╗██╔══██╗██║ ██╔╝
//  ██╔██╗ ██║█████╗     ██║   ██║ █╗ ██║██║   ██║██████╔╝█████╔╝ 
//  ██║╚██╗██║██╔══╝     ██║   ██║███╗██║██║   ██║██╔══██╗██╔═██╗ 
//  ██║ ╚████║███████╗   ██║   ╚███╔███╔╝╚██████╔╝██║  ██║██║  ██╗
//  ╚═╝  ╚═══╝╚══════╝   ╚═╝    ╚══╝╚══╝  ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝
// ============================================================================
//                      D E T A I L E D   N E T W O R K
// ============================================================================
//  This section displays comprehensive network information including:
//  1. Network Name      - The name of the active network connection
//  2. Network Type      - Type of network (Ethernet, Wi-Fi, etc.)
//  3. Local IP Address  - The local IPv4 address of the machine
//  4. Public IP Address - The external/public IP address
//  5. Locale            - The geographic location based on public IP
//  6. MAC Address       - The physical hardware address of the adapter
//  7. Upload Speed      - The average upload speed of the connection
//  8. Download Speed    - The average download speed of the connection
// ============================================================================
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
// ============================================================================
if (config.isEnabled("detailed_network_connection"))
{
    lp.push("");

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

    field("name",      "name",      net.get_network_name());
    field("type",      "type",      c_net.get_network_type());
    field("local_ip",  "local_ip",  net.get_local_ip());
    field("public_ip", "public_ip", net.get_public_ip());
    field("locale",    "locale",    net.get_locale());
    field("mac",       "mac",       net.get_mac_address());
    field("upload",    "upload",    net.get_network_upload_speed());
    field("download",  "download",  net.get_network_download_speed());
}

// ============================================================================
//  ██████╗ ██╗   ██╗███╗   ███╗███╗   ███╗██╗   ██╗
//  ██╔══██╗██║   ██║████╗ ████║████╗ ████║╚██╗ ██╔╝
//  ██║  ██║██║   ██║██╔████╔██║██╔████╔██║ ╚████╔╝ 
//  ██║  ██║██║   ██║██║╚██╔╝██║██║╚██╔╝██║  ╚██╔╝  
//  ██████╔╝╚██████╔╝██║ ╚═╝ ██║██║ ╚═╝ ██║   ██║   
//  ╚═════╝  ╚═════╝ ╚═╝     ╚═╝╚═╝     ╚═╝   ╚═╝   
// ============================================================================
//                   D E T A I L E D   D U M M Y   N E T W O R K
// ============================================================================
//  This section displays dummy/example network information for testing:
//  1. Network Name      - Example: "InterCentury"
//  2. Network Type      - Example: "Ethernet"
//  3. Local IP Address  - Example: "192.168.1.42"
//  4. Read Speed         - Example: "812.45 Mbps"
//  5. Write Speed        - Example: "634.10 Mbps"
//  All values, labels, colors, prefixes, and units are fully JSON-driven.
// ============================================================================
//
//  Output Example:
//  #- Network Info ---------------------------------------------------#
//  ~ Network Name            : InterCentury
//  ~ Network Type            : Ethernet
//  ~ Local IP                : 192.168.1.42
//  ~ Read Speed              : 812.45 Mbps
//  ~ Write Speed             : 634.10 Mbps
// ============================================================================

if (config.isEnabled("dummy_network_info")) {
    lp.push("");

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

    field("name");
    field("type");
    field("local_ip");
    field("read_speed");
    field("write_speed");
}


// ============================================================================
//   ██████╗ ███████╗    ██╗███╗   ██╗███████╗ ██████╗ 
//  ██╔═══██╗██╔════╝    ██║████╗  ██║██╔════╝██╔═══██╗
//  ██║   ██║███████╗    ██║██╔██╗ ██║█████╗  ██║   ██║
//  ██║   ██║╚════██║    ██║██║╚██╗██║██╔══╝  ██║   ██║
//  ╚██████╔╝███████║    ██║██║ ╚████║██║     ╚██████╔╝
//   ╚═════╝ ╚══════╝    ╚═╝╚═╝  ╚═══╝╚═╝      ╚═════╝ 
// ============================================================================
//                    D E T A I L E D   O P E R A T I N G   S Y S T E M
// ============================================================================
//  This section displays comprehensive OS information including:
//  1. Name          - OS name (e.g., "Windows 11 Pro")
//  2. Build         - OS build/version number
//  3. Architecture  - System architecture (e.g., "64-bit")
//  4. Kernel        - Kernel version info
//  5. Uptime        - System uptime since last boot
//  6. Install Date  - OS installation date
//  7. Serial        - OS serial number
//  All labels, values, colors, prefixes, and toggles are fully JSON-driven
//  via the "detailed_operating_system" config block (aliased as "os_info").
// ============================================================================
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
// ============================================================================

    // OS Info (JSON Driven)
    if (config.isEnabled("os_info")) {
        lp.push("");

        const string sec = "os_info";

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

        field("name",         "name",         os.GetOSName());
        field("build",        "build",        os.GetOSVersion());
        field("architecture", "architecture", os.GetOSArchitecture());
        field("kernel",       "kernel",       os.get_os_kernel_info());
        field("uptime",       "uptime",       os.get_os_uptime());
        field("install_date", "install_date", os.get_os_install_date());
        field("serial",       "serial",       os.get_os_serial_number());
    }

// ============================================================================
//   ██████╗██████╗ ██╗   ██╗    ██╗███╗   ██╗███████╗ ██████╗ 
//  ██╔════╝██╔══██╗██║   ██║    ██║████╗  ██║██╔════╝██╔═══██╗
//  ██║     ██████╔╝██║   ██║    ██║██╔██╗ ██║█████╗  ██║   ██║
//  ██║     ██╔═══╝ ██║   ██║    ██║██║╚██╗██║██╔══╝  ██║   ██║
//  ╚██████╗██║     ╚██████╔╝    ██║██║ ╚████║██║     ╚██████╔╝
//   ╚═════╝╚═╝      ╚═════╝     ╚═╝╚═╝  ╚═══╝╚═╝      ╚═════╝ 
// ============================================================================
//                       D E T A I L E D   P R O C E S S O R
// ============================================================================

    if (config.isEnabled("detailed_processor")) {
        lp.push("");

        // Header
        if (config.getNestedBool("detailed_processor", "header.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("detailed_processor", "header.prefix_color", "")
               << config.getPrefix("detailed_processor", "header.prefix", "") << r
               << config.getNestedColor("detailed_processor", "header.text_color", "")
               << config.getLabel("detailed_processor", "header.text", "") << r
               << config.getNestedColor("detailed_processor", "header.suffix_color", "")
               << config.getPrefix("detailed_processor", "header.suffix", "") << r;
            lp.push(ss.str());
        }

        // Brand
        if (config.getNestedBool("detailed_processor", "fields.brand.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("detailed_processor", "fields.brand.brand_prefix_color", "")
               << config.getPrefix("detailed_processor", "fields.brand.brand_prefix", "") << r
               << config.getNestedColor("detailed_processor", "fields.brand.label_color", "")
               << config.getLabel("detailed_processor", "fields.brand.label", "") << r
               << config.getNestedColor("detailed_processor", "fields.brand.label_suffix_color", "")
               << config.getPrefix("detailed_processor", "fields.brand.label_suffix", "") << r
               << config.getNestedColor("detailed_processor", "fields.brand.value_color", "")
               << cpu.get_cpu_info() << r
               << config.getNestedColor("detailed_processor", "fields.brand.value_suffix_color", "")
               << config.getPrefix("detailed_processor", "fields.brand.value_suffix", "") << r;
            lp.push(ss.str());
        }

        // Utilization
        if (config.getNestedBool("detailed_processor", "fields.utilization.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("detailed_processor", "fields.utilization.utilization_prefix_color", "")
               << config.getPrefix("detailed_processor", "fields.utilization.utilization_prefix", "") << r
               << config.getNestedColor("detailed_processor", "fields.utilization.label_color", "")
               << config.getLabel("detailed_processor", "fields.utilization.label", "") << r
               << config.getNestedColor("detailed_processor", "fields.utilization.label_suffix_color", "")
               << config.getPrefix("detailed_processor", "fields.utilization.label_suffix", "") << r
               << config.getNestedColor("detailed_processor", "fields.utilization.value_color", "")
               << cpu.get_cpu_utilization() << r
               << config.getNestedColor("detailed_processor", "fields.utilization.value_suffix_color", "")
               << config.getPrefix("detailed_processor", "fields.utilization.value_suffix", "%") << r;
            lp.push(ss.str());
        }

        // Speed
        if (config.getNestedBool("detailed_processor", "fields.speed.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("detailed_processor", "fields.speed.speed_prefix_color", "")
               << config.getPrefix("detailed_processor", "fields.speed.speed_prefix", "") << r
               << config.getNestedColor("detailed_processor", "fields.speed.label_color", "")
               << config.getLabel("detailed_processor", "fields.speed.label", "") << r
               << config.getNestedColor("detailed_processor", "fields.speed.label_suffix_color", "")
               << config.getPrefix("detailed_processor", "fields.speed.label_suffix", "") << r
               << config.getNestedColor("detailed_processor", "fields.speed.value_color", "")
               << cpu.get_cpu_speed() << r
               << config.getNestedColor("detailed_processor", "fields.speed.value_suffix_color", "")
               << config.getPrefix("detailed_processor", "fields.speed.value_suffix", "") << r;
            lp.push(ss.str());
        }

        // Base Speed
        if (config.getNestedBool("detailed_processor", "fields.base_speed.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("detailed_processor", "fields.base_speed.base_speed_prefix_color", "")
               << config.getPrefix("detailed_processor", "fields.base_speed.base_speed_prefix", "") << r
               << config.getNestedColor("detailed_processor", "fields.base_speed.label_color", "")
               << config.getLabel("detailed_processor", "fields.base_speed.label", "") << r
               << config.getNestedColor("detailed_processor", "fields.base_speed.label_suffix_color", "")
               << config.getPrefix("detailed_processor", "fields.base_speed.label_suffix", "") << r
               << config.getNestedColor("detailed_processor", "fields.base_speed.value_color", "")
               << cpu.get_cpu_base_speed() << r
               << config.getNestedColor("detailed_processor", "fields.base_speed.value_suffix_color", "")
               << config.getPrefix("detailed_processor", "fields.base_speed.value_suffix", "") << r;
            lp.push(ss.str());
        }

        // Cores
        if (config.getNestedBool("detailed_processor", "fields.cores.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("detailed_processor", "fields.cores.cores_prefix_color", "")
               << config.getPrefix("detailed_processor", "fields.cores.cores_prefix", "") << r
               << config.getNestedColor("detailed_processor", "fields.cores.label_color", "")
               << config.getLabel("detailed_processor", "fields.cores.label", "") << r
               << config.getNestedColor("detailed_processor", "fields.cores.label_suffix_color", "")
               << config.getPrefix("detailed_processor", "fields.cores.label_suffix", "") << r
               << config.getNestedColor("detailed_processor", "fields.cores.value_color", "")
               << cpu.get_cpu_cores() << r
               << config.getNestedColor("detailed_processor", "fields.cores.value_suffix_color", "")
               << config.getPrefix("detailed_processor", "fields.cores.value_suffix", "") << r;
            lp.push(ss.str());
        }

        // Logical Processors
        if (config.getNestedBool("detailed_processor", "fields.logical_processors.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("detailed_processor", "fields.logical_processors.logical_processors_prefix_color", "")
               << config.getPrefix("detailed_processor", "fields.logical_processors.logical_processors_prefix", "") << r
               << config.getNestedColor("detailed_processor", "fields.logical_processors.label_color", "")
               << config.getLabel("detailed_processor", "fields.logical_processors.label", "") << r
               << config.getNestedColor("detailed_processor", "fields.logical_processors.label_suffix_color", "")
               << config.getPrefix("detailed_processor", "fields.logical_processors.label_suffix", "") << r
               << config.getNestedColor("detailed_processor", "fields.logical_processors.value_color", "")
               << cpu.get_cpu_logical_processors() << r
               << config.getNestedColor("detailed_processor", "fields.logical_processors.value_suffix_color", "")
               << config.getPrefix("detailed_processor", "fields.logical_processors.value_suffix", "") << r;
            lp.push(ss.str());
        }

        // Sockets
        if (config.getNestedBool("detailed_processor", "fields.sockets.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("detailed_processor", "fields.sockets.sockets_prefix_color", "")
               << config.getPrefix("detailed_processor", "fields.sockets.sockets_prefix", "") << r
               << config.getNestedColor("detailed_processor", "fields.sockets.label_color", "")
               << config.getLabel("detailed_processor", "fields.sockets.label", "") << r
               << config.getNestedColor("detailed_processor", "fields.sockets.label_suffix_color", "")
               << config.getPrefix("detailed_processor", "fields.sockets.label_suffix", "") << r
               << config.getNestedColor("detailed_processor", "fields.sockets.value_color", "")
               << cpu.get_cpu_sockets() << r
               << config.getNestedColor("detailed_processor", "fields.sockets.value_suffix_color", "")
               << config.getPrefix("detailed_processor", "fields.sockets.value_suffix", "") << r;
            lp.push(ss.str());
        }

        // Virtualization
        if (config.getNestedBool("detailed_processor", "fields.virtualization.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("detailed_processor", "fields.virtualization.virtualization_prefix_color", "")
               << config.getPrefix("detailed_processor", "fields.virtualization.virtualization_prefix", "") << r
               << config.getNestedColor("detailed_processor", "fields.virtualization.label_color", "")
               << config.getLabel("detailed_processor", "fields.virtualization.label", "") << r
               << config.getNestedColor("detailed_processor", "fields.virtualization.label_suffix_color", "")
               << config.getPrefix("detailed_processor", "fields.virtualization.label_suffix", "") << r
               << config.getNestedColor("detailed_processor", "fields.virtualization.value_color", "")
               << cpu.get_cpu_virtualization() << r
               << config.getNestedColor("detailed_processor", "fields.virtualization.value_suffix_color", "")
               << config.getPrefix("detailed_processor", "fields.virtualization.value_suffix", "") << r;
            lp.push(ss.str());
        }

        // L1 Cache
        if (config.getNestedBool("detailed_processor", "fields.l1_cache.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("detailed_processor", "fields.l1_cache.l1_cache_prefix_color", "")
               << config.getPrefix("detailed_processor", "fields.l1_cache.l1_cache_prefix", "") << r
               << config.getNestedColor("detailed_processor", "fields.l1_cache.label_color", "")
               << config.getLabel("detailed_processor", "fields.l1_cache.label", "") << r
               << config.getNestedColor("detailed_processor", "fields.l1_cache.label_suffix_color", "")
               << config.getPrefix("detailed_processor", "fields.l1_cache.label_suffix", "") << r
               << config.getNestedColor("detailed_processor", "fields.l1_cache.value_color", "")
               << cpu.get_cpu_l1_cache() << r
               << config.getNestedColor("detailed_processor", "fields.l1_cache.value_suffix_color", "")
               << config.getPrefix("detailed_processor", "fields.l1_cache.value_suffix", "") << r;
            lp.push(ss.str());
        }

        // L2 Cache
        if (config.getNestedBool("detailed_processor", "fields.l2_cache.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("detailed_processor", "fields.l2_cache.l2_cache_prefix_color", "")
               << config.getPrefix("detailed_processor", "fields.l2_cache.l2_cache_prefix", "") << r
               << config.getNestedColor("detailed_processor", "fields.l2_cache.label_color", "")
               << config.getLabel("detailed_processor", "fields.l2_cache.label", "") << r
               << config.getNestedColor("detailed_processor", "fields.l2_cache.label_suffix_color", "")
               << config.getPrefix("detailed_processor", "fields.l2_cache.label_suffix", "") << r
               << config.getNestedColor("detailed_processor", "fields.l2_cache.value_color", "")
               << cpu.get_cpu_l2_cache() << r
               << config.getNestedColor("detailed_processor", "fields.l2_cache.value_suffix_color", "")
               << config.getPrefix("detailed_processor", "fields.l2_cache.value_suffix", "") << r;
            lp.push(ss.str());
        }

        // L3 Cache
        if (config.getNestedBool("detailed_processor", "fields.l3_cache.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("detailed_processor", "fields.l3_cache.l3_cache_prefix_color", "")
               << config.getPrefix("detailed_processor", "fields.l3_cache.l3_cache_prefix", "") << r
               << config.getNestedColor("detailed_processor", "fields.l3_cache.label_color", "")
               << config.getLabel("detailed_processor", "fields.l3_cache.label", "") << r
               << config.getNestedColor("detailed_processor", "fields.l3_cache.label_suffix_color", "")
               << config.getPrefix("detailed_processor", "fields.l3_cache.label_suffix", "") << r
               << config.getNestedColor("detailed_processor", "fields.l3_cache.value_color", "")
               << cpu.get_cpu_l3_cache() << r
               << config.getNestedColor("detailed_processor", "fields.l3_cache.value_suffix_color", "")
               << config.getPrefix("detailed_processor", "fields.l3_cache.value_suffix", "") << r;
            lp.push(ss.str());
        }
    }

// ============================================================================
//   ██████╗██████╗ ██╗   ██╗    ██╗███╗   ██╗███████╗ ██████╗ 
//  ██╔════╝██╔══██╗██║   ██║    ██║████╗  ██║██╔════╝██╔═══██╗
//  ██║  ███╗██████╔╝██║   ██║    ██║██╔██╗ ██║█████╗  ██║   ██║
//  ██║   ██║██╔═══╝ ██║   ██║    ██║██║╚██╗██║██╔══╝  ██║   ██║
//  ╚██████╔╝██║     ╚██████╔╝    ██║██║ ╚████║██║     ╚██████╔╝
//   ╚═════╝ ╚═╝      ╚═════╝     ╚═╝╚═╝  ╚═══╝╚═╝      ╚═════╝ 
// ============================================================================
//                    D E T A I L E D   G R A P H I C S   C A R D
// ============================================================================

    if (config.isEnabled("detailed_graphics_card")) {
        lp.push("");

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

                // Name
                if (config.getNestedBool("detailed_graphics_card", "fields.name.show", true)) {
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
                }

                // Memory
                if (config.getNestedBool("detailed_graphics_card", "fields.memory.show", true)) {
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
                }

                // Usage
                if (config.getNestedBool("detailed_graphics_card", "fields.usage.show", true)) {
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
                }

                // Vendor
                if (config.getNestedBool("detailed_graphics_card", "fields.vendor.show", true)) {
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
                }

                // Driver Version
                if (config.getNestedBool("detailed_graphics_card", "fields.driver.show", true)) {
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
                }

                // Temperature
                if (config.getNestedBool("detailed_graphics_card", "fields.temperature.show", true)) {
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
                }

                // Core Count
                if (config.getNestedBool("detailed_graphics_card", "fields.cores.show", true)) {
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
                }
            }

            // Primary GPU Details
            auto primary = detailed_gpu_info.primary_gpu_info();

            if (config.getNestedBool("detailed_graphics_card", "primary_header.show", true)) {
                lp.push("");

                ostringstream ss;
                ss << config.getNestedColor("detailed_graphics_card", "primary_header.prefix_color", "")
                   << config.getPrefix("detailed_graphics_card", "primary_header.prefix", "") << r
                   << config.getNestedColor("detailed_graphics_card", "primary_header.text_color", "")
                   << config.getLabel("detailed_graphics_card", "primary_header.text", "") << r
                   << config.getNestedColor("detailed_graphics_card", "primary_header.suffix_color", "")
                   << config.getPrefix("detailed_graphics_card", "primary_header.suffix", "") << r;
                lp.push(ss.str());

                // Primary Name
                if (config.getNestedBool("detailed_graphics_card", "primary_fields.name.show", true)) {
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
                }

                // Primary VRAM
                if (config.getNestedBool("detailed_graphics_card", "primary_fields.vram.show", true)) {
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
                }

                // Primary Frequency
                if (config.getNestedBool("detailed_graphics_card", "primary_fields.freq.show", true)) {
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
                }
            }
        }
    }



// ============================================================================
//   ██████╗ ██╗███████╗██████╗ ██╗      █████╗ ██╗   ██╗
//   ██╔══██╗██║██╔════╝██╔══██╗██║     ██╔══██╗╚██╗ ██╔╝
//   ██║  ██║██║███████╗██████╔╝██║     ███████║ ╚████╔╝ 
//   ██║  ██║██║     ██ ██╗     ██║     ██╔══██║  ╚██╔╝  
//   ██████╔╝██║███████╗██║     ███████╗██║  ██║   ██║   
//   ╚═════╝ ╚═╝╚══════╝╚═╝     ╚══════╝╚═╝  ╚═╝   ╚═╝   
// ============================================================================
//                      D E T A I L E D   D I S P L A Y
// ============================================================================
//  Displays comprehensive monitor information:
//  • Display Banner      - Index number with formatted header
//  • Display Name        - Manufacturer and model
//  • Applied Resolution  - Current resolution @ refresh rate
//  • Native Resolution   - Maximum native resolution
//  • Aspect Ratio        - Width-to-height ratio
//  • Scaling             - DPI scaling percentage
//  • Upscale             - Upscaling multiplier
//  • DSR / VSR           - Dynamic Super Resolution status
// ============================================================================
if (config.isEnabled("display_info")) {
    lp.push("");

    const auto& screens = di.getScreens();

    for (size_t i = 0; i < screens.size(); ++i) {
        const auto& s = screens[i];

        // ---------- Display Banner ----------
        if (config.getNestedBool("display_info", "banner.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("display_info", "banner.prefix_color", "")
               << config.getPrefix("display_info", "banner.prefix", "") << r
               << config.getNestedColor("display_info", "banner.text_color", "")
               << config.getLabel("display_info", "banner.text", "")
               << config.getNestedColor("display_info", "banner.index_color", "")
               << (i + 1) << " " << r
               << config.getNestedColor("display_info", "banner.suffix_color", "")
               << config.getPrefix("display_info", "banner.suffix", "") << r;
            lp.push(ss.str());
        }

        // ---------- Name ----------
        if (config.getNestedBool("display_info", "fields.name.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("display_info", "fields.name.name_prefix_color", "")
               << config.getPrefix("display_info", "fields.name.name_prefix", "") << r
               << config.getNestedColor("display_info", "fields.name.label_color", "")
               << config.getLabel("display_info", "fields.name.label", "") << r
               << config.getNestedColor("display_info", "fields.name.label_suffix_color", "")
               << config.getLabel("display_info", "fields.name.label_suffix", "") << r
               << config.getNestedColor("display_info", "fields.name.value_color", "")
               << s.name << r
               << config.getNestedColor("display_info", "fields.name.value_suffix_color", "")
               << config.getLabel("display_info", "fields.name.value_suffix", "") << r;
            lp.push(ss.str());
        }

        // ---------- Applied Resolution ----------
        if (config.getNestedBool("display_info", "fields.applied_resolution.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("display_info", "fields.applied_resolution.applied_resolution_prefix_color", "")
               << config.getPrefix("display_info", "fields.applied_resolution.applied_resolution_prefix", "") << r
               << config.getNestedColor("display_info", "fields.applied_resolution.label_color", "")
               << config.getLabel("display_info", "fields.applied_resolution.label", "") << r
               << config.getNestedColor("display_info", "fields.applied_resolution.label_suffix_color", "")
               << config.getLabel("display_info", "fields.applied_resolution.label_suffix", "") << r
               << config.getNestedColor("display_info", "fields.applied_resolution.value_color", "")
               << s.current_width
               << config.getNestedColor("display_info", "fields.applied_resolution.x_color", "")
               << config.getLabel("display_info", "fields.applied_resolution.x_symbol", "x") << r
               << config.getNestedColor("display_info", "fields.applied_resolution.value_color", "")
               << s.current_height
               << config.getNestedColor("display_info", "fields.applied_resolution.at_symbol_color", "")
               << config.getLabel("display_info", "fields.applied_resolution.at_symbol", " @") << r
               << config.getNestedColor("display_info", "fields.applied_resolution.value_color", "")
               << s.refresh_rate
               << config.getNestedColor("display_info", "fields.applied_resolution.hz_color", "")
               << config.getLabel("display_info", "fields.applied_resolution.hz_symbol", "Hz") << r
               << config.getNestedColor("display_info", "fields.applied_resolution.value_suffix_color", "")
               << config.getLabel("display_info", "fields.applied_resolution.value_suffix", "") << r;
            lp.push(ss.str());
        }

        // ---------- Native Resolution ----------
        if (config.getNestedBool("display_info", "fields.native_resolution.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("display_info", "fields.native_resolution.native_resolution_prefix_color", "")
               << config.getPrefix("display_info", "fields.native_resolution.native_resolution_prefix", "") << r
               << config.getNestedColor("display_info", "fields.native_resolution.label_color", "")
               << config.getLabel("display_info", "fields.native_resolution.label", "") << r
               << config.getNestedColor("display_info", "fields.native_resolution.label_suffix_color", "")
               << config.getLabel("display_info", "fields.native_resolution.label_suffix", "") << r
               << config.getNestedColor("display_info", "fields.native_resolution.value_color", "")
               << s.native_resolution << r
               << config.getNestedColor("display_info", "fields.native_resolution.value_suffix_color", "")
               << config.getLabel("display_info", "fields.native_resolution.value_suffix", "") << r;
            lp.push(ss.str());
        }

        // ---------- Aspect Ratio ----------
        if (config.getNestedBool("display_info", "fields.aspect_ratio.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("display_info", "fields.aspect_ratio.aspect_ratio_prefix_color", "")
               << config.getPrefix("display_info", "fields.aspect_ratio.aspect_ratio_prefix", "") << r
               << config.getNestedColor("display_info", "fields.aspect_ratio.label_color", "")
               << config.getLabel("display_info", "fields.aspect_ratio.label", "") << r
               << config.getNestedColor("display_info", "fields.aspect_ratio.label_suffix_color", "")
               << config.getLabel("display_info", "fields.aspect_ratio.label_suffix", "") << r
               << config.getNestedColor("display_info", "fields.aspect_ratio.value_color", "")
               << s.aspect_ratio << r
               << config.getNestedColor("display_info", "fields.aspect_ratio.value_suffix_color", "")
               << config.getLabel("display_info", "fields.aspect_ratio.value_suffix", "") << r;
            lp.push(ss.str());
        }

        // ---------- Scaling ----------
        if (config.getNestedBool("display_info", "fields.scaling.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("display_info", "fields.scaling.scaling_prefix_color", "")
               << config.getPrefix("display_info", "fields.scaling.scaling_prefix", "") << r
               << config.getNestedColor("display_info", "fields.scaling.label_color", "")
               << config.getLabel("display_info", "fields.scaling.label", "") << r
               << config.getNestedColor("display_info", "fields.scaling.label_suffix_color", "")
               << config.getLabel("display_info", "fields.scaling.label_suffix", "") << r
               << config.getNestedColor("display_info", "fields.scaling.value_color", "")
               << s.scale_percent
               << config.getNestedColor("display_info", "fields.scaling.percent_color", "")
               << config.getLabel("display_info", "fields.scaling.percent_symbol", "%") << r
               << config.getNestedColor("display_info", "fields.scaling.value_suffix_color", "")
               << config.getLabel("display_info", "fields.scaling.value_suffix", "") << r;
            lp.push(ss.str());
        }

        // ---------- Upscale ----------
        if (config.getNestedBool("display_info", "fields.upscale.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("display_info", "fields.upscale.upscale_prefix_color", "")
               << config.getPrefix("display_info", "fields.upscale.upscale_prefix", "") << r
               << config.getNestedColor("display_info", "fields.upscale.label_color", "")
               << config.getLabel("display_info", "fields.upscale.label", "") << r
               << config.getNestedColor("display_info", "fields.upscale.label_suffix_color", "")
               << config.getLabel("display_info", "fields.upscale.label_suffix", "") << r
               << config.getNestedColor("display_info", "fields.upscale.value_color", "")
               << s.upscale << r
               << config.getNestedColor("display_info", "fields.upscale.value_suffix_color", "")
               << config.getLabel("display_info", "fields.upscale.value_suffix", "") << r;
            lp.push(ss.str());
        }

        // ---------- DSR / VSR ----------
        if (config.getNestedBool("display_info", "fields.dsr.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("display_info", "fields.dsr.dsr_prefix_color", "")
               << config.getPrefix("display_info", "fields.dsr.dsr_prefix", "") << r
               << config.getNestedColor("display_info", "fields.dsr.label_color", "")
               << config.getLabel("display_info", "fields.dsr.label", "") << r
               << config.getNestedColor("display_info", "fields.dsr.label_suffix_color", "")
               << config.getLabel("display_info", "fields.dsr.label_suffix", "") << r
               << config.getNestedColor(
                    "display_info",
                    s.dsr_enabled ? "fields.dsr.enabled_color" : "fields.dsr.disabled_color",
                    ""
                  )
               << config.getLabel(
                    "display_info",
                    s.dsr_enabled ? "fields.dsr.enabled_text" : "fields.dsr.disabled_text",
                    s.dsr_enabled ? "Enabled" : "Disabled"
                  ) << r
               << config.getNestedColor("display_info", "fields.dsr.brackets_color", "")
               << config.getLabel("display_info", "fields.dsr.bracket_open", " (") << r
               << config.getNestedColor("display_info", "fields.dsr.type_color", "")
               << s.dsr_type
               << config.getNestedColor("display_info", "fields.dsr.brackets_color", "")
               << config.getLabel("display_info", "fields.dsr.bracket_close", ")") << r
               << config.getNestedColor("display_info", "fields.dsr.value_suffix_color", "")
               << config.getLabel("display_info", "fields.dsr.value_suffix", "") << r;
            lp.push(ss.str());
        }

        lp.push("");
    }
}


// ============================================================================
//  ██████╗ ██╗ ██████╗ ███████╗    ██╗███╗   ██╗███████╗ ██████╗ 
//  ██╔══██╗██║██╔═══██╗██╔════╝    ██║████╗  ██║██╔════╝██╔═══██╗
//  ██████╔╝██║██║   ██║███████╗    ██║██╔██╗ ██║█████╗  ██║   ██║
//  ██╔══██╗██║██║   ██║╚════██║    ██║██║╚██╗██║██╔══╝  ██║   ██║
//  ██████╔╝██║╚██████╔╝███████║    ██║██║ ╚████║██║     ╚██████╔╝
//  ╚═════╝ ╚═╝ ╚═════╝ ╚══════╝    ╚═╝╚═╝  ╚═══╝╚═╝      ╚═════╝ 
// ============================================================================
//              D E T A I L E D   B I O S   &   M O T H E R B O A R D
// ============================================================================
//  This section displays comprehensive BIOS and motherboard information:
//  1. Bios Vendor           - Manufacturer of the system BIOS/UEFI
//  2. Bios Version          - Installed BIOS/UEFI version string
//  3. Bios Date             - Release date of the installed BIOS
//  4. Motherboard Model     - Model identifier of the motherboard
//  5. Motherboard Manufacturer - Motherboard vendor/brand
//  All labels, values, colors, prefixes, and toggles are fully JSON-driven
//  via the "detailed_bios_and_motherboard" config block (aliased as "bios_mb_info").
// ============================================================================
//
//  Output Example:
//  #- BIOS & Motherboard Info ----------------------------------------#
//  ~ Bios Vendor             : American Megatrends Inc.
//  ~ Bios Version            : F5
//  ~ Bios Date               : 2024-03-12
//  ~ Motherboard Model       : ROG STRIX B650E-F
//  ~ Motherboard Manufacturer: ASUSTeK COMPUTER INC.
// ============================================================================
if (config.isEnabled("bios_mb_info")) {
    lp.push("");

    // ---------- HEADER ----------
    if (config.getNestedBool("bios_mb_info", "header.show", true)) {
        ostringstream ss;
        ss << config.getNestedColor("bios_mb_info", "header.prefix_color", "")
           << config.getPrefix("bios_mb_info", "header.prefix", "") << r
           << config.getNestedColor("bios_mb_info", "header.text_color", "")
           << config.getLabel("bios_mb_info", "header.text", "") << r
           << config.getNestedColor("bios_mb_info", "header.suffix_color", "")
           << config.getPrefix("bios_mb_info", "header.suffix", "") << r;
        lp.push(ss.str());
    }

    // ---------- BIOS VENDOR ----------
    if (config.getNestedBool("bios_mb_info", "fields.bios_vendor.show", true)) {
        ostringstream ss;
        ss << config.getNestedColor("bios_mb_info", "fields.bios_vendor.bios_vendor_prefix_color", "")
           << config.getPrefix("bios_mb_info", "fields.bios_vendor.bios_vendor_prefix", "") << r
           << config.getNestedColor("bios_mb_info", "fields.bios_vendor.label_color", "")
           << config.getLabel("bios_mb_info", "fields.bios_vendor.label", "") << r
           << config.getNestedColor("bios_mb_info", "fields.bios_vendor.label_suffix_color", "")
           << config.getLabel("bios_mb_info", "fields.bios_vendor.label_suffix", "") << r
           << config.getNestedColor("bios_mb_info", "fields.bios_vendor.value_color", "")
           << sys.get_bios_vendor() << r
           << config.getNestedColor("bios_mb_info", "fields.bios_vendor.value_suffix_color", "")
           << config.getLabel("bios_mb_info", "fields.bios_vendor.value_suffix", "") << r;
        lp.push(ss.str());
    }

    // ---------- BIOS VERSION ----------
    if (config.getNestedBool("bios_mb_info", "fields.bios_version.show", true)) {
        ostringstream ss;
        ss << config.getNestedColor("bios_mb_info", "fields.bios_version.bios_version_prefix_color", "")
           << config.getPrefix("bios_mb_info", "fields.bios_version.bios_version_prefix", "") << r
           << config.getNestedColor("bios_mb_info", "fields.bios_version.label_color", "")
           << config.getLabel("bios_mb_info", "fields.bios_version.label", "") << r
           << config.getNestedColor("bios_mb_info", "fields.bios_version.label_suffix_color", "")
           << config.getLabel("bios_mb_info", "fields.bios_version.label_suffix", "") << r
           << config.getNestedColor("bios_mb_info", "fields.bios_version.value_color", "")
           << sys.get_bios_version() << r
           << config.getNestedColor("bios_mb_info", "fields.bios_version.value_suffix_color", "")
           << config.getLabel("bios_mb_info", "fields.bios_version.value_suffix", "") << r;
        lp.push(ss.str());
    }

    // ---------- BIOS DATE ----------
    if (config.getNestedBool("bios_mb_info", "fields.bios_date.show", true)) {
        ostringstream ss;
        ss << config.getNestedColor("bios_mb_info", "fields.bios_date.bios_date_prefix_color", "")
           << config.getPrefix("bios_mb_info", "fields.bios_date.bios_date_prefix", "") << r
           << config.getNestedColor("bios_mb_info", "fields.bios_date.label_color", "")
           << config.getLabel("bios_mb_info", "fields.bios_date.label", "") << r
           << config.getNestedColor("bios_mb_info", "fields.bios_date.label_suffix_color", "")
           << config.getLabel("bios_mb_info", "fields.bios_date.label_suffix", "") << r
           << config.getNestedColor("bios_mb_info", "fields.bios_date.value_color", "")
           << sys.get_bios_date() << r
           << config.getNestedColor("bios_mb_info", "fields.bios_date.value_suffix_color", "")
           << config.getLabel("bios_mb_info", "fields.bios_date.value_suffix", "") << r;
        lp.push(ss.str());
    }

    // ---------- MOTHERBOARD MODEL ----------
    if (config.getNestedBool("bios_mb_info", "fields.mb_model.show", true)) {
        ostringstream ss;
        ss << config.getNestedColor("bios_mb_info", "fields.mb_model.mb_model_prefix_color", "")
           << config.getPrefix("bios_mb_info", "fields.mb_model.mb_model_prefix", "") << r
           << config.getNestedColor("bios_mb_info", "fields.mb_model.label_color", "")
           << config.getLabel("bios_mb_info", "fields.mb_model.label", "") << r
           << config.getNestedColor("bios_mb_info", "fields.mb_model.label_suffix_color", "")
           << config.getLabel("bios_mb_info", "fields.mb_model.label_suffix", "") << r
           << config.getNestedColor("bios_mb_info", "fields.mb_model.value_color", "")
           << sys.get_motherboard_model() << r
           << config.getNestedColor("bios_mb_info", "fields.mb_model.value_suffix_color", "")
           << config.getLabel("bios_mb_info", "fields.mb_model.value_suffix", "") << r;
        lp.push(ss.str());
    }

    // ---------- MOTHERBOARD MANUFACTURER ----------
    if (config.getNestedBool("bios_mb_info", "fields.mb_manufacturer.show", true)) {
        ostringstream ss;
        ss << config.getNestedColor("bios_mb_info", "fields.mb_manufacturer.mb_manufacturer_prefix_color", "")
           << config.getPrefix("bios_mb_info", "fields.mb_manufacturer.mb_manufacturer_prefix", "") << r
           << config.getNestedColor("bios_mb_info", "fields.mb_manufacturer.label_color", "")
           << config.getLabel("bios_mb_info", "fields.mb_manufacturer.label", "") << r
           << config.getNestedColor("bios_mb_info", "fields.mb_manufacturer.label_suffix_color", "")
           << config.getLabel("bios_mb_info", "fields.mb_manufacturer.label_suffix", "") << r
           << config.getNestedColor("bios_mb_info", "fields.mb_manufacturer.value_color", "")
           << sys.get_motherboard_manufacturer() << r
           << config.getNestedColor("bios_mb_info", "fields.mb_manufacturer.value_suffix_color", "")
           << config.getLabel("bios_mb_info", "fields.mb_manufacturer.value_suffix", "") << r;
        lp.push(ss.str());
    }
}


// ============================================================================
//  ██╗   ██╗███████╗███████╗██████╗     ██╗███╗   ██╗███████╗ ██████╗ 
//  ██║   ██║██╔════╝██╔════╝██╔══██╗    ██║████╗  ██║██╔════╝██╔═══██╗
//  ██║   ██║███████╗█████╗  ██████╔╝    ██║██╔██╗ ██║█████╗  ██║   ██║
//  ██║   ██║╚════██║██╔══╝  ██╔══██╗    ██║██║╚██╗██║██╔══╝  ██║   ██║
//  ╚██████╔╝███████║███████╗██║  ██║    ██║██║ ╚████║██║     ╚██████╔╝
//   ╚═════╝ ╚══════╝╚══════╝╚═╝  ╚═╝    ╚═╝╚═╝  ╚═══╝╚═╝      ╚═════╝ 
// ============================================================================
//                      D E T A I L E D   U S E R   A C C O U N T
// ============================================================================
//  This section displays comprehensive user account information:
//  1. Username           - The currently logged-in user's account name
//  2. Computer Name      - The hostname of the machine
//  3. Domain             - The Windows domain or workgroup the PC belongs to
//  All labels, values, colors, prefixes, and toggles are fully JSON-driven
//  via the "detailed_user_account" config block (aliased as "user_info").
// ============================================================================
//
//  Output Example:
//  #- User Info ------------------------------------------------------#
//  ~ Username              : JohnDoe
//  ~ Computer Name         : DESKTOP-4X9K2P1
//  ~ Domain                : WORKGROUP
// ============================================================================
if (config.isEnabled("user_info")) {
    lp.push("");

    // ---------- HEADER ----------
    if (config.getNestedBool("user_info", "header.show", true)) {
        ostringstream ss;
        ss << config.getNestedColor("user_info", "header.prefix_color", "")
           << config.getPrefix("user_info", "header.prefix", "") << r
           << config.getNestedColor("user_info", "header.text_color", "")
           << config.getLabel("user_info", "header.text", "") << r
           << config.getNestedColor("user_info", "header.suffix_color", "")
           << config.getPrefix("user_info", "header.suffix", "") << r;
        lp.push(ss.str());
    }

    // ---------- USERNAME ----------
    if (config.getNestedBool("user_info", "fields.username.show", true)) {
        ostringstream ss;
        ss << config.getNestedColor("user_info", "fields.username.username_prefix_color", "")
           << config.getPrefix("user_info", "fields.username.username_prefix", "") << r
           << config.getNestedColor("user_info", "fields.username.label_color", "")
           << config.getLabel("user_info", "fields.username.label", "") << r
           << config.getNestedColor("user_info", "fields.username.label_suffix_color", "")
           << config.getLabel("user_info", "fields.username.label_suffix", "") << r
           << config.getNestedColor("user_info", "fields.username.value_color", "")
           << user.get_username() << r
           << config.getNestedColor("user_info", "fields.username.value_suffix_color", "")
           << config.getLabel("user_info", "fields.username.value_suffix", "") << r;
        lp.push(ss.str());
    }

    // ---------- COMPUTER NAME ----------
    if (config.getNestedBool("user_info", "fields.computer_name.show", true)) {
        ostringstream ss;
        ss << config.getNestedColor("user_info", "fields.computer_name.computer_name_prefix_color", "")
           << config.getPrefix("user_info", "fields.computer_name.computer_name_prefix", "") << r
           << config.getNestedColor("user_info", "fields.computer_name.label_color", "")
           << config.getLabel("user_info", "fields.computer_name.label", "") << r
           << config.getNestedColor("user_info", "fields.computer_name.label_suffix_color", "")
           << config.getLabel("user_info", "fields.computer_name.label_suffix", "") << r
           << config.getNestedColor("user_info", "fields.computer_name.value_color", "")
           << user.get_computer_name() << r
           << config.getNestedColor("user_info", "fields.computer_name.value_suffix_color", "")
           << config.getLabel("user_info", "fields.computer_name.value_suffix", "") << r;
        lp.push(ss.str());
    }

    // ---------- DOMAIN ----------
    if (config.getNestedBool("user_info", "fields.domain.show", true)) {
        ostringstream ss;
        ss << config.getNestedColor("user_info", "fields.domain.domain_prefix_color", "")
           << config.getPrefix("user_info", "fields.domain.domain_prefix", "") << r
           << config.getNestedColor("user_info", "fields.domain.label_color", "")
           << config.getLabel("user_info", "fields.domain.label", "") << r
           << config.getNestedColor("user_info", "fields.domain.label_suffix_color", "")
           << config.getLabel("user_info", "fields.domain.label_suffix", "") << r
           << config.getNestedColor("user_info", "fields.domain.value_color", "")
           << user.get_domain_name() << r
           << config.getNestedColor("user_info", "fields.domain.value_suffix_color", "")
           << config.getLabel("user_info", "fields.domain.value_suffix", "") << r;
        lp.push(ss.str());
    }
}

// ============================================================================
//  ██████╗ ███████╗██████╗ ███████╗ ██████╗ ██████╗ ███╗   ███╗ █████╗ ███╗   ██╗ ██████╗███████╗
//  ██╔══██╗██╔════╝██╔══██╗██╔════╝██╔═══██╗██╔══██╗████╗ ████║██╔══██╗████╗  ██║██╔════╝██╔════╝
//  ██████╔╝█████╗  ██████╔╝█████╗  ██║   ██║██████╔╝██╔████╔██║███████║██╔██╗ ██║██║     █████╗  
//  ██╔═══╝ ██╔══╝  ██╔══██╗██╔══╝  ██║   ██║██╔══██╗██║╚██╔╝██║██╔══██║██║╚██╗██║██║     ██╔══╝  
//  ██║     ███████╗██║  ██║██║     ╚██████╔╝██║  ██║██║ ╚═╝ ██║██║  ██║██║ ╚████║╚██████╗███████╗
//  ╚═╝     ╚══════╝╚═╝  ╚═╝╚═╝      ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝ ╚═════╝╚══════╝
// ============================================================================
//                       D E T A I L E D   P E R F O R M A N C E
// ============================================================================
//  This section displays real-time system performance metrics:
//  1. System Uptime      - Time elapsed since the last system boot
//  2. CPU Usage          - Current processor utilization percentage
//  3. RAM Usage           - Current memory utilization percentage
//  4. Disk Usage          - Current storage utilization percentage
//  5. GPU Usage           - Current graphics card utilization percentage
//  All labels, values, colors, prefixes, and toggles are fully JSON-driven
//  via the "detailed_resource_usage" config block (aliased as "performance_info").
// ============================================================================
//
//  Output Example:
//  #- Performance Info -----------------------------------------------#
//  ~ System Uptime          : 3d 4h 12m
//  ~ CPU Usage              : 12%
//  ~ RAM Usage              : 47%
//  ~ Disk Usage             : 68%
//  ~ GPU Usage              : 8%
// ============================================================================

// Performance Info (JSON Driven)
if (config.isEnabled("performance_info")) {
    lp.push("");

    // ---------- HEADER ----------
    if (config.getNestedBool("performance_info", "header.show", true)) {
        ostringstream ss;
        ss << config.getNestedColor("performance_info", "header.prefix_color", "")
           << config.getLabel("performance_info", "header.prefix", "") << r
           << config.getNestedColor("performance_info", "header.text_color", "")
           << config.getLabel("performance_info", "header.text", "") << r
           << config.getNestedColor("performance_info", "header.suffix_color", "")
           << config.getLabel("performance_info", "header.suffix", "") << r;
        lp.push(ss.str());
    }

    // ---------- SYSTEM UPTIME ----------
    if (config.getNestedBool("performance_info", "fields.uptime.show", true)) {
        ostringstream ss;
        ss << config.getNestedColor("performance_info", "fields.uptime.uptime_prefix_color", "")
           << config.getLabel("performance_info", "fields.uptime.uptime_prefix", "") << r
           << config.getNestedColor("performance_info", "fields.uptime.label_color", "")
           << config.getLabel("performance_info", "fields.uptime.label", "") << r
           << config.getNestedColor("performance_info", "fields.uptime.label_suffix_color", "")
           << config.getLabel("performance_info", "fields.uptime.label_suffix", "") << r
           << config.getNestedColor("performance_info", "fields.uptime.value_color", "")
           << perf.get_system_uptime() << r
           << config.getNestedColor("performance_info", "fields.uptime.value_suffix_color", "")
           << config.getLabel("performance_info", "fields.uptime.value_suffix", "") << r;
        lp.push(ss.str());
    }

    // ---------- CPU USAGE ----------
    if (config.getNestedBool("performance_info", "fields.cpu_usage.show", true)) {
        ostringstream ss;
        ss << config.getNestedColor("performance_info", "fields.cpu_usage.cpu_usage_prefix_color", "")
           << config.getLabel("performance_info", "fields.cpu_usage.cpu_usage_prefix", "") << r
           << config.getNestedColor("performance_info", "fields.cpu_usage.label_color", "")
           << config.getLabel("performance_info", "fields.cpu_usage.label", "") << r
           << config.getNestedColor("performance_info", "fields.cpu_usage.label_suffix_color", "")
           << config.getLabel("performance_info", "fields.cpu_usage.label_suffix", "") << r
           << config.getNestedColor("performance_info", "fields.cpu_usage.value_color", "")
           << perf.get_cpu_usage_percent() << r
           << config.getNestedColor("performance_info", "fields.cpu_usage.value_suffix_color", "")
           << config.getLabel("performance_info", "fields.cpu_usage.value_suffix", "") << r;
        lp.push(ss.str());
    }

    // ---------- RAM USAGE ----------
    if (config.getNestedBool("performance_info", "fields.ram_usage.show", true)) {
        ostringstream ss;
        ss << config.getNestedColor("performance_info", "fields.ram_usage.ram_usage_prefix_color", "")
           << config.getLabel("performance_info", "fields.ram_usage.ram_usage_prefix", "") << r
           << config.getNestedColor("performance_info", "fields.ram_usage.label_color", "")
           << config.getLabel("performance_info", "fields.ram_usage.label", "") << r
           << config.getNestedColor("performance_info", "fields.ram_usage.label_suffix_color", "")
           << config.getLabel("performance_info", "fields.ram_usage.label_suffix", "") << r
           << config.getNestedColor("performance_info", "fields.ram_usage.value_color", "")
           << perf.get_ram_usage_percent() << r
           << config.getNestedColor("performance_info", "fields.ram_usage.value_suffix_color", "")
           << config.getLabel("performance_info", "fields.ram_usage.value_suffix", "") << r;
        lp.push(ss.str());
    }

    // ---------- DISK USAGE ----------
    if (config.getNestedBool("performance_info", "fields.disk_usage.show", true)) {
        ostringstream ss;
        ss << config.getNestedColor("performance_info", "fields.disk_usage.disk_usage_prefix_color", "")
           << config.getLabel("performance_info", "fields.disk_usage.disk_usage_prefix", "") << r
           << config.getNestedColor("performance_info", "fields.disk_usage.label_color", "")
           << config.getLabel("performance_info", "fields.disk_usage.label", "") << r
           << config.getNestedColor("performance_info", "fields.disk_usage.label_suffix_color", "")
           << config.getLabel("performance_info", "fields.disk_usage.label_suffix", "") << r
           << config.getNestedColor("performance_info", "fields.disk_usage.value_color", "")
           << perf.get_disk_usage_percent() << r
           << config.getNestedColor("performance_info", "fields.disk_usage.value_suffix_color", "")
           << config.getLabel("performance_info", "fields.disk_usage.value_suffix", "") << r;
        lp.push(ss.str());
    }

    // ---------- GPU USAGE ----------
    if (config.getNestedBool("performance_info", "fields.gpu_usage.show", true)) {
        ostringstream ss;
        ss << config.getNestedColor("performance_info", "fields.gpu_usage.gpu_usage_prefix_color", "")
           << config.getLabel("performance_info", "fields.gpu_usage.gpu_usage_prefix", "") << r
           << config.getNestedColor("performance_info", "fields.gpu_usage.label_color", "")
           << config.getLabel("performance_info", "fields.gpu_usage.label", "") << r
           << config.getNestedColor("performance_info", "fields.gpu_usage.label_suffix_color", "")
           << config.getLabel("performance_info", "fields.gpu_usage.label_suffix", "") << r
           << config.getNestedColor("performance_info", "fields.gpu_usage.value_color", "")
           << perf.get_gpu_usage_percent() << r
           << config.getNestedColor("performance_info", "fields.gpu_usage.value_suffix_color", "")
           << config.getLabel("performance_info", "fields.gpu_usage.value_suffix", "") << r;
        lp.push(ss.str());
    }
}

// ============================================================================
//   █████╗ ██╗   ██╗██████╗ ██╗ ██████╗     █████╗     ██████╗  ██████╗ ██╗    ██╗███████╗██████╗ 
//  ██╔══██╗██║   ██║██╔══██╗██║██╔═══██╗   ██╔══██╗    ██╔══██╗██╔═══██╗██║    ██║██╔════╝██╔══██╗
//  ███████║██║   ██║██║  ██║██║██║   ██║   ███████║    ██████╔╝██║   ██║██║ █╗ ██║█████╗  ██████╔╝
//  ██╔══██║██║   ██║██║  ██║██║██║   ██║   ██╔══██║    ██╔═══╝ ██║   ██║██║███╗██║██╔══╝  ██╔══██╗
//  ██║  ██║╚██████╔╝██████╔╝██║╚██████╔╝   ██║  ██║    ██║     ╚██████╔╝╚███╔███╔╝███████╗██║  ██║
//  ╚═╝  ╚═╝ ╚═════╝ ╚═════╝ ╚═╝ ╚═════╝    ╚═╝  ╚═╝    ╚═╝      ╚═════╝  ╚══╝╚══╝ ╚══════╝╚═╝  ╚═╝
// ============================================================================
//                   D E T A I L E D   A U D I O   &   P O W E R
// ============================================================================
//  This section displays audio device and power/battery information:
//  1. Audio Output Devices - List of active/available playback devices
//  2. Audio Input Devices  - List of active/available recording devices
//  3. Power Status         - Wired connection or battery percentage/charging state
//  All labels, values, colors, prefixes, and toggles are fully JSON-driven
//  via the "detailed_audio_and_power" config block (aliased as "audio_power_info").
// ============================================================================
//
//  Output Example:
//  #- Audio Output -----------------------------------------------------#
//  ~ 1 Speakers (Realtek High Definition Audio) (active)
//  #- Audio Input ------------------------------------------------------#
//  ~ 1 Microphone Array (Realtek High Definition Audio) (active)
//  #- Power  -------------------------------------------------------------#
//  ~ Battery powered (87%) (Charging)
// ============================================================================

// Audio & Power Info (JSON Driven)
if (config.isEnabled("audio_power_info")) {
    lp.push("");
    ExtraInfo audio;

    // ---------- OUTPUT DEVICES ----------
    if (config.getNestedBool("audio_power_info", "output.show", true)) {
        vector<AudioDevice> outputDevices = audio.get_output_devices();

        ostringstream ss;
        ss << config.getNestedColor("audio_power_info", "output.header.prefix_color", "")
           << config.getLabel("audio_power_info", "output.header.prefix", "") << r
           << config.getNestedColor("audio_power_info", "output.header.text_color", "")
           << config.getLabel("audio_power_info", "output.header.text", "") << r
           << config.getNestedColor("audio_power_info", "output.header.suffix_color", "")
           << config.getLabel("audio_power_info", "output.header.suffix", "") << r;
        lp.push(ss.str());

        int audio_output_device_count = 0;
        for (const auto& device : outputDevices) {
            audio_output_device_count++;
            ostringstream oss;
            oss << config.getNestedColor("audio_power_info", "output.fields.device.prefix_color", "")
                << config.getLabel("audio_power_info", "output.fields.device.prefix", "") << r
                << config.getNestedColor("audio_power_info", "output.fields.device.index_color", "")
                << audio_output_device_count << r << " "
                << config.getNestedColor("audio_power_info", "output.fields.device.name_color", "")
                << device.name << r
                << config.getNestedColor("audio_power_info", "output.fields.device.value_suffix_color", "")
                << config.getLabel("audio_power_info", "output.fields.device.value_suffix", "") << r;

            if (device.isActive) {
                oss << " " << config.getNestedColor("audio_power_info", "output.fields.device.active_label_color", "")
                    << config.getLabel("audio_power_info", "output.fields.device.active_label", "") << r;
            }
            lp.push(oss.str());
        }
    }

    // ---------- INPUT DEVICES ----------
    if (config.getNestedBool("audio_power_info", "input.show", true)) {
        vector<AudioDevice> inputDevices = audio.get_input_devices();

        ostringstream ss;
        ss << config.getNestedColor("audio_power_info", "input.header.prefix_color", "")
           << config.getLabel("audio_power_info", "input.header.prefix", "") << r
           << config.getNestedColor("audio_power_info", "input.header.text_color", "")
           << config.getLabel("audio_power_info", "input.header.text", "") << r
           << config.getNestedColor("audio_power_info", "input.header.suffix_color", "")
           << config.getLabel("audio_power_info", "input.header.suffix", "") << r;
        lp.push(ss.str());

        int audio_input_device_count = 0;
        for (const auto& device : inputDevices) {
            audio_input_device_count++;
            ostringstream oss;
            oss << config.getNestedColor("audio_power_info", "input.fields.device.prefix_color", "")
                << config.getLabel("audio_power_info", "input.fields.device.prefix", "") << r
                << config.getNestedColor("audio_power_info", "input.fields.device.index_color", "")
                << audio_input_device_count << r << " "
                << config.getNestedColor("audio_power_info", "input.fields.device.name_color", "")
                << device.name << r
                << config.getNestedColor("audio_power_info", "input.fields.device.value_suffix_color", "")
                << config.getLabel("audio_power_info", "input.fields.device.value_suffix", "") << r;

            if (device.isActive) {
                oss << " " << config.getNestedColor("audio_power_info", "input.fields.device.active_label_color", "")
                    << config.getLabel("audio_power_info", "input.fields.device.active_label", "") << r;
            }
            lp.push(oss.str());
        }
    }

    // ---------- POWER STATUS ----------
    if (config.getNestedBool("audio_power_info", "power.show", true)) {
        lp.push("");
        PowerStatus power = audio.get_power_status();

        ostringstream ss;
        ss << config.getNestedColor("audio_power_info", "power.header.prefix_color", "")
           << config.getLabel("audio_power_info", "power.header.prefix", "") << r
           << config.getNestedColor("audio_power_info", "power.header.text_color", "")
           << config.getLabel("audio_power_info", "power.header.text", "") << r
           << config.getNestedColor("audio_power_info", "power.header.suffix_color", "")
           << config.getLabel("audio_power_info", "power.header.suffix", "") << r;
        lp.push(ss.str());

        ostringstream ossPower;
        if (!power.hasBattery) {
            ossPower << config.getNestedColor("audio_power_info", "power.fields.wired.brackets_color", "")
                      << config.getLabel("audio_power_info", "power.fields.wired.bracket_open", "") << r
                      << config.getNestedColor("audio_power_info", "power.fields.wired.text_color", "")
                      << config.getLabel("audio_power_info", "power.fields.wired.text", "") << r
                      << config.getNestedColor("audio_power_info", "power.fields.wired.value_suffix_color", "")
                      << config.getLabel("audio_power_info", "power.fields.wired.value_suffix", "") << r
                      << config.getNestedColor("audio_power_info", "power.fields.wired.brackets_color", "")
                      << config.getLabel("audio_power_info", "power.fields.wired.bracket_close", "") << r;
        }
        else {
            ossPower << config.getNestedColor("audio_power_info", "power.fields.battery.prefix_color", "")
                      << config.getLabel("audio_power_info", "power.fields.battery.prefix", "") << r
                      << config.getNestedColor("audio_power_info", "power.fields.battery.label_color", "")
                      << config.getLabel("audio_power_info", "power.fields.battery.label", "") << r
                      << config.getNestedColor("audio_power_info", "power.fields.battery.label_suffix_color", "")
                      << config.getLabel("audio_power_info", "power.fields.battery.label_suffix", "") << r
                      << config.getNestedColor("audio_power_info", "power.fields.battery.brackets_color", "")
                      << config.getLabel("audio_power_info", "power.fields.battery.bracket_open", "") << r
                      << config.getNestedColor("audio_power_info", "power.fields.battery.value_color", "")
                      << power.batteryPercent << r
                      << config.getNestedColor("audio_power_info", "power.fields.battery.value_suffix_color", "")
                      << config.getLabel("audio_power_info", "power.fields.battery.value_suffix", "") << r
                      << config.getNestedColor("audio_power_info", "power.fields.battery.brackets_color", "")
                      << config.getLabel("audio_power_info", "power.fields.battery.bracket_close", "") << r;

            if (power.isCharging) {
                ossPower << " " << config.getNestedColor("audio_power_info", "power.fields.battery.charging_color", "")
                          << config.getLabel("audio_power_info", "power.fields.battery.charging_text", "") << r;
            }
            else {
                ossPower << " " << config.getNestedColor("audio_power_info", "power.fields.battery.not_charging_color", "")
                          << config.getLabel("audio_power_info", "power.fields.battery.not_charging_text", "") << r;
            }
        }
        lp.push(ossPower.str());
    }
}















 


    // Print remaining ASCII art lines (if art is taller than info)
    lp.finish();

    cout << endl;






    return 0;
}


