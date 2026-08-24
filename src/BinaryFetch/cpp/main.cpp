

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



// ------------------ main (modified to stream output) ------------------
/*
Before you try to implement modifications inside the main() function:

- main() is intended to act as an orchestrator/controller, not a logic container.
- Avoid adding heavy logic, calculations, or system queries directly here.
- I designed main() to initialize components and control execution flow only.
- All feature logic should live inside their respective modules/classes.
- main() should only:
    - initialize components
    - control execution order  
    - format and stream output using LivePrinter (lp.push)

Why this matters:
- Keeps the codebase maintainable as the project scales
- Prevents main.cpp from becoming unreadable and error-prone
- Makes testing, refactoring, and debugging significantly easier

If you feel the need to add complex logic here,
it is a sign that the logic should be moved into a new module.
*/


//Initialize Global Variables (if any) here ------ (start)

//Initialize Global Variables (if any) here ------ (end)

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
        ss << config.getColor("compact_date_and_time", "prefixes.prefix_color", "white")
           << config.getPrefix("compact_date_and_time", "prefixes.prefix", "") << r;
    }

    // ---------- TIME SECTION ----------
    if (config.isNestedEnabled("compact_date_and_time", "time", "enabled")) {
        ss << config.getNestedColor("compact_date_and_time", "time", "bracket", "white") << "(" << r;

        if (config.isNestedEnabled("compact_date_and_time", "time", "show_label")) {
            ss << config.getNestedColor("compact_date_and_time", "time", "label", "white") << "Time: " << r;
        }

        bool wrote = false;

        if (config.isNestedEnabled("compact_date_and_time", "time", "show_hour")) {
            ss << config.getNestedColor("compact_date_and_time", "time", "hour", "white")
               << setw(2) << setfill('0') << time.getHour() << r;
            wrote = true;
        }

        if (config.isNestedEnabled("compact_date_and_time", "time", "show_minute")) {
            if (wrote) ss << config.getNestedColor("compact_date_and_time", "time", "sep", "white") << ":" << r;
            ss << config.getNestedColor("compact_date_and_time", "time", "minute", "white")
               << setw(2) << setfill('0') << time.getMinute() << r;
            wrote = true;
        }

        if (config.isNestedEnabled("compact_date_and_time", "time", "show_second")) {
            if (wrote) ss << config.getNestedColor("compact_date_and_time", "time", "sep", "white") << ":" << r;
            ss << config.getNestedColor("compact_date_and_time", "time", "second", "white")
               << setw(2) << setfill('0') << time.getSecond() << r;
        }

        ss << config.getNestedColor("compact_date_and_time", "time", "bracket", "white") << ") " << r;
    }

    // ---------- DATE SECTION ----------
    if (config.isNestedEnabled("compact_date_and_time", "date", "enabled")) {
        ss << config.getNestedColor("compact_date_and_time", "date", "bracket", "white") << "(" << r;

        if (config.isNestedEnabled("compact_date_and_time", "date", "show_label")) {
            ss << config.getNestedColor("compact_date_and_time", "date", "label", "white") << "Date: " << r;
        }

        bool wrote = false;

        if (config.isNestedEnabled("compact_date_and_time", "date", "show_day")) {
            ss << config.getNestedColor("compact_date_and_time", "date", "day", "white")
               << setw(2) << setfill('0') << time.getDay() << r;
            wrote = true;
        }

        if (config.isNestedEnabled("compact_date_and_time", "date", "show_month_name")) {
            if (wrote) ss << config.getNestedColor("compact_date_and_time", "date", "sep", "white") << " : " << r;
            ss << config.getNestedColor("compact_date_and_time", "date", "month_name", "white")
               << time.getMonthName() << r;
            wrote = true;
        }

        if (config.isNestedEnabled("compact_date_and_time", "date", "show_month_num")) {
            if (wrote) ss << " ";
            ss << config.getNestedColor("compact_date_and_time", "date", "month_num", "white")
               << setw(2) << setfill('0') << time.getMonthNumber() << r;
            wrote = true;
        }

        if (config.isNestedEnabled("compact_date_and_time", "date", "show_year")) {
            if (wrote) ss << config.getNestedColor("compact_date_and_time", "date", "sep", "white") << " : " << r;
            ss << config.getNestedColor("compact_date_and_time", "date", "year", "white")
               << time.getYearNumber() << r;
        }

        ss << config.getNestedColor("compact_date_and_time", "date", "bracket", "white") << ") " << r;
    }

    // ---------- WEEK SECTION ----------
    if (config.isNestedEnabled("compact_date_and_time", "week", "enabled")) {
        ss << config.getNestedColor("compact_date_and_time", "week", "bracket", "white") << "(" << r;

        if (config.isNestedEnabled("compact_date_and_time", "week", "show_label")) {
            ss << config.getNestedColor("compact_date_and_time", "week", "label", "white") << "Week: " << r;
        }

        bool wrote = false;

        if (config.isNestedEnabled("compact_date_and_time", "week", "show_num")) {
            ss << config.getNestedColor("compact_date_and_time", "week", "num", "white")
               << time.getWeekNumber() << r;
            wrote = true;
        }

        if (config.isNestedEnabled("compact_date_and_time", "week", "show_day_name")) {
            if (wrote) ss << config.getNestedColor("compact_date_and_time", "week", "sep", "white") << " - " << r;
            ss << config.getNestedColor("compact_date_and_time", "week", "day_name", "white")
               << time.getDayName() << r;
        }

        ss << config.getNestedColor("compact_date_and_time", "week", "bracket", "white") << ") " << r;
    }

    // ---------- LEAP YEAR SECTION ----------
    if (config.isNestedEnabled("compact_date_and_time", "leap_year", "enabled")) {
        ss << config.getNestedColor("compact_date_and_time", "leap_year", "bracket", "white") << "(" << r;

        if (config.isNestedEnabled("compact_date_and_time", "leap_year", "show_label")) {
            ss << config.getNestedColor("compact_date_and_time", "leap_year", "label", "white") << "Leap Year: " << r;
        }

        if (config.isNestedEnabled("compact_date_and_time", "leap_year", "show_val")) {
            ss << config.getNestedColor("compact_date_and_time", "leap_year", "val", "white")
               << time.getLeapYear() << r;
        }

        ss << config.getNestedColor("compact_date_and_time", "leap_year", "bracket", "white") << ") " << r;
    }

    lp.push(ss.str());
}

// ==================== COMPACT OPERATING SYSTEM ====================
if (config.isEnabled("compact_operating_system")) {
    ostringstream ss;

    // Prefix - comes entirely from JSON (can be emoji, text, or empty)
    if (config.isFieldEnabled("compact_operating_system", "prefixes.show")) {
        ss << config.getColor("compact_operating_system", "prefixes.prefix_color", "white")
           << config.getPrefix("compact_operating_system", "prefixes.prefix", "") << r;
    }

    // Label
    ss << config.getColor("compact_operating_system", "label.color", "white")
       << config.getLabel("compact_operating_system", "label.text", "OS") << r;

    // Separator
    ss << config.getColor("compact_operating_system", "separator.color", "white")
       << config.getPrefix("compact_operating_system", "separator.text", ":") << " " << r;

    // Name field
    if (config.isFieldEnabled("compact_operating_system", "fields.name.show")) {
        ss << config.getColor("compact_operating_system", "fields.name.value_color", "white")
           << c_os.getOSName() << r << " ";
    }

    // Build field
    if (config.isFieldEnabled("compact_operating_system", "fields.build.show")) {
        ss << config.getColor("compact_operating_system", "fields.build.value_color", "white")
           << c_os.getOSBuild() << r;
    }

    // Architecture (with brackets)
    if (config.isFieldEnabled("compact_operating_system", "fields.arch.show")) {
        ss << config.getColor("compact_operating_system", "brackets.color", "white") << "(" << r
           << config.getColor("compact_operating_system", "fields.arch.value_color", "white")
           << c_os.getArchitecture() << r
           << config.getColor("compact_operating_system", "brackets.color", "white") << ")" << r;
    }

    // Uptime (with brackets)
    if (config.isFieldEnabled("compact_operating_system", "fields.uptime.show")) {
        ss << config.getColor("compact_operating_system", "brackets.color", "white") << "(" << r
           << config.getColor("compact_operating_system", "fields.uptime.label_color", "white") << "uptime: " << r
           << config.getColor("compact_operating_system", "fields.uptime.value_color", "white")
           << c_os.getUptime() << r
           << config.getColor("compact_operating_system", "brackets.color", "white") << ")" << r;
    }

    lp.push(ss.str());
}

// ==================== COMPACT CPU ====================
if (config.isEnabled("compact_processor")) {
    ostringstream ss;

    // Prefix - comes entirely from JSON (can be emoji, text, or empty)
    if (config.isFieldEnabled("compact_processor", "prefixes.show")) {
        ss << config.getColor("compact_processor", "prefixes.prefix_color", "white")
           << config.getPrefix("compact_processor", "prefixes.prefix", "") << r;
    }

    // Label
    ss << config.getColor("compact_processor", "label.color", "white")
       << config.getLabel("compact_processor", "label.text", "CPU") << r;

    // Separator
    ss << config.getColor("compact_processor", "separator.color", "white")
       << config.getPrefix("compact_processor", "separator.text", ":") << " " << r;

    // Name field
    if (config.isFieldEnabled("compact_processor", "fields.name.show")) {
        ss << config.getColor("compact_processor", "fields.name.value_color", "white")
           << c_cpu.getCPUName() << r;
    }

    // Cores and Threads (with brackets)
    if (config.isFieldEnabled("compact_processor", "fields.cores.show") ||
        config.isFieldEnabled("compact_processor", "fields.threads.show")) {
        ss << config.getColor("compact_processor", "brackets.color", "white") << "(" << r;

        if (config.isFieldEnabled("compact_processor", "fields.cores.show")) {
            ss << config.getColor("compact_processor", "fields.cores.value_color", "white")
               << c_cpu.getCPUCores() << r
               << config.getColor("compact_processor", "text_color", "white") << "C" << r;
        }

        if (config.isFieldEnabled("compact_processor", "fields.cores.show") &&
            config.isFieldEnabled("compact_processor", "fields.threads.show")) {
            ss << "/";
        }

        if (config.isFieldEnabled("compact_processor", "fields.threads.show")) {
            ss << config.getColor("compact_processor", "fields.threads.value_color", "white")
               << c_cpu.getCPUThreads() << r
               << config.getColor("compact_processor", "text_color", "white") << "T" << r;
        }

        ss << config.getColor("compact_processor", "brackets.color", "white") << ")" << r;
    }

    // Clock speed
    if (config.isFieldEnabled("compact_processor", "fields.clock.show")) {
        ss << fixed << setprecision(2)
           << config.getColor("compact_processor", "fields.clock.at_symbol_color", "white") << "@" << r
           << config.getColor("compact_processor", "fields.clock.value_color", "white") << " "
           << c_cpu.getClockSpeed() << " GHz" << r;
    }

    lp.push(ss.str());
}

// ==================== COMPACT GPU ====================
if (config.isEnabled("compact_graphics_card")) {
    ostringstream ss;

    // Prefix - comes entirely from JSON (can be emoji, text, or empty)
    if (config.isFieldEnabled("compact_graphics_card", "prefixes.show")) {
        ss << config.getColor("compact_graphics_card", "prefixes.prefix_color", "white")
           << config.getPrefix("compact_graphics_card", "prefixes.prefix", "") << r;
    }

    // Label
    ss << config.getColor("compact_graphics_card", "label.color", "white")
       << config.getLabel("compact_graphics_card", "label.text", "GPU") << r;

    // Separator
    ss << config.getColor("compact_graphics_card", "separator.color", "white")
       << config.getPrefix("compact_graphics_card", "separator.text", ":") << " " << r;

    // Name field
    if (config.isFieldEnabled("compact_graphics_card", "fields.name.show")) {
        ss << config.getColor("compact_graphics_card", "fields.name.value_color", "white")
           << c_gpu.getGPUName() << r;
    }

    // Usage (with brackets)
    if (config.isFieldEnabled("compact_graphics_card", "fields.usage.show")) {
        ss << config.getColor("compact_graphics_card", "brackets.color", "white") << "(" << r
           << config.getColor("compact_graphics_card", "fields.usage.value_color", "white")
           << c_gpu.getGPUUsagePercent() << "%" << r
           << config.getColor("compact_graphics_card", "brackets.color", "white") << ")" << r;
    }

    // VRAM (with brackets)
    if (config.isFieldEnabled("compact_graphics_card", "fields.vram.show")) {
        ss << config.getColor("compact_graphics_card", "brackets.color", "white") << "(" << r
           << config.getColor("compact_graphics_card", "fields.vram.value_color", "white")
           << c_gpu.getVRAMGB() << " GB" << r
           << config.getColor("compact_graphics_card", "brackets.color", "white") << ")" << r;
    }

    // Frequency (with brackets)
    if (config.isFieldEnabled("compact_graphics_card", "fields.freq.show")) {
        ss << config.getColor("compact_graphics_card", "brackets.color", "white") << "(" << r
           << config.getColor("compact_graphics_card", "fields.freq.at_symbol_color", "white") << "@" << r
           << config.getColor("compact_graphics_card", "fields.freq.value_color", "white")
           << c_gpu.getGPUFrequency() << r
           << config.getColor("compact_graphics_card", "brackets.color", "white") << ") " << r;
    }

    lp.push(ss.str());
}

// ==================== COMPACT DISPLAY ====================
if (config.isEnabled("compact_display_monitor")) {
    CompactScreen screenDetector;
    auto screens = screenDetector.getScreens();

    if (screens.empty()) {
        ostringstream ss;
        ss << config.getColor("compact_display_monitor", "header.text_color", "white")
           << config.getLabel("compact_display_monitor", "header.text", "Display") << r
           << config.getColor("compact_display_monitor", "header.separator_color", "white")
           << config.getPrefix("compact_display_monitor", "header.separator", ":") << " " << r
           << config.getColor("compact_display_monitor", "fields.name.value_color", "white")
           << "No displays detected" << r;
        lp.push(ss.str());
    } else {
        for (size_t i = 0; i < screens.size(); ++i) {
            const auto& screen = screens[i];
            ostringstream ss;

            // Prefix - comes entirely from JSON
            if (config.isFieldEnabled("compact_display_monitor", "prefixes.show")) {
                ss << config.getColor("compact_display_monitor", "prefixes.prefix_color", "white")
                   << config.getPrefix("compact_display_monitor", "prefixes.prefix", "") << r;
            }

            // Header: Display N:
            ss << config.getColor("compact_display_monitor", "header.text_color", "white")
               << config.getLabel("compact_display_monitor", "header.text", "Display") << " " << (i + 1) << r
               << config.getColor("compact_display_monitor", "header.separator_color", "white")
               << config.getPrefix("compact_display_monitor", "header.separator", ":") << " " << r;

            // Display name
            if (config.isFieldEnabled("compact_display_monitor", "fields.name.show")) {
                ss << config.getColor("compact_display_monitor", "fields.name.value_color", "white")
                   << screen.name << r << " ";
            }

            // Resolution: (3840 x 2160)
            if (config.isFieldEnabled("compact_display_monitor", "fields.resolution.show")) {
                ss << config.getColor("compact_display_monitor", "brackets.color", "white") << "(" << r
                   << config.getColor("compact_display_monitor", "fields.resolution.value_color", "white")
                   << screen.native_width << r
                   << config.getColor("compact_display_monitor", "fields.resolution.x_color", "white") << " x " << r
                   << config.getColor("compact_display_monitor", "fields.resolution.value_color", "white")
                   << screen.native_height << r
                   << config.getColor("compact_display_monitor", "brackets.color", "white") << ") " << r;
            }

            // Scale: (Scale: 175%)
            if (config.isFieldEnabled("compact_display_monitor", "fields.scale.show")) {
                ss << config.getColor("compact_display_monitor", "brackets.color", "white") << "(" << r
                   << config.getColor("compact_display_monitor", "fields.scale.label_color", "white") << "Scale: " << r
                   << config.getColor("compact_display_monitor", "fields.scale.value_color", "white")
                   << screen.scale_percent << "%" << r
                   << config.getColor("compact_display_monitor", "brackets.color", "white") << ") " << r;
            }

            // Upscale: (upscale: 4x)
            if (config.isFieldEnabled("compact_display_monitor", "fields.upscale.show")) {
                ss << config.getColor("compact_display_monitor", "brackets.color", "white") << "(" << r
                   << config.getColor("compact_display_monitor", "fields.upscale.label_color", "white") << "upscale: " << r
                   << config.getColor("compact_display_monitor", "fields.upscale.value_color", "white")
                   << screen.upscale << r
                   << config.getColor("compact_display_monitor", "brackets.color", "white") << ") " << r;
            }

            // Refresh rate: (@60Hz)
            if (config.isFieldEnabled("compact_display_monitor", "fields.refresh.show")) {
                ss << config.getColor("compact_display_monitor", "brackets.color", "white") << "(" << r
                   << config.getColor("compact_display_monitor", "fields.refresh.at_symbol_color", "white") << "@" << r
                   << config.getColor("compact_display_monitor", "fields.refresh.value_color", "white")
                   << screen.refresh_rate << "Hz" << r
                   << config.getColor("compact_display_monitor", "brackets.color", "white") << ")" << r;
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
        ss << config.getColor("compact_system_memory", "prefixes.prefix_color", "white")
           << config.getPrefix("compact_system_memory", "prefixes.prefix", "") << r;
    }

    // Label
    ss << config.getColor("compact_system_memory", "label.color", "white")
       << config.getLabel("compact_system_memory", "label.text", "Memory") << r;

    // Separator
    ss << config.getColor("compact_system_memory", "separator.color", "white")
       << config.getPrefix("compact_system_memory", "separator.text", ":") << " " << r;

    // Total memory (with brackets)
    if (config.isFieldEnabled("compact_system_memory", "fields.total.show")) {
        ss << config.getColor("compact_system_memory", "brackets.color", "white") << "(" << r
           << config.getColor("compact_system_memory", "fields.total.label_color", "white") << "total: " << r
           << config.getColor("compact_system_memory", "fields.total.value_color", "white")
           << c_memory.get_total_memory() << " GB" << r
           << config.getColor("compact_system_memory", "brackets.color", "white") << ")" << r;
    }

    // Free memory (with brackets)
    if (config.isFieldEnabled("compact_system_memory", "fields.free.show")) {
        ss << " " << config.getColor("compact_system_memory", "brackets.color", "white") << "(" << r
           << config.getColor("compact_system_memory", "fields.total.label_color", "white") << "free: " << r
           << config.getColor("compact_system_memory", "fields.free.value_color", "white")
           << c_memory.get_free_memory() << " GB" << r
           << config.getColor("compact_system_memory", "brackets.color", "white") << ")" << r;
    }

    // Used percentage (with brackets)
    if (config.isFieldEnabled("compact_system_memory", "fields.percent.show")) {
        ss << " " << config.getColor("compact_system_memory", "brackets.color", "white") << "(" << r
           << config.getColor("compact_system_memory", "fields.percent.value_color", "white")
           << c_memory.get_used_memory_percent() << "%" << r
           << config.getColor("compact_system_memory", "brackets.color", "white") << ")" << r;
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
            ss << config.getColor("compact_audio_devices", "input.prefixes.prefix_color", "white")
               << config.getPrefix("compact_audio_devices", "input.prefixes.prefix", "") << r;
        }

        // Input label
        ss << config.getColor("compact_audio_devices", "input.label.color", "white")
           << config.getLabel("compact_audio_devices", "input.label.text", "Audio Input") << r;

        // Input separator
        ss << config.getColor("compact_audio_devices", "input.separator.color", "white")
           << config.getPrefix("compact_audio_devices", "input.separator.text", ":") << " " << r;

        // Input device name
        ss << config.getColor("compact_audio_devices", "input.device_color", "white")
           << c_audio.active_audio_input() << r << " ";

        // Input status
        ss << config.getColor("compact_audio_devices", "brackets.color", "white") << "[" << r
           << config.getColor("compact_audio_devices", "input.status_color", "white")
           << c_audio.active_audio_input_status() << r
           << config.getColor("compact_audio_devices", "brackets.color", "white") << "]" << r;

        lp.push(ss.str());
    }

    // Output device
    if (config.isFieldEnabled("compact_audio_devices", "output.show")) {
        ostringstream ss;

        // Output prefix - from JSON
        if (config.isFieldEnabled("compact_audio_devices", "output.prefixes.show")) {
            ss << config.getColor("compact_audio_devices", "output.prefixes.prefix_color", "white")
               << config.getPrefix("compact_audio_devices", "output.prefixes.prefix", "") << r;
        }

        // Output label
        ss << config.getColor("compact_audio_devices", "output.label.color", "white")
           << config.getLabel("compact_audio_devices", "output.label.text", "Audio Output") << r;

        // Output separator
        ss << config.getColor("compact_audio_devices", "output.separator.color", "white")
           << config.getPrefix("compact_audio_devices", "output.separator.text", ":") << " " << r;

        // Output device name
        ss << config.getColor("compact_audio_devices", "output.device_color", "white")
           << c_audio.active_audio_output() << r << " ";

        // Output status
        ss << config.getColor("compact_audio_devices", "brackets.color", "white") << "[" << r
           << config.getColor("compact_audio_devices", "output.status_color", "white")
           << c_audio.active_audio_output_status() << r
           << config.getColor("compact_audio_devices", "brackets.color", "white") << "]" << r;

        lp.push(ss.str());
    }
}

// ==================== COMPACT PERFORMANCE ====================
if (config.isEnabled("compact_resource_usage")) {
    ostringstream ss;

    // Prefix - from JSON
    if (config.isFieldEnabled("compact_resource_usage", "prefixes.show")) {
        ss << config.getColor("compact_resource_usage", "prefixes.prefix_color", "white")
           << config.getPrefix("compact_resource_usage", "prefixes.prefix", "") << r;
    }

    // Label
    ss << config.getColor("compact_resource_usage", "label.color", "white")
       << config.getLabel("compact_resource_usage", "label.text", "Performance") << r;

    // Separator
    ss << config.getColor("compact_resource_usage", "separator.color", "white")
       << config.getPrefix("compact_resource_usage", "separator.text", ":") << " " << r;

    // Helper lambda for adding performance stats
    auto addPerf = [&](const string& field, const string& label, const string& colorKey, auto val) {
        if (config.isFieldEnabled("compact_resource_usage", "fields." + field + ".show")) {
            ss << config.getColor("compact_resource_usage", "brackets.color", "white") << "(" << r
               << config.getColor("compact_resource_usage", "fields." + field + ".label_color", "white")
               << label << ": " << r
               << config.getColor("compact_resource_usage", "fields." + field + ".value_color", "white")
               << val << "%" << r
               << config.getColor("compact_resource_usage", "brackets.color", "white") << ") " << r;
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
        ss << config.getColor("compact_user_account", "prefixes.prefix_color", "white")
           << config.getPrefix("compact_user_account", "prefixes.prefix", "") << r;
    }

    // Label
    ss << config.getColor("compact_user_account", "label.color", "white")
       << config.getLabel("compact_user_account", "label.text", "User") << r;

    // Separator
    ss << config.getColor("compact_user_account", "separator.color", "white")
       << config.getPrefix("compact_user_account", "separator.text", ":") << " " << r;

    // Username
    if (config.isFieldEnabled("compact_user_account", "fields.username.show")) {
        ss << config.getColor("compact_user_account", "fields.username.value_color", "white")
           << "@" << c_user.getUsername() << r;
    }

    // Domain (with brackets)
    if (config.isFieldEnabled("compact_user_account", "fields.domain.show")) {
        ss << " " << config.getColor("compact_user_account", "brackets.color", "white") << "(" << r
           << config.getColor("compact_user_account", "label_color", "white") << "Domain: " << r
           << config.getColor("compact_user_account", "fields.domain.value_color", "white")
           << c_user.getDomain() << r
           << config.getColor("compact_user_account", "brackets.color", "white") << ")" << r;
    }

    // Type (with brackets)
    if (config.isFieldEnabled("compact_user_account", "fields.type.show")) {
        ss << " " << config.getColor("compact_user_account", "brackets.color", "white") << "(" << r
           << config.getColor("compact_user_account", "label_color", "white") << "Type: " << r
           << config.getColor("compact_user_account", "fields.type.value_color", "white")
           << c_user.isAdmin() << r
           << config.getColor("compact_user_account", "brackets.color", "white") << ")" << r;
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
            ss << config.getColor("compact_network_connection", "prefixes.prefix_color", "white")
               << config.getPrefix("compact_network_connection", "prefixes.prefix", "") << r;
        }

        // Label
        ss << config.getColor("compact_network_connection", "label.color", "white")
           << config.getLabel("compact_network_connection", "label.text", "Network") << r;

        // Separator
        ss << config.getColor("compact_network_connection", "separator.color", "white")
           << config.getPrefix("compact_network_connection", "separator.text", ":") << " " << r;

        // Network Name (with brackets)
        if (config.isFieldEnabled("compact_network_connection", "fields.name.show")) {
            ss << config.getColor("compact_network_connection", "brackets.color", "white") << "(" << r
               << config.getColor("compact_network_connection", "label_color", "white") << "Name: " << r
               << config.getColor("compact_network_connection", "fields.name.value_color", "white")
               << c_net.get_network_name() << r
               << config.getColor("compact_network_connection", "brackets.color", "white") << ") " << r;
        }

        // Network Type (with brackets)
        if (config.isFieldEnabled("compact_network_connection", "fields.type.show")) {
            ss << config.getColor("compact_network_connection", "brackets.color", "white") << "(" << r
               << config.getColor("compact_network_connection", "label_color", "white") << "Type: " << r
               << config.getColor("compact_network_connection", "fields.type.value_color", "white")
               << c_net.get_network_type() << r
               << config.getColor("compact_network_connection", "brackets.color", "white") << ") " << r;
        }

        // IP Address (with brackets)
        if (config.isFieldEnabled("compact_network_connection", "fields.ip.show")) {
            ss << config.getColor("compact_network_connection", "brackets.color", "white") << "(" << r
               << config.getColor("compact_network_connection", "label_color", "white") << "ip: " << r
               << config.getColor("compact_network_connection", "fields.ip.value_color", "white")
               << c_net.get_network_ip() << r
               << config.getColor("compact_network_connection", "brackets.color", "white") << ")" << r;
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
            ss << config.getColor("compact_disk_storage", "usage.prefixes.prefix_color", "white")
               << config.getPrefix("compact_disk_storage", "usage.prefixes.prefix", "") << r;
        }

        // Usage label
        ss << config.getColor("compact_disk_storage", "usage.label.color", "white")
           << config.getLabel("compact_disk_storage", "usage.label.text", "Disk Usage") << r;

        // Usage separator
        ss << config.getColor("compact_disk_storage", "usage.separator.color", "white")
           << config.getPrefix("compact_disk_storage", "usage.separator.text", ":") << " " << r;

        for (const auto& d : disks) {
            ss << config.getColor("compact_disk_storage", "brackets.color", "white") << "(" << r
               << config.getColor("compact_disk_storage", "fields.letter_color", "white")
               << d.first[0] << ":" << r << " "
               << config.getColor("compact_disk_storage", "fields.percent_color", "white")
               << fixed << setprecision(1) << d.second << "%" << r
               << config.getColor("compact_disk_storage", "brackets.color", "white") << ") " << r;
        }
        lp.push(ss.str());
    }

    // Disk Capacity
    if (config.isFieldEnabled("compact_disk_storage", "capacity.show")) {
        auto caps = disk.getDiskCapacity();
        ostringstream sc;

        // Capacity prefix - from JSON
        if (config.isFieldEnabled("compact_disk_storage", "capacity.prefixes.show")) {
            sc << config.getColor("compact_disk_storage", "capacity.prefixes.prefix_color", "white")
               << config.getPrefix("compact_disk_storage", "capacity.prefixes.prefix", "") << r;
        }

        // Capacity label
        sc << config.getColor("compact_disk_storage", "capacity.label.color", "white")
           << config.getLabel("compact_disk_storage", "capacity.label.text", "Disk Cap") << r;

        // Capacity separator
        sc << config.getColor("compact_disk_storage", "capacity.separator.color", "white")
           << config.getPrefix("compact_disk_storage", "capacity.separator.text", ":") << " " << r;

        for (const auto& c : caps) {
            sc << config.getColor("compact_disk_storage", "brackets.color", "white") << "(" << r
               << config.getColor("compact_disk_storage", "fields.letter_color", "white")
               << c.first[0] << r
               << config.getColor("compact_disk_storage", "fields.separator_color", "white") << "-" << r
               << config.getColor("compact_disk_storage", "fields.capacity_color", "white")
               << c.second << "GB" << r
               << config.getColor("compact_disk_storage", "brackets.color", "white") << ")" << r;
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
                    "blue")
               << config.getPrefix(
                    "detailed_disk_storage",
                    "storage_summary.header.prefix",
                    "")
               << r

               << config.getNestedColor(
                    "detailed_disk_storage",
                    "storage_summary.header.text_color",
                    "red")
               << config.getLabel(
                    "detailed_disk_storage",
                    "storage_summary.header.text",
                    "")
               << r

               << config.getNestedColor(
                    "detailed_disk_storage",
                    "storage_summary.header.suffix_color",
                    "blue")
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
                            "bright_cyan")
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
                            "red")
                       << config.getLabel(
                            "detailed_disk_storage",
                            "storage_summary.fields.drive_letter.label",
                            "Disk")
                       << r;

                    // Opening parenthesis
                    ss << config.getNestedColor(
                            "detailed_disk_storage",
                            "storage_summary.fields.drive_letter.letter_prefix_color",
                            "blue")
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
                            "red")
                       << d.drive_letter
                       << r;

                    // Closing parenthesis
                    ss << config.getNestedColor(
                            "detailed_disk_storage",
                            "storage_summary.fields.drive_letter.letter_suffix_color",
                            "red")
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
                        "cyan")
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
                            "bright_cyan")
                       << config.getPrefix(
                            "detailed_disk_storage",
                            "storage_summary.brackets.round_open",
                            "(")
                       << r

                       << config.getNestedColor(
                            "detailed_disk_storage",
                            "storage_summary.fields.used_label.value_color",
                            "red")
                       << config.getLabel(
                            "detailed_disk_storage",
                            "storage_summary.fields.used_label.text",
                            "")
                       << r

                       << config.getNestedColor(
                            "detailed_disk_storage",
                            "storage_summary.brackets.round_color",
                            "bright_cyan")
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
                            "bright_cyan")
                       << fmt_storage(d.used_space)
                       << r

                       << config.getNestedColor(
                            "detailed_disk_storage",
                            "storage_summary.fields.used_space.unit_color",
                            "cyan")
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
                            "cyan")
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
                            "bright_cyan")
                       << fmt_storage(d.total_space)
                       << r

                       << config.getNestedColor(
                            "detailed_disk_storage",
                            "storage_summary.fields.total_space.unit_color",
                            "blue")
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
                            "red")
                       << config.getPrefix(
                            "detailed_disk_storage",
                            "storage_summary.fields.used_percentage.prefix",
                            "")
                       << r;

                    // Percentage value
                    ss << config.getNestedColor(
                            "detailed_disk_storage",
                            "storage_summary.fields.used_percentage.value_color",
                            "bright_blue")
                       << fmt_percentage(d.used_percentage)
                       << r;

                    // Percentage suffix
                    ss << config.getNestedColor(
                            "detailed_disk_storage",
                            "storage_summary.fields.used_percentage.suffix_color",
                            "red")
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
                        "bright_cyan")
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
                            "red")
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
                                "blue")
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
                                "bright_cyan")
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
                        "cyan")
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
                    "blue")
               << config.getPrefix(
                    "detailed_disk_storage",
                    "disk_performance.header.prefix",
                    "")
               << r

               << config.getNestedColor(
                    "detailed_disk_storage",
                    "disk_performance.header.text_color",
                    "bright_cyan")
               << config.getLabel(
                    "detailed_disk_storage",
                    "disk_performance.header.text",
                    "")
               << r

               << config.getNestedColor(
                    "detailed_disk_storage",
                    "disk_performance.header.suffix_color",
                    "blue")
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
                        "red")
                   << config.getLabel(
                        "detailed_disk_storage",
                        "disk_performance.fields.drive_letter.label",
                        "Disk")
                   << r;

                // Opening parenthesis
                ss << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance.fields.drive_letter.letter_prefix_color",
                        "blue")
                   << config.getPrefix(
                        "detailed_disk_storage",
                        "disk_performance.fields.drive_letter.letter_prefix",
                        "(")
                   << r;

                // Actual drive letter
                ss << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance.fields.drive_letter.letter_color",
                        "red")
                   << d.drive_letter
                   << r;

                // Closing parenthesis
                ss << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance.fields.drive_letter.letter_suffix_color",
                        "red")
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
                    "cyan")
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
                        "bright_cyan")
                   << config.getLabel(
                        "detailed_disk_storage",
                        "disk_performance.fields.read_speed.label",
                        "")
                   << r

                   << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance.fields.read_speed.value_color",
                        "red")
                   << fmt_speed(d.read_speed)
                   << r

                   << " "

                   << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance.fields.read_speed.unit_color",
                        "bright_cyan")
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
                            "blue")
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
                        "bright_cyan")
                   << config.getLabel(
                        "detailed_disk_storage",
                        "disk_performance.fields.write_speed.label",
                        "")
                   << r

                   << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance.fields.write_speed.value_color",
                        "red")
                   << fmt_speed(d.write_speed)
                   << r

                   << " "

                   << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance.fields.write_speed.unit_color",
                        "bright_cyan")
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
                            "blue")
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
                        "bright_cyan")
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
                            "blue")
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
                            "bright_cyan")
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
                    "cyan")
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
                    "bright_cyan")
               << config.getPrefix(
                    "detailed_disk_storage",
                    "disk_performance_predicted.header.prefix",
                    "")
               << r

               << config.getNestedColor(
                    "detailed_disk_storage",
                    "disk_performance_predicted.header.text_color",
                    "bright_cyan")
               << config.getLabel(
                    "detailed_disk_storage",
                    "disk_performance_predicted.header.text",
                    "")
               << r

               << config.getNestedColor(
                    "detailed_disk_storage",
                    "disk_performance_predicted.header.suffix_color",
                    "bright_cyan")
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
                        "red")
                   << config.getLabel(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.drive_letter.label",
                        "Disk")
                   << r;

                // Opening parenthesis
                ss << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.drive_letter.letter_prefix_color",
                        "blue")
                   << config.getPrefix(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.drive_letter.letter_prefix",
                        "(")
                   << r;

                // Actual drive letter
                ss << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.drive_letter.letter_color",
                        "red")
                   << d.drive_letter
                   << r;

                // Closing parenthesis
                ss << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.drive_letter.letter_suffix_color",
                        "red")
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
                    "cyan")
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
                        "bright_cyan")
                   << config.getLabel(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.read_speed.label",
                        "")
                   << r

                   << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.read_speed.value_color",
                        "cyan")
                   << fmt_speed(d.predicted_read_speed)
                   << r

                   << " "

                   << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.read_speed.unit_color",
                        "bright_cyan")
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
                            "blue")
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
                        "bright_cyan")
                   << config.getLabel(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.write_speed.label",
                        "")
                   << r

                   << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.write_speed.value_color",
                        "cyan")
                   << fmt_speed(d.predicted_write_speed)
                   << r

                   << " "

                   << config.getNestedColor(
                        "detailed_disk_storage",
                        "disk_performance_predicted.fields.write_speed.unit_color",
                        "bright_cyan")
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
                            "blue")
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
                        "bright_cyan")
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
                            "blue")
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
                            "bright_cyan")
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
                    "cyan")
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

    // Network header
    if (config.getNestedBool(
            "detailed_network_connection",
            "header.show",
            true))
    {
        ostringstream ss;

        ss << config.getColor(
                "detailed_network_connection",
                "header.prefix_color",
                "bright_blue")
           << config.getPrefix(
                "detailed_network_connection",
                "header.prefix",
                "#- ")
           << r

           << config.getColor(
                "detailed_network_connection",
                "header.text_color",
                "red")
           << config.getLabel(
                "detailed_network_connection",
                "header.text",
                "Network Info ")
           << r

           << config.getColor(
                "detailed_network_connection",
                "header.suffix_color",
                "cyan")
           << config.getPrefix(
                "detailed_network_connection",
                "header.suffix",
                "---------------------------------------------------#")
           << r;

        lp.push(ss.str());
    }


    // Network Name
    if (config.getNestedBool(
            "detailed_network_connection",
            "fields.name.show",
            true))
    {
        ostringstream ss;

        ss << config.getColor(
                "detailed_network_connection",
                "fields.name.name_prefix_color",
                "red")
           << config.getPrefix(
                "detailed_network_connection",
                "fields.name.name_prefix",
                "~ ")
           << r

           << config.getColor(
                "detailed_network_connection",
                "fields.name.label_color",
                "blue")
           << left
           << setw(26)
           << config.getLabel(
                "detailed_network_connection",
                "fields.name.label",
                "Network Name")
           << r

           << config.getColor(
                "detailed_network_connection",
                "separator.color",
                "red")
           << config.getPrefix(
                "detailed_network_connection",
                "separator.text",
                ":")
           << " "
           << r

           << config.getColor(
                "detailed_network_connection",
                "fields.name.value_color",
                "blue")
           << net.get_network_name()
           << r;

        lp.push(ss.str());
    }


    // Network Type
    if (config.getNestedBool(
            "detailed_network_connection",
            "fields.type.show",
            true))
    {
        ostringstream ss;

        ss << config.getColor(
                "detailed_network_connection",
                "fields.type.type_prefix_color",
                "red")
           << config.getPrefix(
                "detailed_network_connection",
                "fields.type.type_prefix",
                "~ ")
           << r

           << config.getColor(
                "detailed_network_connection",
                "fields.type.label_color",
                "blue")
           << left
           << setw(26)
           << config.getLabel(
                "detailed_network_connection",
                "fields.type.label",
                "Network Type")
           << r

           << config.getColor(
                "detailed_network_connection",
                "separator.color",
                "red")
           << config.getPrefix(
                "detailed_network_connection",
                "separator.text",
                ":")
           << " "
           << r

           << config.getColor(
                "detailed_network_connection",
                "fields.type.value_color",
                "bright_cyan")
           << c_net.get_network_type()
           << r;

        lp.push(ss.str());
    }


    // Local IP
    if (config.getNestedBool(
            "detailed_network_connection",
            "fields.local_ip.show",
            true))
    {
        ostringstream ss;

        ss << config.getColor(
                "detailed_network_connection",
                "fields.local_ip.local_ip_prefix_color",
                "red")
           << config.getPrefix(
                "detailed_network_connection",
                "fields.local_ip.local_ip_prefix",
                "~ ")
           << r

           << config.getColor(
                "detailed_network_connection",
                "fields.local_ip.label_color",
                "blue")
           << left
           << setw(26)
           << config.getLabel(
                "detailed_network_connection",
                "fields.local_ip.label",
                "Local IP")
           << r

           << config.getColor(
                "detailed_network_connection",
                "separator.color",
                "red")
           << config.getPrefix(
                "detailed_network_connection",
                "separator.text",
                ":")
           << " "
           << r

           << config.getColor(
                "detailed_network_connection",
                "fields.local_ip.value_color",
                "cyan")
           << net.get_local_ip()
           << r;

        lp.push(ss.str());
    }


    // Public IP
    if (config.getNestedBool(
            "detailed_network_connection",
            "fields.public_ip.show",
            true))
    {
        ostringstream ss;

        ss << config.getColor(
                "detailed_network_connection",
                "fields.public_ip.public_ip_prefix_color",
                "red")
           << config.getPrefix(
                "detailed_network_connection",
                "fields.public_ip.public_ip_prefix",
                "~ ")
           << r

           << config.getColor(
                "detailed_network_connection",
                "fields.public_ip.label_color",
                "blue")
           << left
           << setw(26)
           << config.getLabel(
                "detailed_network_connection",
                "fields.public_ip.label",
                "Public IP:")
           << r

           << config.getColor(
                "detailed_network_connection",
                "separator.color",
                "red")
           << config.getPrefix(
                "detailed_network_connection",
                "separator.text",
                ":")
           << " "
           << r

           << config.getColor(
                "detailed_network_connection",
                "fields.public_ip.value_color",
                "cyan")
           << net.get_public_ip()
           << r;

        lp.push(ss.str());
    }


    // Locale
    if (config.getNestedBool(
            "detailed_network_connection",
            "fields.locale.show",
            true))
    {
        ostringstream ss;

        ss << config.getColor(
                "detailed_network_connection",
                "fields.locale.locale_prefix_color",
                "red")
           << config.getPrefix(
                "detailed_network_connection",
                "fields.locale.locale_prefix",
                "~ ")
           << r

           << config.getColor(
                "detailed_network_connection",
                "fields.locale.label_color",
                "blue")
           << left
           << setw(26)
           << config.getLabel(
                "detailed_network_connection",
                "fields.locale.label",
                "Locale")
           << r

           << config.getColor(
                "detailed_network_connection",
                "separator.color",
                "red")
           << config.getPrefix(
                "detailed_network_connection",
                "separator.text",
                ":")
           << " "
           << r

           << config.getColor(
                "detailed_network_connection",
                "fields.locale.value_color",
                "bright_cyan")
           << net.get_locale()
           << r;

        lp.push(ss.str());
    }


    // MAC Address
    if (config.getNestedBool(
            "detailed_network_connection",
            "fields.mac.show",
            true))
    {
        ostringstream ss;

        ss << config.getColor(
                "detailed_network_connection",
                "fields.mac.mac_prefix_color",
                "red")
           << config.getPrefix(
                "detailed_network_connection",
                "fields.mac.mac_prefix",
                "~ ")
           << r

           << config.getColor(
                "detailed_network_connection",
                "fields.mac.label_color",
                "blue")
           << left
           << setw(26)
           << config.getLabel(
                "detailed_network_connection",
                "fields.mac.label",
                "Mac address")
           << r

           << config.getColor(
                "detailed_network_connection",
                "separator.color",
                "red")
           << config.getPrefix(
                "detailed_network_connection",
                "separator.text",
                ":")
           << " "
           << r

           << config.getColor(
                "detailed_network_connection",
                "fields.mac.value_color",
                "blue")
           << net.get_mac_address()
           << r;

        lp.push(ss.str());
    }


    // Upload Speed
    if (config.getNestedBool(
            "detailed_network_connection",
            "fields.upload.show",
            true))
    {
        ostringstream ss;

        ss << config.getColor(
                "detailed_network_connection",
                "fields.upload.upload_prefix_color",
                "red")
           << config.getPrefix(
                "detailed_network_connection",
                "fields.upload.upload_prefix",
                "~ ")
           << r

           << config.getColor(
                "detailed_network_connection",
                "fields.upload.label_color",
                "blue")
           << left
           << setw(26)
           << config.getLabel(
                "detailed_network_connection",
                "fields.upload.label",
                "avg upload speed")
           << r

           << config.getColor(
                "detailed_network_connection",
                "separator.color",
                "red")
           << config.getPrefix(
                "detailed_network_connection",
                "separator.text",
                ":")
           << " "
           << r

           << config.getColor(
                "detailed_network_connection",
                "fields.upload.value_color",
                "bright_cyan")
           << net.get_network_upload_speed()
           << r;

        lp.push(ss.str());
    }


    // Download Speed
    if (config.getNestedBool(
            "detailed_network_connection",
            "fields.download.show",
            true))
    {
        ostringstream ss;

        ss << config.getColor(
                "detailed_network_connection",
                "fields.download.download_prefix_color",
                "red")
           << config.getPrefix(
                "detailed_network_connection",
                "fields.download.download_prefix",
                "~ ")
           << r

           << config.getColor(
                "detailed_network_connection",
                "fields.download.label_color",
                "blue")
           << left
           << setw(26)
           << config.getLabel(
                "detailed_network_connection",
                "fields.download.label",
                "avg download speed")
           << r

           << config.getColor(
                "detailed_network_connection",
                "separator.color",
                "red")
           << config.getPrefix(
                "detailed_network_connection",
                "separator.text",
                ":")
           << " "
           << r

           << config.getColor(
                "detailed_network_connection",
                "fields.download.value_color",
                "blue")
           << net.get_network_download_speed()
           << r;

        lp.push(ss.str());
    }
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

    // ---------- HEADER ----------
    if (config.getNestedBool("dummy_network_info", "header.show", true)) {
        ostringstream ss;
        ss << config.getNestedColor("dummy_network_info", "header.prefix_color", "")
           << config.getPrefix("dummy_network_info", "header.prefix", "") << r
           << config.getNestedColor("dummy_network_info", "header.text_color", "")
           << config.getLabel("dummy_network_info", "header.text", "") << r
           << config.getNestedColor("dummy_network_info", "header.suffix_color", "")
           << config.getPrefix("dummy_network_info", "header.suffix", "") << r;
        lp.push(ss.str());
    }

    // ---------- NETWORK NAME ----------
    if (config.getNestedBool("dummy_network_info", "fields.name.show", true)) {
        ostringstream ss;
        ss << config.getNestedColor("dummy_network_info", "fields.name.prefix_color", "")
           << config.getPrefix("dummy_network_info", "fields.name.prefix", "") << r
           << config.getNestedColor("dummy_network_info", "fields.name.label_color", "")
           << left << setw(26) << config.getLabel("dummy_network_info", "fields.name.label", "") << r
           << config.getNestedColor("dummy_network_info", "separator.color", "")
           << config.getPrefix("dummy_network_info", "separator.text", "") << " " << r
           << config.getNestedColor("dummy_network_info", "fields.name.value_color", "")
           << config.getLabel("dummy_network_info", "fields.name.value", "") << r;
        lp.push(ss.str());
    }

    // ---------- NETWORK TYPE ----------
    if (config.getNestedBool("dummy_network_info", "fields.type.show", true)) {
        ostringstream ss;
        ss << config.getNestedColor("dummy_network_info", "fields.type.prefix_color", "")
           << config.getPrefix("dummy_network_info", "fields.type.prefix", "") << r
           << config.getNestedColor("dummy_network_info", "fields.type.label_color", "")
           << left << setw(26) << config.getLabel("dummy_network_info", "fields.type.label", "") << r
           << config.getNestedColor("dummy_network_info", "separator.color", "")
           << config.getPrefix("dummy_network_info", "separator.text", "") << " " << r
           << config.getNestedColor("dummy_network_info", "fields.type.value_color", "")
           << config.getLabel("dummy_network_info", "fields.type.value", "") << r;
        lp.push(ss.str());
    }

    // ---------- LOCAL IP ----------
    if (config.getNestedBool("dummy_network_info", "fields.local_ip.show", true)) {
        ostringstream ss;
        ss << config.getNestedColor("dummy_network_info", "fields.local_ip.prefix_color", "")
           << config.getPrefix("dummy_network_info", "fields.local_ip.prefix", "") << r
           << config.getNestedColor("dummy_network_info", "fields.local_ip.label_color", "")
           << left << setw(26) << config.getLabel("dummy_network_info", "fields.local_ip.label", "") << r
           << config.getNestedColor("dummy_network_info", "separator.color", "")
           << config.getPrefix("dummy_network_info", "separator.text", "") << " " << r
           << config.getNestedColor("dummy_network_info", "fields.local_ip.value_color", "")
           << config.getLabel("dummy_network_info", "fields.local_ip.value", "") << r;
        lp.push(ss.str());
    }

    // ---------- READ SPEED ----------
    if (config.getNestedBool("dummy_network_info", "fields.read_speed.show", true)) {
        ostringstream ss;
        ss << config.getNestedColor("dummy_network_info", "fields.read_speed.prefix_color", "")
           << config.getPrefix("dummy_network_info", "fields.read_speed.prefix", "") << r
           << config.getNestedColor("dummy_network_info", "fields.read_speed.label_color", "")
           << left << setw(26) << config.getLabel("dummy_network_info", "fields.read_speed.label", "") << r
           << config.getNestedColor("dummy_network_info", "separator.color", "")
           << config.getPrefix("dummy_network_info", "separator.text", "") << " " << r
           << config.getNestedColor("dummy_network_info", "fields.read_speed.value_color", "")
           << config.getLabel("dummy_network_info", "fields.read_speed.value", "") << r
           << config.getNestedColor("dummy_network_info", "fields.read_speed.unit_color", "")
           << config.getLabel("dummy_network_info", "fields.read_speed.unit", "") << r;
        lp.push(ss.str());
    }

    // ---------- WRITE SPEED ----------
    if (config.getNestedBool("dummy_network_info", "fields.write_speed.show", true)) {
        ostringstream ss;
        ss << config.getNestedColor("dummy_network_info", "fields.write_speed.prefix_color", "")
           << config.getPrefix("dummy_network_info", "fields.write_speed.prefix", "") << r
           << config.getNestedColor("dummy_network_info", "fields.write_speed.label_color", "")
           << left << setw(26) << config.getLabel("dummy_network_info", "fields.write_speed.label", "") << r
           << config.getNestedColor("dummy_network_info", "separator.color", "")
           << config.getPrefix("dummy_network_info", "separator.text", "") << " " << r
           << config.getNestedColor("dummy_network_info", "fields.write_speed.value_color", "")
           << config.getLabel("dummy_network_info", "fields.write_speed.value", "") << r
           << config.getNestedColor("dummy_network_info", "fields.write_speed.unit_color", "")
           << config.getLabel("dummy_network_info", "fields.write_speed.unit", "") << r;
        lp.push(ss.str());
    }
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

        // Header
        if (config.getNestedBool("os_info", "header.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("os_info", "header.prefix_color", "")
               << config.getPrefix("os_info", "header.prefix", "") << r
               << config.getNestedColor("os_info", "header.text_color", "")
               << config.getLabel("os_info", "header.text", "") << r
               << config.getNestedColor("os_info", "header.suffix_color", "")
               << config.getPrefix("os_info", "header.suffix", "") << r;
            lp.push(ss.str());
        }

        // Name
        if (config.getNestedBool("os_info", "fields.name.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("os_info", "fields.name.name_prefix_color", "")
               << config.getPrefix("os_info", "fields.name.name_prefix", "") << r
               << config.getNestedColor("os_info", "fields.name.label_color", "")
               << left << setw(26) << config.getLabel("os_info", "fields.name.label", "") << r
               << config.getNestedColor("os_info", "separator.color", "")
               << config.getPrefix("os_info", "separator.text", "") << " " << r
               << config.getNestedColor("os_info", "fields.name.value_color", "")
               << os.GetOSName() << r;
            lp.push(ss.str());
        }

        // Build
        if (config.getNestedBool("os_info", "fields.build.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("os_info", "fields.build.build_prefix_color", "")
               << config.getPrefix("os_info", "fields.build.build_prefix", "") << r
               << config.getNestedColor("os_info", "fields.build.label_color", "")
               << left << setw(26) << config.getLabel("os_info", "fields.build.label", "") << r
               << config.getNestedColor("os_info", "separator.color", "")
               << config.getPrefix("os_info", "separator.text", "") << " " << r
               << config.getNestedColor("os_info", "fields.build.value_color", "")
               << os.GetOSVersion() << r;
            lp.push(ss.str());
        }

        // Architecture
        if (config.getNestedBool("os_info", "fields.architecture.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("os_info", "fields.architecture.architecture_prefix_color", "")
               << config.getPrefix("os_info", "fields.architecture.architecture_prefix", "") << r
               << config.getNestedColor("os_info", "fields.architecture.label_color", "")
               << left << setw(26) << config.getLabel("os_info", "fields.architecture.label", "") << r
               << config.getNestedColor("os_info", "separator.color", "")
               << config.getPrefix("os_info", "separator.text", "") << " " << r
               << config.getNestedColor("os_info", "fields.architecture.value_color", "")
               << os.GetOSArchitecture() << r;
            lp.push(ss.str());
        }

        // Kernel
        if (config.getNestedBool("os_info", "fields.kernel.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("os_info", "fields.kernel.kernel_prefix_color", "")
               << config.getPrefix("os_info", "fields.kernel.kernel_prefix", "") << r
               << config.getNestedColor("os_info", "fields.kernel.label_color", "")
               << left << setw(26) << config.getLabel("os_info", "fields.kernel.label", "") << r
               << config.getNestedColor("os_info", "separator.color", "")
               << config.getPrefix("os_info", "separator.text", "") << " " << r
               << config.getNestedColor("os_info", "fields.kernel.value_color", "")
               << os.get_os_kernel_info() << r;
            lp.push(ss.str());
        }

        // Uptime
        if (config.getNestedBool("os_info", "fields.uptime.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("os_info", "fields.uptime.uptime_prefix_color", "")
               << config.getPrefix("os_info", "fields.uptime.uptime_prefix", "") << r
               << config.getNestedColor("os_info", "fields.uptime.label_color", "")
               << left << setw(26) << config.getLabel("os_info", "fields.uptime.label", "") << r
               << config.getNestedColor("os_info", "separator.color", "")
               << config.getPrefix("os_info", "separator.text", "") << " " << r
               << config.getNestedColor("os_info", "fields.uptime.value_color", "")
               << os.get_os_uptime() << r;
            lp.push(ss.str());
        }

        // Install Date
        if (config.getNestedBool("os_info", "fields.install_date.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("os_info", "fields.install_date.install_date_prefix_color", "")
               << config.getPrefix("os_info", "fields.install_date.install_date_prefix", "") << r
               << config.getNestedColor("os_info", "fields.install_date.label_color", "")
               << left << setw(26) << config.getLabel("os_info", "fields.install_date.label", "") << r
               << config.getNestedColor("os_info", "separator.color", "")
               << config.getPrefix("os_info", "separator.text", "") << " " << r
               << config.getNestedColor("os_info", "fields.install_date.value_color", "")
               << os.get_os_install_date() << r;
            lp.push(ss.str());
        }

        // Serial
        if (config.getNestedBool("os_info", "fields.serial.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("os_info", "fields.serial.serial_prefix_color", "")
               << config.getPrefix("os_info", "fields.serial.serial_prefix", "") << r
               << config.getNestedColor("os_info", "fields.serial.label_color", "")
               << left << setw(26) << config.getLabel("os_info", "fields.serial.label", "") << r
               << config.getNestedColor("os_info", "separator.color", "")
               << config.getPrefix("os_info", "separator.text", "") << " " << r
               << config.getNestedColor("os_info", "fields.serial.value_color", "")
               << os.get_os_serial_number() << r;
            lp.push(ss.str());
        }
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
//  This section displays comprehensive CPU information including:
//  1. Brand               - CPU model/brand string
//  2. Utilization          - Current CPU usage percentage
//  3. Speed                - Current clock speed
//  4. Base Speed           - Base/rated clock speed
//  5. Cores                - Number of physical cores
//  6. Logical Processors   - Number of logical processors (threads)
//  7. Sockets              - Number of CPU sockets
//  8. Virtualization       - Virtualization support status
//  9. L1 Cache             - L1 cache size
//  10. L2 Cache            - L2 cache size
//  11. L3 Cache            - L3 cache size
//  All labels, values, colors, prefixes, and toggles are fully JSON-driven
//  via the "detailed_processor" config block (aliased as "cpu_info").
// ============================================================================
//
//  Output Example:
//  #- CPU Info -------------------------------------------------------#
//  ~ Brand                   : AMD Ryzen 9 7950X
//  ~ Utilization              : 12%
//  ~ Speed                   : 5.4 GHz
//  ~ Base Speed               : 4.5 GHz
//  ~ Cores                   : 16
//  ~ Logical Processors       : 32
//  ~ Sockets                 : 1
//  ~ Virtualization           : Enabled
//  ~ L1 Cache                : 1.0 MB
//  ~ L2 Cache                : 16.0 MB
//  ~ L3 Cache                : 64.0 MB
// ============================================================================

    // CPU Info (JSON Driven)
    if (config.isEnabled("cpu_info")) {
        lp.push("");

        // Header
        if (config.getNestedBool("cpu_info", "header.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("cpu_info", "header.prefix_color", "")
               << config.getPrefix("cpu_info", "header.prefix", "") << r
               << config.getNestedColor("cpu_info", "header.text_color", "")
               << config.getLabel("cpu_info", "header.text", "") << r
               << config.getNestedColor("cpu_info", "header.suffix_color", "")
               << config.getPrefix("cpu_info", "header.suffix", "") << r;
            lp.push(ss.str());
        }

        // Brand
        if (config.getNestedBool("cpu_info", "fields.brand.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("cpu_info", "fields.brand.brand_prefix_color", "")
               << config.getPrefix("cpu_info", "fields.brand.brand_prefix", "") << r
               << config.getNestedColor("cpu_info", "fields.brand.label_color", "")
               << left << setw(26) << config.getLabel("cpu_info", "fields.brand.label", "") << r
               << config.getNestedColor("cpu_info", "separator.color", "")
               << config.getPrefix("cpu_info", "separator.text", "") << " " << r
               << config.getNestedColor("cpu_info", "fields.brand.value_color", "")
               << cpu.get_cpu_info() << r;
            lp.push(ss.str());
        }

        // Utilization
        if (config.getNestedBool("cpu_info", "fields.utilization.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("cpu_info", "fields.utilization.utilization_prefix_color", "")
               << config.getPrefix("cpu_info", "fields.utilization.utilization_prefix", "") << r
               << config.getNestedColor("cpu_info", "fields.utilization.label_color", "")
               << left << setw(26) << config.getLabel("cpu_info", "fields.utilization.label", "") << r
               << config.getNestedColor("cpu_info", "separator.color", "")
               << config.getPrefix("cpu_info", "separator.text", "") << " " << r
               << config.getNestedColor("cpu_info", "fields.utilization.value_color", "")
               << cpu.get_cpu_utilization() << r
               << config.getNestedColor("cpu_info", "percent_sign.color", "")
               << config.getPrefix("cpu_info", "percent_sign.text", "%") << r;
            lp.push(ss.str());
        }

        // Speed
        if (config.getNestedBool("cpu_info", "fields.speed.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("cpu_info", "fields.speed.speed_prefix_color", "")
               << config.getPrefix("cpu_info", "fields.speed.speed_prefix", "") << r
               << config.getNestedColor("cpu_info", "fields.speed.label_color", "")
               << left << setw(26) << config.getLabel("cpu_info", "fields.speed.label", "") << r
               << config.getNestedColor("cpu_info", "separator.color", "")
               << config.getPrefix("cpu_info", "separator.text", "") << " " << r
               << config.getNestedColor("cpu_info", "fields.speed.value_color", "")
               << cpu.get_cpu_speed() << r;
            lp.push(ss.str());
        }

        // Base Speed
        if (config.getNestedBool("cpu_info", "fields.base_speed.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("cpu_info", "fields.base_speed.base_speed_prefix_color", "")
               << config.getPrefix("cpu_info", "fields.base_speed.base_speed_prefix", "") << r
               << config.getNestedColor("cpu_info", "fields.base_speed.label_color", "")
               << left << setw(26) << config.getLabel("cpu_info", "fields.base_speed.label", "") << r
               << config.getNestedColor("cpu_info", "separator.color", "")
               << config.getPrefix("cpu_info", "separator.text", "") << " " << r
               << config.getNestedColor("cpu_info", "fields.base_speed.value_color", "")
               << cpu.get_cpu_base_speed() << r;
            lp.push(ss.str());
        }

        // Cores
        if (config.getNestedBool("cpu_info", "fields.cores.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("cpu_info", "fields.cores.cores_prefix_color", "")
               << config.getPrefix("cpu_info", "fields.cores.cores_prefix", "") << r
               << config.getNestedColor("cpu_info", "fields.cores.label_color", "")
               << left << setw(26) << config.getLabel("cpu_info", "fields.cores.label", "") << r
               << config.getNestedColor("cpu_info", "separator.color", "")
               << config.getPrefix("cpu_info", "separator.text", "") << " " << r
               << config.getNestedColor("cpu_info", "fields.cores.value_color", "")
               << cpu.get_cpu_cores() << r;
            lp.push(ss.str());
        }

        // Logical Processors
        if (config.getNestedBool("cpu_info", "fields.logical_processors.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("cpu_info", "fields.logical_processors.logical_processors_prefix_color", "")
               << config.getPrefix("cpu_info", "fields.logical_processors.logical_processors_prefix", "") << r
               << config.getNestedColor("cpu_info", "fields.logical_processors.label_color", "")
               << left << setw(26) << config.getLabel("cpu_info", "fields.logical_processors.label", "") << r
               << config.getNestedColor("cpu_info", "separator.color", "")
               << config.getPrefix("cpu_info", "separator.text", "") << " " << r
               << config.getNestedColor("cpu_info", "fields.logical_processors.value_color", "")
               << cpu.get_cpu_logical_processors() << r;
            lp.push(ss.str());
        }

        // Sockets
        if (config.getNestedBool("cpu_info", "fields.sockets.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("cpu_info", "fields.sockets.sockets_prefix_color", "")
               << config.getPrefix("cpu_info", "fields.sockets.sockets_prefix", "") << r
               << config.getNestedColor("cpu_info", "fields.sockets.label_color", "")
               << left << setw(26) << config.getLabel("cpu_info", "fields.sockets.label", "") << r
               << config.getNestedColor("cpu_info", "separator.color", "")
               << config.getPrefix("cpu_info", "separator.text", "") << " " << r
               << config.getNestedColor("cpu_info", "fields.sockets.value_color", "")
               << cpu.get_cpu_sockets() << r;
            lp.push(ss.str());
        }

        // Virtualization
        if (config.getNestedBool("cpu_info", "fields.virtualization.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("cpu_info", "fields.virtualization.virtualization_prefix_color", "")
               << config.getPrefix("cpu_info", "fields.virtualization.virtualization_prefix", "") << r
               << config.getNestedColor("cpu_info", "fields.virtualization.label_color", "")
               << left << setw(26) << config.getLabel("cpu_info", "fields.virtualization.label", "") << r
               << config.getNestedColor("cpu_info", "separator.color", "")
               << config.getPrefix("cpu_info", "separator.text", "") << " " << r
               << config.getNestedColor("cpu_info", "fields.virtualization.value_color", "")
               << cpu.get_cpu_virtualization() << r;
            lp.push(ss.str());
        }

        // L1 Cache
        if (config.getNestedBool("cpu_info", "fields.l1_cache.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("cpu_info", "fields.l1_cache.l1_cache_prefix_color", "")
               << config.getPrefix("cpu_info", "fields.l1_cache.l1_cache_prefix", "") << r
               << config.getNestedColor("cpu_info", "fields.l1_cache.label_color", "")
               << left << setw(26) << config.getLabel("cpu_info", "fields.l1_cache.label", "") << r
               << config.getNestedColor("cpu_info", "separator.color", "")
               << config.getPrefix("cpu_info", "separator.text", "") << " " << r
               << config.getNestedColor("cpu_info", "fields.l1_cache.value_color", "")
               << cpu.get_cpu_l1_cache() << r;
            lp.push(ss.str());
        }

        // L2 Cache
        if (config.getNestedBool("cpu_info", "fields.l2_cache.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("cpu_info", "fields.l2_cache.l2_cache_prefix_color", "")
               << config.getPrefix("cpu_info", "fields.l2_cache.l2_cache_prefix", "") << r
               << config.getNestedColor("cpu_info", "fields.l2_cache.label_color", "")
               << left << setw(26) << config.getLabel("cpu_info", "fields.l2_cache.label", "") << r
               << config.getNestedColor("cpu_info", "separator.color", "")
               << config.getPrefix("cpu_info", "separator.text", "") << " " << r
               << config.getNestedColor("cpu_info", "fields.l2_cache.value_color", "")
               << cpu.get_cpu_l2_cache() << r;
            lp.push(ss.str());
        }

        // L3 Cache
        if (config.getNestedBool("cpu_info", "fields.l3_cache.show", true)) {
            ostringstream ss;
            ss << config.getNestedColor("cpu_info", "fields.l3_cache.l3_cache_prefix_color", "")
               << config.getPrefix("cpu_info", "fields.l3_cache.l3_cache_prefix", "") << r
               << config.getNestedColor("cpu_info", "fields.l3_cache.label_color", "")
               << left << setw(26) << config.getLabel("cpu_info", "fields.l3_cache.label", "") << r
               << config.getNestedColor("cpu_info", "separator.color", "")
               << config.getPrefix("cpu_info", "separator.text", "") << " " << r
               << config.getNestedColor("cpu_info", "fields.l3_cache.value_color", "")
               << cpu.get_cpu_l3_cache() << r;
            lp.push(ss.str());
        }
    }

// ============================================================================
//   ██████╗ ██████╗ ██╗   ██╗    ██╗███╗   ██╗███████╗ ██████╗ 
//  ██╔════╝ ██╔══██╗██║   ██║    ██║████╗  ██║██╔════╝██╔═══██╗
//  ██║  ███╗██████╔╝██║   ██║    ██║██╔██╗ ██║█████╗  ██║   ██║
//  ██║   ██║██╔═══╝ ██║   ██║    ██║██║╚██╗██║██╔══╝  ██║   ██║
//  ╚██████╔╝██║     ╚██████╔╝    ██║██║ ╚████║██║     ╚██████╔╝
//   ╚═════╝ ╚═╝      ╚═════╝     ╚═╝╚═╝  ╚═══╝╚═╝      ╚═════╝ 
// ============================================================================
//                    D E T A I L E D   G R A P H I C S   C A R D
// ============================================================================
//  This section displays comprehensive GPU information including:
//  1. Name            - GPU model name (per GPU, multi-GPU supported)
//  2. Memory           - VRAM information
//  3. Usage            - Current GPU usage percentage
//  4. Vendor           - GPU vendor (NVIDIA, AMD, Intel, etc.)
//  5. Driver Version   - Installed driver version
//  6. Temperature      - Current GPU temperature
//  7. Core Count       - Number of GPU cores
//  Plus a "Primary GPU Details" summary block:
//  8. Name / VRAM / Frequency of the primary GPU
//  If no GPU is detected, an error line is shown instead.
//  All labels, values, colors, prefixes, and toggles are fully JSON-driven
//  via the "detailed_graphics_card" config block (aliased as "gpu_info").
// ============================================================================
//
//  Output Example:
//  #- GPU Info -------------------------------------------------------#
//  GPU 1
//  |-> Name                : NVIDIA GeForce RTX 4090
//  |-> Memory               : 24 GB
//  |-> Usage                : 8%
//  |-> Vendor               : NVIDIA
//  |-> Driver Version       : 551.86
//  |-> Temperature          : 45 C
//  |-> Core Count           : 16384
//
//  #- Primary GPU Details ---------------------------------------#
//  |-> Name                : NVIDIA GeForce RTX 4090
//  |-> VRAM                 : 24 GiB
//  |-> Frequency            : 2.52 GHz
// ============================================================================

                // GPU Info (JSON Driven)
                if (config.isEnabled("gpu_info")) {
                    lp.push("");
                    auto all_gpu_info = obj_gpu.get_all_gpu_info();

                    if (all_gpu_info.empty()) {
                        if (config.getNestedBool("gpu_info", "header.show", true)) {
                            ostringstream ss;
                            ss << config.getNestedColor("gpu_info", "header.prefix_color", "")
                               << config.getPrefix("gpu_info", "header.prefix", "") << r
                               << config.getNestedColor("gpu_info", "header.text_color", "")
                               << config.getLabel("gpu_info", "header.text", "") << r
                               << config.getNestedColor("gpu_info", "header.suffix_color", "")
                               << config.getPrefix("gpu_info", "header.suffix", "") << r;
                            lp.push(ss.str());
                        }
                        lp.push(config.getColor("gpu_info", "error_color", "white") + "No GPU detected." + r);
                    }
                    else {
                        // Main Header
                        if (config.getNestedBool("gpu_info", "header.show", true)) {
                            ostringstream ss;
                            ss << config.getNestedColor("gpu_info", "header.prefix_color", "")
                               << config.getPrefix("gpu_info", "header.prefix", "") << r
                               << config.getNestedColor("gpu_info", "header.text_color", "")
                               << config.getLabel("gpu_info", "header.text", "") << r
                               << config.getNestedColor("gpu_info", "header.suffix_color", "")
                               << config.getPrefix("gpu_info", "header.suffix", "") << r;
                            lp.push(ss.str());
                        }

                        for (size_t i = 0; i < all_gpu_info.size(); ++i) {
                            auto& g = all_gpu_info[i];

                            // GPU index line
                            if (config.getNestedBool("gpu_info", "gpu_header.show", true)) {
                                ostringstream label;
                                if (i == 0) {
                                    label << config.getNestedColor("gpu_info", "gpu_header.index_color", "")
                                          << config.getLabel("gpu_info", "gpu_header.text", "GPU ") << (i + 1) << r;
                                }
                                else {
                                    label << config.getNestedColor("gpu_info", "gpu_header.prefix_color", "")
                                          << config.getPrefix("gpu_info", "gpu_header.prefix", "") << r
                                          << config.getNestedColor("gpu_info", "gpu_header.index_color", "")
                                          << config.getLabel("gpu_info", "gpu_header.text", "GPU ") << (i + 1) << r
                                          << config.getNestedColor("gpu_info", "gpu_header.suffix_color", "")
                                          << config.getPrefix("gpu_info", "gpu_header.suffix", "") << r;
                                }

                                string lbl = label.str();
                                if (lbl.length() < 27) lbl += string(27 - lbl.length(), ' ');
                                lp.push(lbl);
                            }

                            // Name
                            if (config.getNestedBool("gpu_info", "fields.name.show", true)) {
                                ostringstream ss;
                                ss << config.getNestedColor("gpu_info", "fields.name.name_prefix_color", "")
                                   << config.getPrefix("gpu_info", "fields.name.name_prefix", "") << r
                                   << config.getNestedColor("gpu_info", "fields.name.label_color", "")
                                   << left << setw(23) << config.getLabel("gpu_info", "fields.name.label", "") << r
                                   << config.getNestedColor("gpu_info", "separator.color", "")
                                   << config.getPrefix("gpu_info", "separator.text", "") << " " << r
                                   << config.getNestedColor("gpu_info", "fields.name.value_color", "")
                                   << g.gpu_name << r;
                                lp.push(ss.str());
                            }

                            // Memory
                            if (config.getNestedBool("gpu_info", "fields.memory.show", true)) {
                                ostringstream ss;
                                ss << config.getNestedColor("gpu_info", "fields.memory.memory_prefix_color", "")
                                   << config.getPrefix("gpu_info", "fields.memory.memory_prefix", "") << r
                                   << config.getNestedColor("gpu_info", "fields.memory.label_color", "")
                                   << left << setw(23) << config.getLabel("gpu_info", "fields.memory.label", "") << r
                                   << config.getNestedColor("gpu_info", "separator.color", "")
                                   << config.getPrefix("gpu_info", "separator.text", "") << " " << r
                                   << config.getNestedColor("gpu_info", "fields.memory.value_color", "")
                                   << g.gpu_memory << r;
                                lp.push(ss.str());
                            }

                            // Usage
                            if (config.getNestedBool("gpu_info", "fields.usage.show", true)) {
                                ostringstream ss;
                                ss << config.getNestedColor("gpu_info", "fields.usage.usage_prefix_color", "")
                                   << config.getPrefix("gpu_info", "fields.usage.usage_prefix", "") << r
                                   << config.getNestedColor("gpu_info", "fields.usage.label_color", "")
                                   << left << setw(23) << config.getLabel("gpu_info", "fields.usage.label", "") << r
                                   << config.getNestedColor("gpu_info", "separator.color", "")
                                   << config.getPrefix("gpu_info", "separator.text", "") << " " << r
                                   << config.getNestedColor("gpu_info", "fields.usage.value_color", "")
                                   << g.gpu_usage << r
                                   << config.getNestedColor("gpu_info", "percent_sign.color", "")
                                   << config.getPrefix("gpu_info", "percent_sign.text", "%") << r;
                                lp.push(ss.str());
                            }

                            // Vendor
                            if (config.getNestedBool("gpu_info", "fields.vendor.show", true)) {
                                ostringstream ss;
                                ss << config.getNestedColor("gpu_info", "fields.vendor.vendor_prefix_color", "")
                                   << config.getPrefix("gpu_info", "fields.vendor.vendor_prefix", "") << r
                                   << config.getNestedColor("gpu_info", "fields.vendor.label_color", "")
                                   << left << setw(23) << config.getLabel("gpu_info", "fields.vendor.label", "") << r
                                   << config.getNestedColor("gpu_info", "separator.color", "")
                                   << config.getPrefix("gpu_info", "separator.text", "") << " " << r
                                   << config.getNestedColor("gpu_info", "fields.vendor.value_color", "")
                                   << g.gpu_vendor << r;
                                lp.push(ss.str());
                            }

                            // Driver Version
                            if (config.getNestedBool("gpu_info", "fields.driver.show", true)) {
                                ostringstream ss;
                                ss << config.getNestedColor("gpu_info", "fields.driver.driver_prefix_color", "")
                                   << config.getPrefix("gpu_info", "fields.driver.driver_prefix", "") << r
                                   << config.getNestedColor("gpu_info", "fields.driver.label_color", "")
                                   << left << setw(23) << config.getLabel("gpu_info", "fields.driver.label", "") << r
                                   << config.getNestedColor("gpu_info", "separator.color", "")
                                   << config.getPrefix("gpu_info", "separator.text", "") << " " << r
                                   << config.getNestedColor("gpu_info", "fields.driver.value_color", "")
                                   << g.gpu_driver_version << r;
                                lp.push(ss.str());
                            }

                            // Temperature
                            if (config.getNestedBool("gpu_info", "fields.temperature.show", true)) {
                                ostringstream ss;
                                ss << config.getNestedColor("gpu_info", "fields.temperature.temperature_prefix_color", "")
                                   << config.getPrefix("gpu_info", "fields.temperature.temperature_prefix", "") << r
                                   << config.getNestedColor("gpu_info", "fields.temperature.label_color", "")
                                   << left << setw(23) << config.getLabel("gpu_info", "fields.temperature.label", "") << r
                                   << config.getNestedColor("gpu_info", "separator.color", "")
                                   << config.getPrefix("gpu_info", "separator.text", "") << " " << r
                                   << config.getNestedColor("gpu_info", "fields.temperature.value_color", "")
                                   << g.gpu_temperature << r
                                   << config.getColor("gpu_info", "unit_color", "white") << " C" << r;
                                lp.push(ss.str());
                            }

                            // Core Count
                            if (config.getNestedBool("gpu_info", "fields.cores.show", true)) {
                                ostringstream ss;
                                ss << config.getNestedColor("gpu_info", "fields.cores.cores_prefix_color", "")
                                   << config.getPrefix("gpu_info", "fields.cores.cores_prefix", "") << r
                                   << config.getNestedColor("gpu_info", "fields.cores.label_color", "")
                                   << left << setw(23) << config.getLabel("gpu_info", "fields.cores.label", "") << r
                                   << config.getNestedColor("gpu_info", "separator.color", "")
                                   << config.getPrefix("gpu_info", "separator.text", "") << " " << r
                                   << config.getNestedColor("gpu_info", "fields.cores.value_color", "")
                                   << g.gpu_core_count << r;
                                lp.push(ss.str());
                            }
                        }

                        // Primary GPU Details
                        auto primary = detailed_gpu_info.primary_gpu_info();
                        if (config.getNestedBool("gpu_info", "primary_header.show", true)) {
                            lp.push("");
                            ostringstream ss;
                            ss << config.getNestedColor("gpu_info", "primary_header.prefix_color", "")
                               << config.getPrefix("gpu_info", "primary_header.prefix", "") << r
                               << config.getNestedColor("gpu_info", "primary_header.text_color", "")
                               << config.getLabel("gpu_info", "primary_header.text", "") << r
                               << config.getNestedColor("gpu_info", "primary_header.suffix_color", "")
                               << config.getPrefix("gpu_info", "primary_header.suffix", "") << r;
                            lp.push(ss.str());

                            // Primary Name
                            {
                                ostringstream ss;
                                ss << config.getNestedColor("gpu_info", "primary_fields.name.name_prefix_color", "")
                                   << config.getPrefix("gpu_info", "primary_fields.name.name_prefix", "") << r
                                   << config.getNestedColor("gpu_info", "primary_fields.name.label_color", "")
                                   << left << setw(23) << config.getLabel("gpu_info", "primary_fields.name.label", "") << r
                                   << config.getNestedColor("gpu_info", "separator.color", "")
                                   << config.getPrefix("gpu_info", "separator.text", "") << " " << r
                                   << config.getNestedColor("gpu_info", "fields.name.value_color", "")
                                   << primary.name << r;
                                lp.push(ss.str());
                            }
                            // Primary VRAM
                            {
                                ostringstream ss;
                                ss << config.getNestedColor("gpu_info", "primary_fields.vram.vram_prefix_color", "")
                                   << config.getPrefix("gpu_info", "primary_fields.vram.vram_prefix", "") << r
                                   << config.getNestedColor("gpu_info", "primary_fields.vram.label_color", "")
                                   << left << setw(23) << config.getLabel("gpu_info", "primary_fields.vram.label", "") << r
                                   << config.getNestedColor("gpu_info", "separator.color", "")
                                   << config.getPrefix("gpu_info", "separator.text", "") << " " << r
                                   << config.getNestedColor("gpu_info", "fields.memory.value_color", "")
                                   << primary.vram_gb << r
                                   << config.getColor("gpu_info", "unit_color", "white") << " GiB" << r;
                                lp.push(ss.str());
                            }
                            // Primary Frequency
                            {
                                ostringstream ss;
                                ss << config.getNestedColor("gpu_info", "primary_fields.freq.freq_prefix_color", "")
                                   << config.getPrefix("gpu_info", "primary_fields.freq.freq_prefix", "") << r
                                   << config.getNestedColor("gpu_info", "primary_fields.freq.label_color", "")
                                   << left << setw(23) << config.getLabel("gpu_info", "primary_fields.freq.label", "") << r
                                   << config.getNestedColor("gpu_info", "separator.color", "")
                                   << config.getPrefix("gpu_info", "separator.text", "") << " " << r
                                   << config.getNestedColor("gpu_info", "primary_fields.freq.value_color", "")
                                   << primary.frequency_ghz << r
                                   << config.getColor("gpu_info", "unit_color", "white") << " GHz" << r;
                                lp.push(ss.str());
                            }
                        }
                    }
                }

        // ================= DISPLAY INFO (FULLY JSON DRIVEN) =================
        if (config.isEnabled("display_info")) {
            lp.push("");

            const auto& screens = di.getScreens();

            for (size_t i = 0; i < screens.size(); ++i) {
                const auto& s = screens[i];

                // ---------- Display Banner ----------
                if (config.isSubEnabled("display_info", "show_display_banner")) {
                    ostringstream ss;
                    ss << config.getColor("display_info", "#-", "blue") << config.getPrefix("display_info", "header", "#- ") << r
                        << config.getColor("display_info", "display_banner_text", "cyan")
                        << config.getLabel("display_info", "header", "Display ") << (i + 1) << " " << r
                        << config.getColor("display_info", "display_banner_line", "red")
                        << config.getPrefix("display_info", "suffix", "------------------------------------------------------#") << r;
                    lp.push(ss.str());
                }

                // ---------- Name ----------
                if (config.isSubEnabled("display_info", "show_name")) {
                    ostringstream ss;
                    ss << config.getColor("display_info", "|->", "cyan") << config.getPrefix("display_info", "item", "|-> ") << r
                        << config.getColor("display_info", "name_label_color", "blue")
                        << left << setw(23) << config.getLabel("display_info", "name", "Name") << r
                        << config.getColor("display_info", ":", "blue") << ": " << r
                        << config.getColor("display_info", "name_value_color", "cyan") << s.name << r;
                    lp.push(ss.str());
                }

                // ---------- Applied Resolution ----------
                if (config.isSubEnabled("display_info", "show_applied_resolution")) {
                    ostringstream ss;
                    ss << config.getColor("display_info", "|->", "cyan") << config.getPrefix("display_info", "item", "|-> ") << r
                        << config.getColor("display_info", "applied_res_label_color", "blue")
                        << left << setw(23) << config.getLabel("display_info", "applied_res", "Applied Resolution") << r
                        << config.getColor("display_info", ":", "blue") << ": " << r
                        << config.getColor("display_info", "applied_res_value_color", "cyan")
                        << s.current_width
                        << config.getColor("display_info", "x", "blue") << "x"
                        << s.current_height
                        << config.getColor("display_info", "@", "blue") << " @"
                        << s.refresh_rate
                        << config.getColor("display_info", "hz_color", "red") << "Hz" << r;
                    lp.push(ss.str());
                }

                // ---------- Native Resolution ----------
                if (config.isSubEnabled("display_info", "show_native_resolution")) {
                    ostringstream ss;
                    ss << config.getColor("display_info", "|->", "cyan") << config.getPrefix("display_info", "item", "|-> ") << r
                        << config.getColor("display_info", "native_res_label_color", "blue")
                        << left << setw(23) << config.getLabel("display_info", "native_res", "Native Resolution") << r
                        << config.getColor("display_info", ":", "blue") << ": " << r
                        << config.getColor("display_info", "native_res_value_color", "cyan")
                        << s.native_resolution << r;
                    lp.push(ss.str());
                }

                // ---------- Aspect Ratio ----------
                if (config.isSubEnabled("display_info", "show_aspect_ratio")) {
                    ostringstream ss;
                    ss << config.getColor("display_info", "|->", "cyan") << config.getPrefix("display_info", "item", "|-> ") << r
                        << config.getColor("display_info", "aspect_ratio_label_color", "blue")
                        << left << setw(23) << config.getLabel("display_info", "aspect_ratio", "Aspect Ratio") << r
                        << config.getColor("display_info", ":", "blue") << ": " << r
                        << config.getColor("display_info", "aspect_ratio_value_color", "cyan")
                        << s.aspect_ratio << r;
                    lp.push(ss.str());
                }

                // ---------- Scaling ----------
                if (config.isSubEnabled("display_info", "show_scaling")) {
                    ostringstream ss;
                    ss << config.getColor("display_info", "|->", "cyan") << config.getPrefix("display_info", "item", "|-> ") << r
                        << config.getColor("display_info", "scaling_label_color", "blue")
                        << left << setw(23) << config.getLabel("display_info", "scaling", "Scaling") << r
                        << config.getColor("display_info", ":", "blue") << ": " << r
                        << config.getColor("display_info", "scaling_value_color", "cyan")
                        << s.scale_percent
                        << config.getColor("display_info", "%", "blue") << "%" << r;
                    lp.push(ss.str());
                }

                // ---------- Upscale ----------
                if (config.isSubEnabled("display_info", "show_upscale")) {
                    ostringstream ss;
                    ss << config.getColor("display_info", "|->", "cyan") << config.getPrefix("display_info", "item", "|-> ") << r
                        << config.getColor("display_info", "upscale_label_color", "blue")
                        << left << setw(23) << config.getLabel("display_info", "upscale", "Upscale") << r
                        << config.getColor("display_info", ":", "blue") << ": " << r
                        << config.getColor("display_info", "upscale_value_color", "cyan")
                        << s.upscale << r;
                    lp.push(ss.str());
                }

                // ---------- DSR / VSR ----------
                if (config.isSubEnabled("display_info", "show_dsr")) {
                    ostringstream ss;
                    ss << config.getColor("display_info", "|->", "cyan") << config.getPrefix("display_info", "item", "|-> ") << r
                        << config.getColor("display_info", "dsr_label_color", "blue")
                        << left << setw(23) << config.getLabel("display_info", "dsr", "DSR / VSR") << r
                        << config.getColor("display_info", ":", "blue") << ": " << r
                        << config.getColor(
                            "display_info",
                            s.dsr_enabled ? "dsr_enabled_color" : "dsr_disabled_color",
                            s.dsr_enabled ? "green" : "red"
                        )
                        << (s.dsr_enabled ? "Enabled" : "Disabled") << r
                        << config.getColor("display_info", "dsr_brackets_color", "blue")
                        << " (" << r
                        << config.getColor("display_info", "dsr_type_color", "cyan")
                        << s.dsr_type
                        << config.getColor("display_info", "dsr_brackets_color", "blue")
                        << ")" << r;
                    lp.push(ss.str());
                }

                lp.push("");
            }
        }

        // BIOS & Motherboard Info (JSON Driven)
        if (config.isEnabled("bios_mb_info")) {
            lp.push("");

            // Header
            if (config.isSubEnabled("bios_mb_info", "show_header")) {
                ostringstream ss;
                ss << config.getColor("bios_mb_info", "#-", "white") << config.getPrefix("bios_mb_info", "header", "#- ") << r
                    << config.getColor("bios_mb_info", "header_text_color", "white") << config.getLabel("bios_mb_info", "header", "BIOS & Motherboard Info ") << r
                    << config.getColor("bios_mb_info", "separator_line", "white")
                    << config.getPrefix("bios_mb_info", "suffix", "----------------------------------------#") << r;
                lp.push(ss.str());
            }

            // Bios Vendor
            if (config.isSubEnabled("bios_mb_info", "show_bios_vendor")) {
                ostringstream ss;
                ss << config.getColor("bios_mb_info", "~", "white") << config.getPrefix("bios_mb_info", "item", "~ ") << r
                    << config.getColor("bios_mb_info", "vendor_label_color", "white")
                    << left << setw(26) << config.getLabel("bios_mb_info", "vendor", "Bios Vendor") << r
                    << config.getColor("bios_mb_info", ":", "white") << ": " << r
                    << config.getColor("bios_mb_info", "vendor_value_color", "white") << sys.get_bios_vendor() << r;
                lp.push(ss.str());
            }

            // Bios Version
            if (config.isSubEnabled("bios_mb_info", "show_bios_version")) {
                ostringstream ss;
                ss << config.getColor("bios_mb_info", "~", "white") << config.getPrefix("bios_mb_info", "item", "~ ") << r
                    << config.getColor("bios_mb_info", "version_label_color", "white")
                    << left << setw(26) << config.getLabel("bios_mb_info", "version", "Bios Version") << r
                    << config.getColor("bios_mb_info", ":", "white") << ": " << r
                    << config.getColor("bios_mb_info", "version_value_color", "white") << sys.get_bios_version() << r;
                lp.push(ss.str());
            }

            // Bios Date
            if (config.isSubEnabled("bios_mb_info", "show_bios_date")) {
                ostringstream ss;
                ss << config.getColor("bios_mb_info", "~", "white") << config.getPrefix("bios_mb_info", "item", "~ ") << r
                    << config.getColor("bios_mb_info", "date_label_color", "white")
                    << left << setw(26) << config.getLabel("bios_mb_info", "date", "Bios Date") << r
                    << config.getColor("bios_mb_info", ":", "white") << ": " << r
                    << config.getColor("bios_mb_info", "date_value_color", "white") << sys.get_bios_date() << r;
                lp.push(ss.str());
            }

            // Motherboard Model
            if (config.isSubEnabled("bios_mb_info", "show_mb_model")) {
                ostringstream ss;
                ss << config.getColor("bios_mb_info", "~", "white") << config.getPrefix("bios_mb_info", "item", "~ ") << r
                    << config.getColor("bios_mb_info", "model_label_color", "white")
                    << left << setw(26) << config.getLabel("bios_mb_info", "model", "Motherboard Model") << r
                    << config.getColor("bios_mb_info", ":", "white") << ": " << r
                    << config.getColor("bios_mb_info", "model_value_color", "white") << sys.get_motherboard_model() << r;
                lp.push(ss.str());
            }

            // Motherboard Manufacturer
            if (config.isSubEnabled("bios_mb_info", "show_mb_manufacturer")) {
                ostringstream ss;
                ss << config.getColor("bios_mb_info", "~", "white") << config.getPrefix("bios_mb_info", "item", "~ ") << r
                    << config.getColor("bios_mb_info", "mfg_label_color", "white")
                    << left << setw(26) << config.getLabel("bios_mb_info", "mfg", "Motherboard Manufacturer") << r
                    << config.getColor("bios_mb_info", ":", "white") << ": " << r
                    << config.getColor("bios_mb_info", "mfg_value_color", "white") << sys.get_motherboard_manufacturer() << r;
                lp.push(ss.str());
            }
        }

        // User Info (JSON Driven)
        if (config.isEnabled("user_info")) {
            lp.push("");

            // Header
            if (config.isSubEnabled("user_info", "show_header")) {
                ostringstream ss;
                ss << config.getColor("user_info", "#-", "white") << config.getPrefix("user_info", "header", "#- ") << r
                    << config.getColor("user_info", "header_text_color", "white") << config.getLabel("user_info", "header", "User Info ") << r
                    << config.getColor("user_info", "separator_line", "white")
                    << config.getPrefix("user_info", "suffix", "------------------------------------------------------#") << r;
                lp.push(ss.str());
            }

            // Username
            if (config.isSubEnabled("user_info", "show_username")) {
                ostringstream ss;
                ss << config.getColor("user_info", "~", "white") << config.getPrefix("user_info", "item", "~ ") << r
                    << config.getColor("user_info", "username_label_color", "white")
                    << left << setw(26) << config.getLabel("user_info", "username", "Username") << r
                    << config.getColor("user_info", ":", "white") << ": " << r
                    << config.getColor("user_info", "username_value_color", "white") << user.get_username() << r;
                lp.push(ss.str());
            }

            // Computer Name
            if (config.isSubEnabled("user_info", "show_computer_name")) {
                ostringstream ss;
                ss << config.getColor("user_info", "~", "white") << config.getPrefix("user_info", "item", "~ ") << r
                    << config.getColor("user_info", "computer_name_label_color", "white")
                    << left << setw(26) << config.getLabel("user_info", "computer_name", "Computer Name") << r
                    << config.getColor("user_info", ":", "white") << ": " << r
                    << config.getColor("user_info", "computer_name_value_color", "white") << user.get_computer_name() << r;
                lp.push(ss.str());
            }

            // Domain
            if (config.isSubEnabled("user_info", "show_domain")) {
                ostringstream ss;
                ss << config.getColor("user_info", "~", "white") << config.getPrefix("user_info", "item", "~ ") << r
                    << config.getColor("user_info", "domain_label_color", "white")
                    << left << setw(26) << config.getLabel("user_info", "domain", "Domain") << r
                    << config.getColor("user_info", ":", "white") << ": " << r
                    << config.getColor("user_info", "domain_value_color", "white") << user.get_domain_name() << r;
                lp.push(ss.str());
            }
        }

        // Performance Info (JSON Driven)
        if (config.isEnabled("performance_info")) {
            lp.push("");

            // Header
            if (config.isSubEnabled("performance_info", "show_header")) {
                ostringstream ss;
                ss << config.getColor("performance_info", "#-", "white") << config.getPrefix("performance_info", "header", "#- ") << r
                    << config.getColor("performance_info", "header_text_color", "white") << config.getLabel("performance_info", "header", "Performance Info ") << r
                    << config.getColor("performance_info", "separator_line", "white")
                    << config.getPrefix("performance_info", "suffix", "-----------------------------------------------#") << r;
                lp.push(ss.str());
            }

            // System Uptime
            if (config.isSubEnabled("performance_info", "show_uptime")) {
                ostringstream ss;
                ss << config.getColor("performance_info", "~", "white") << config.getPrefix("performance_info", "item", "~ ") << r
                    << config.getColor("performance_info", "uptime_label_color", "white")
                    << left << setw(26) << config.getLabel("performance_info", "uptime", "System Uptime") << r
                    << config.getColor("performance_info", ":", "white") << ": " << r
                    << config.getColor("performance_info", "uptime_value_color", "white") << perf.get_system_uptime() << r;
                lp.push(ss.str());
            }

            // CPU Usage
            if (config.isSubEnabled("performance_info", "show_cpu_usage")) {
                ostringstream ss;
                ss << config.getColor("performance_info", "~", "white") << config.getPrefix("performance_info", "item", "~ ") << r
                    << config.getColor("performance_info", "cpu_usage_label_color", "white")
                    << left << setw(26) << config.getLabel("performance_info", "cpu_usage", "CPU Usage") << r
                    << config.getColor("performance_info", ":", "white") << ": " << r
                    << config.getColor("performance_info", "usage_value_color", "white") << perf.get_cpu_usage_percent() << r
                    << config.getColor("performance_info", "%", "white") << "%" << r;
                lp.push(ss.str());
            }

            // RAM Usage
            if (config.isSubEnabled("performance_info", "show_ram_usage")) {
                ostringstream ss;
                ss << config.getColor("performance_info", "~", "white") << config.getPrefix("performance_info", "item", "~ ") << r
                    << config.getColor("performance_info", "ram_usage_label_color", "white")
                    << left << setw(26) << config.getLabel("performance_info", "ram_usage", "RAM Usage") << r
                    << config.getColor("performance_info", ":", "white") << ": " << r
                    << config.getColor("performance_info", "usage_value_color", "white") << perf.get_ram_usage_percent() << r
                    << config.getColor("performance_info", "%", "white") << "%" << r;
                lp.push(ss.str());
            }

            // Disk Usage
            if (config.isSubEnabled("performance_info", "show_disk_usage")) {
                ostringstream ss;
                ss << config.getColor("performance_info", "~", "white") << config.getPrefix("performance_info", "item", "~ ") << r
                    << config.getColor("performance_info", "disk_usage_label_color", "white")
                    << left << setw(26) << config.getLabel("performance_info", "disk_usage", "Disk Usage") << r
                    << config.getColor("performance_info", ":", "white") << ": " << r
                    << config.getColor("performance_info", "usage_value_color", "white") << perf.get_disk_usage_percent() << r
                    << config.getColor("performance_info", "%", "white") << "%" << r;
                lp.push(ss.str());
            }

            // GPU Usage
            if (config.isSubEnabled("performance_info", "show_gpu_usage")) {
                ostringstream ss;
                ss << config.getColor("performance_info", "~", "white") << config.getPrefix("performance_info", "item", "~ ") << r
                    << config.getColor("performance_info", "gpu_usage_label_color", "white")
                    << left << setw(26) << config.getLabel("performance_info", "gpu_usage", "GPU Usage") << r
                    << config.getColor("performance_info", ":", "white") << ": " << r
                    << config.getColor("performance_info", "usage_value_color", "white") << perf.get_gpu_usage_percent() << r
                    << config.getColor("performance_info", "%", "white") << "%" << r;
                lp.push(ss.str());
            }
        }

        // Audio & Power Info (JSON Driven)
        if (config.isEnabled("audio_power_info")) {
            lp.push("");
            ExtraInfo audio;

            // --- Output devices ---
            vector<AudioDevice> outputDevices = audio.get_output_devices();

            if (config.isSubEnabled("audio_power_info", "show_output_header")) {
                ostringstream ss;
                ss << config.getColor("audio_power_info", "#-", "white") << config.getPrefix("audio_power_info", "audio_output_header", "#- ") << r
                    << config.getColor("audio_power_info", "header_text_color", "white") << config.getLabel("audio_power_info", "audio_output_header", "Audio Output ") << r
                    << config.getColor("audio_power_info", "separator_line", "white")
                    << config.getPrefix("audio_power_info", "audio_output_suffix", "---------------------------------------------------#") << r;
                lp.push(ss.str());
            }

            int audio_output_device_count = 0;
            for (const auto& device : outputDevices) {
                audio_output_device_count++;
                ostringstream oss;
                oss << config.getColor("audio_power_info", "~", "white") << config.getPrefix("audio_power_info", "item", "~ ") << r
                    << config.getColor("audio_power_info", "index_color", "white") << audio_output_device_count << r << " "
                    << config.getColor("audio_power_info", "device_name_color", "white") << device.name << r;

                if (device.isActive && config.isSubEnabled("audio_power_info", "show_active_status")) {
                    oss << " " << config.getColor("audio_power_info", "active_label_color", "white") << "(active)" << r;
                }
                lp.push(oss.str());
            }

            // --- Input devices ---
            vector<AudioDevice> inputDevices = audio.get_input_devices();

            if (config.isSubEnabled("audio_power_info", "show_input_header")) {
                ostringstream ss;
                ss << config.getColor("audio_power_info", "#-", "white") << config.getPrefix("audio_power_info", "audio_input_header", "#- ") << r
                    << config.getColor("audio_power_info", "header_text_color", "white") << config.getLabel("audio_power_info", "audio_input_header", "Audio Input ") << r
                    << config.getColor("audio_power_info", "separator_line", "white")
                    << config.getPrefix("audio_power_info", "audio_input_suffix", "----------------------------------------------------#") << r;
                lp.push(ss.str());
            }

            int audio_input_device_count = 0;
            for (const auto& device : inputDevices) {
                audio_input_device_count++;
                ostringstream oss;
                oss << config.getColor("audio_power_info", "~", "white") << config.getPrefix("audio_power_info", "item", "~ ") << r
                    << config.getColor("audio_power_info", "index_color", "white") << audio_input_device_count << r << " "
                    << config.getColor("audio_power_info", "device_name_color", "white") << device.name << r;

                if (device.isActive && config.isSubEnabled("audio_power_info", "show_active_status")) {
                    oss << " " << config.getColor("audio_power_info", "active_label_color", "white") << "(active)" << r;
                }
                lp.push(oss.str());
            }

            // --- Power Status ---
            if (config.isSubEnabled("audio_power_info", "show_power_info")) {
                lp.push("");
                PowerStatus power = audio.get_power_status();

                if (config.isSubEnabled("audio_power_info", "show_power_header")) {
                    ostringstream ss;
                    ss << config.getColor("audio_power_info", "#-", "white") << config.getPrefix("audio_power_info", "power_header", "#- ") << r
                        << config.getColor("audio_power_info", "header_text_color", "white") << config.getLabel("audio_power_info", "power_header", "Power  ") << r
                        << config.getColor("audio_power_info", "separator_line", "white")
                        << config.getPrefix("audio_power_info", "power_suffix", "---------------------------------------------------------#") << r;
                    lp.push(ss.str());
                }

                ostringstream ossPower;
                if (!power.hasBattery) {
                    ossPower << config.getColor("audio_power_info", "bracket_color", "white") << "[" << r
                        << config.getColor("audio_power_info", "wired_text_color", "white") << "Wired connection" << r
                        << config.getColor("audio_power_info", "bracket_color", "white") << "]" << r;
                }
                else {
                    ossPower << config.getColor("audio_power_info", "~", "white") << config.getPrefix("audio_power_info", "item", "~ ") << r
                        << config.getColor("audio_power_info", "battery_label_color", "white") << "Battery powered " << r
                        << config.getColor("audio_power_info", "bracket_color", "white") << "(" << r
                        << config.getColor("audio_power_info", "battery_percent_color", "white") << power.batteryPercent << r
                        << config.getColor("audio_power_info", "unit_color", "white") << "%)" << r;

                    if (power.isCharging) {
                        ossPower << " " << config.getColor("audio_power_info", "charging_status_color", "white") << "(Charging)" << r;
                    }
                    else {
                        ossPower << " " << config.getColor("audio_power_info", "not_charging_status_color", "white") << "(Not Charging)" << r;
                    }
                }
                lp.push(ossPower.str());
            }
        }



    //----------------- END OF JSON-CONTROLLED COMPACT SECTIONS -----------------//

 

    // ---------------- End of info lines ----------------

    // Print remaining ASCII art lines (if art is taller than info)
    lp.finish();

    cout << endl;




    // End of CoUninitialize 
    
    // Uninitializes the COM library for the current thread, releasing 
    // resources allocated by COM and cleaning up any COM-related 
    // state. Should be called once for every successful CoInitialize() 
    // or CoInitializeEx() call to avoid memory/resource leaks.

    return 0;
}



/*
===============================================================================
                    BINARYFETCH MAIN.CPP - COMPREHENSIVE DOCUMENTATION
===============================================================================

OVERVIEW:
---------
BinaryFetch is a Windows system information tool that displays comprehensive
hardware, software, and performance data in both compact and detailed formats.
This main.cpp file serves as the central orchestrator, managing:
1. Module initialization and data collection
2. Configuration loading and JSON-driven output control
3. Live printing with ASCII art integration
4. Color-coded output formatting

KEY ARCHITECTURAL PRINCIPLES:
-----------------------------
- main() acts as an orchestrator/controller ONLY
- All logic resides in dedicated modules/classes
- No heavy calculations or system queries directly in main()
- Output formatting controlled entirely via LivePrinter
- Configuration-driven display via JSON

COMPONENT STRUCTURE:
====================

I. HEADER INCLUDES
------------------

A. SYSTEM HEADERS:
   - <iostream>, <iomanip>, <vector>, <functional>, <sstream>
   - <fstream>, <string>, <regex>
   - Windows API: <windows.h>, <shlobj.h>, <direct.h>
   - COM/WMI: <comdef.h>, <Wbemidl.h>

B. ASCII ART MODULE:
   - #include "AsciiArt.h" - Handles ASCII art loading and display

C. FULL SYSTEM INFO MODULES (Detailed Mode):
   - OSInfo.h          - OS name, version, build, architecture, uptime
   - CPUInfo.h         - CPU model, cores, threads, clocks, cache
   - MemoryInfo.h      - RAM capacity, usage, modules, speed, type
   - GPUInfo.h         - Basic GPU information (name, memory, usage)
   - DetailedGPUInfo.h - Advanced GPU details (VRAM, clocks, temps)
   - StorageInfo.h     - Disk drives, partitions, usage, performance
   - NetworkInfo.h     - Network adapters, IP, speeds, MAC
   - PerformanceInfo.h - Real-time CPU/RAM/GPU/Disk usage
   - UserInfo.h        - Username, PC name, domain, admin status
   - SystemInfo.h      - Motherboard, BIOS, manufacturer
   - DisplayInfo.h     - Monitor resolution, refresh, scaling
   - ExtraInfo.h       - Audio devices, power status
   - DetailedScreen.h  - EDID, PPI, HDR, detailed display info

D. COMPACT MODE MODULES:
   - CompactAudio.h      - Audio device summary
   - CompactOS.h         - Lightweight OS summary
   - CompactCPU.h        - Lightweight CPU summary
   - CompactMemory.h     - Lightweight RAM summary
   - CompactScreen.h     - Screen resolution/refresh summary
   - CompactSystem.h     - Motherboard/system summary
   - CompactGPU.h        - Lightweight GPU summary
   - CompactPerformance.h - Performance stats
   - CompactUser.h       - User info summary
   - CompactNetwork.h    - Network info summary
   - compact_disk_info.h - Storage summary
   - TimeInfo.h          - Current time/date information

E. THIRD-PARTY:
   - nlohmann/json.hpp - JSON parsing and manipulation

II. JSON CONFIGURATION SYSTEM:
-------------------------------

A. CONFIG FILE LOCATIONS:
   1. Development Mode (LOAD_DEFAULT_CONFIG=true):
      - Loads from: "Default_BinaryFetch_Config.json" (project folder)
   2. Production Mode (LOAD_DEFAULT_CONFIG=false):
      - User config: C:\Users\Public\BinaryFetch\BinaryFetch_Config.json
      - Self-healing: If missing, extracts from EXE resource (IDR_DEFAULT_CONFIG=101)

B. CONFIG STRUCTURE HIERARCHY:
   {
     "section_name": {
       "enabled": true/false,
       "colors": {
         "key": "color_name"
       },
       "sections": {
         "subsection": {
           "enabled": true/false,
           "colors": {
             "nested_key": "color_name"
           }
         }
       }
     }
   }

C. COLOR SUPPORT:
   Available colors: red, green, yellow, blue, magenta, cyan, white,
   bright_red, bright_green, bright_yellow, bright_blue, bright_magenta,
   bright_cyan, bright_white, reset

D. HELPER LAMBDA FUNCTIONS:
   1. getColor() - Retrieves color codes from JSON config
   2. isEnabled() - Checks if a main section is enabled
   3. isSubEnabled() - Checks if subsection is enabled
   4. isSectionEnabled() - Checks nested sections
   5. isNestedEnabled() - Deep nested configuration checking

III. LIVE PRINTER SYSTEM:
-------------------------

A. PURPOSE:
   - Synchronizes ASCII art with system info output
   - Maintains column alignment across both text and art
   - Handles real-time streaming of formatted output

B. KEY METHODS (AsciiArt class):
   1. loadFromFile() - Loads ASCII art from user config location
      - Checks: C:\Users\<User>\AppData\BinaryFetch\BinaryArt.txt
      - Falls back to Default_Ascii_Art.txt if missing
      - Auto-creates directory and file if needed

   2. LivePrinter.push() - Adds formatted line to output queue
   3. LivePrinter.finish() - Prints remaining ASCII art lines

IV. MODULE FUNCTIONALITY SUMMARY:
---------------------------------

A. TIME MODULES:
   - TimeInfo: Returns current time info (second, minute, hour, day, week, month, year, leap year)

B. COMPACT MODULES (Single-line summaries):
   1. CompactOS: OS name, build, architecture, uptime
   2. CompactCPU: CPU name, cores/threads, clock speed
   3. CompactGPU: GPU name, usage, VRAM, frequency
   4. CompactScreen: Multi-display detection with resolution, scale, refresh
   5. CompactMemory: Total/free RAM, usage percentage
   6. CompactAudio: Active input/output audio devices
   7. CompactPerformance: CPU/GPU/RAM/Disk usage percentages
   8. CompactUser: Username, domain, admin status
   9. CompactNetwork: Network name, type, IP address
   10. DiskInfo: Disk usage percentages and capacities

C. DETAILED MODULES (Multi-line expanded info):
   1. MemoryInfo: RAM modules with capacity, type, speed, usage
   2. StorageInfo: Comprehensive disk info with performance metrics
   3. NetworkInfo: Full network details including IPs, speeds, MAC
   4. OSInfo: Complete OS information including kernel, serial, install date
   5. CPUInfo: Detailed CPU specs including caches, virtualization, sockets
   6. GPUInfo: Multi-GPU support with vendor, driver, temperature
   7. DisplayInfo: Per-display details including resolution, scaling, DSR/VSR
   8. SystemInfo: BIOS and motherboard information
   9. UserInfo: User and computer identification
   10. PerformanceInfo: Real-time performance metrics
   11. ExtraInfo: Audio devices and power/battery status

V. OUTPUT SECTIONS ORGANIZATION:
--------------------------------

A. COMPACT MODE SECTIONS (Top-to-bottom flow):
   1. BinaryFetch Header
   2. Compact Time
   3. Compact OS
   4. Compact CPU
   5. Compact GPU
   6. Compact Screen
   7. Compact Memory
   8. Compact Audio
   9. Compact Performance
   10. Compact User
   11. Compact Network
   12. Compact Disk

B. DETAILED MODE SECTIONS:
   1. Detailed Memory
   2. Detailed Storage
   3. Network Info
   4. OS Info
   5. CPU Info
   6. GPU Info
   7. Display Info
   8. BIOS & Motherboard Info
   9. User Info
   10. Performance Info
   11. Audio & Power Info

VI. CONFIGURATION-DRIVEN FEATURES:
-----------------------------------

A. ENABLE/DISABLE CONTROL:
   Each section can be independently enabled/disabled via JSON config

B. COLOR CUSTOMIZATION:
   Every text element can have its color defined in JSON

C. SUB-SECTION CONTROL:
   Fine-grained control over individual data points within sections

D. EMOJI SUPPORT:
   UTF-8 emoji display with configurable on/off toggle

VII. DEVELOPER FEATURES:
------------------------

A. DUMMY DATA MODES: (now we can toggle these directly form the json config)
    user can access them too 
   - dummy_compact_network: Test compact network with fake data
   - dummy_detailed_network: Test detailed network with fake data

B. TESTING SITE:
   Designated area for quick testing (currently commented out)

C. COM INITIALIZATION:
   Proper COM library initialization for WMI queries

D. ERROR HANDLING:
   - Graceful degradation when modules fail
   - Config file fallback to defaults
   - ASCII art load failure doesn't crash program

VIII. OUTPUT FORMATTING PATTERNS:
---------------------------------

A. COMPACT MODE PATTERN:
   [Emoji] Label: Value (Additional Info) @ Unit

B. DETAILED MODE PATTERNS:
   1. Header: "#- Section Name ---------------------------------------#"
   2. Data Line: "~ Label: Value" or "|-> Label: Value"
   3. Sub-sections: "#-> Subsection Label: Value"

C. MEMORY MODULE FORMATTING:
   - Zero-padded capacity (02GB, 16GB, etc.)
   - Percentage-based usage display

D. STORAGE FORMATTING:
   - Fixed-width numeric alignment
   - GiB units for consistency
   - Performance prediction display

IX. KEY DATA STRUCTURES:
------------------------

A. storage_data (StorageInfo.h):
   - drive_letter, total_space, used_space, used_percentage
   - file_system, is_external, serial_number
   - read_speed, write_speed, predicted_read/write_speed
   - storage_type

B. AudioDevice (ExtraInfo.h):
   - name, isActive

C. PowerStatus (ExtraInfo.h):
   - hasBattery, batteryPercent, isCharging

D. ScreenInfo (DisplayInfo.h):
   - name, current_width/height, native_resolution
   - refresh_rate, aspect_ratio, scale_percent
   - upscale, dsr_enabled, dsr_type

X. PERFORMANCE CONSIDERATIONS:
-------------------------------

A. LAZY EVALUATION:
   - Modules only query data when their section is enabled
   - No unnecessary system calls

B. CACHING:
   - Some modules cache results for repeated access
   - Time-sensitive data (performance) queried real-time

C. MEMORY MANAGEMENT:
   - Proper COM initialization/deinitialization
   - RAII principles in module design

XI. EXTENSION GUIDELINES:
-------------------------

A. ADDING NEW MODULES:
   1. Create header/implementation files
   2. Include in appropriate section (compact/detailed)
   3. Add to initialization section in main()
   4. Create JSON configuration schema
   5. Add to output section with proper formatting

B. MODIFYING EXISTING MODULES:
   1. Update JSON configuration structure
   2. Modify helper functions as needed
   3. Update output formatting in main()

C. CONFIGURATION UPDATES:
   1. Update Default_BinaryFetch_Config.json
   2. Add resource to EXE (resource.h)
   3. Update self-healing extraction logic if needed

XII. KNOWN LIMITATIONS:
------------------------

A. PLATFORM: Windows only (uses Windows-specific APIs)
B. ADMIN PRIVILEGES: Some info requires admin rights
C. PERFORMANCE: Initial load may be slow on older systems
D. UNICODE: Some terminals may not display emojis properly

XIII. TROUBLESHOOTING:
----------------------

A. ASCII ART NOT DISPLAYING:
   1. Check C:\Users\<User>\AppData\BinaryFetch\BinaryArt.txt exists
   2. Verify file is not empty
   3. Check console supports UTF-8

B. MISSING INFORMATION:
   1. Verify module is enabled in config
   2. Check if admin privileges are needed
   3. Verify WMI services are running

C. COLOR ISSUES:
   1. Check terminal supports ANSI colors
   2. Verify color names in JSON are correct
   3. Reset code (\033[0m) should follow colored text


------------------------------------------------------------------------------





```
CLASS: AsciiArt
OBJECT: art
FUNCTIONS:
1. loadFromFile() - Loads ASCII art from user config folder, returns bool

CLASS: LivePrinter
OBJECT: lp
FUNCTIONS:
1. push(string) - Adds formatted line to output queue
2. finish() - Prints remaining ASCII art lines

CLASS: OSInfo
OBJECT: os
FUNCTIONS:
1. GetOSName() - Returns OS name (e.g., "Windows 11 Pro")
2. GetOSVersion() - Returns OS build version
3. GetOSArchitecture() - Returns architecture (e.g., "64-bit")
4. get_os_kernel_info() - Returns kernel information
5. get_os_uptime() - Returns system uptime as string
6. get_os_install_date() - Returns installation date
7. get_os_serial_number() - Returns OS serial number

CLASS: CPUInfo
OBJECT: cpu
FUNCTIONS:
1. get_cpu_info() - Returns CPU brand/model
2. get_cpu_utilization() - Returns CPU usage percentage
3. get_cpu_speed() - Returns current CPU speed
4. get_cpu_base_speed() - Returns base CPU speed
5. get_cpu_cores() - Returns number of physical cores
6. get_cpu_logical_processors() - Returns number of logical processors
7. get_cpu_sockets() - Returns number of CPU sockets
8. get_cpu_virtualization() - Returns virtualization support status
9. get_cpu_l1_cache() - Returns L1 cache size
10. get_cpu_l2_cache() - Returns L2 cache size
11. get_cpu_l3_cache() - Returns L3 cache size

CLASS: MemoryInfo
OBJECT: ram
FUNCTIONS:
1. getTotal() - Returns total RAM in GB
2. getFree() - Returns free RAM in GB
3. getUsedPercentage() - Returns RAM usage percentage
4. getModules() - Returns vector of RAM module information

CLASS: GPUInfo
OBJECT: obj_gpu
FUNCTIONS:
1. get_all_gpu_info() - Returns vector of all GPU information

STRUCT: GPUData (returned by get_all_gpu_info())
- gpu_name - GPU model name
- gpu_memory - VRAM information
- gpu_usage - GPU usage percentage
- gpu_vendor - GPU vendor
- gpu_driver_version - Driver version
- gpu_temperature - GPU temperature
- gpu_core_count - Number of cores

CLASS: DetailedGPUInfo
OBJECT: detailed_gpu_info
FUNCTIONS:
1. primary_gpu_info() - Returns detailed info about primary GPU
   - name - GPU name
   - vram_gb - VRAM in GB
   - frequency_ghz - Clock frequency in GHz

CLASS: StorageInfo
OBJECT: storage
FUNCTIONS:
1. process_storage_info(callback) - Processes all storage devices with callback

STRUCT: storage_data (passed to callback)
- drive_letter - Drive letter (e.g., "C:")
- total_space - Total space in GiB
- used_space - Used space in GiB
- used_percentage - Usage percentage
- file_system - File system type
- is_external - Boolean for external/internal
- serial_number - Disk serial number
- read_speed - Read speed in MB/s
- write_speed - Write speed in MB/s
- predicted_read_speed - Predicted read speed
- predicted_write_speed - Predicted write speed
- storage_type - Storage type (SSD/HDD/etc)

CLASS: NetworkInfo
OBJECT: net
FUNCTIONS:
1. get_network_name() - Returns network name
2. get_local_ip() - Returns local IP address
3. get_public_ip() - Returns public IP address
4. get_locale() - Returns system locale
5. get_mac_address() - Returns MAC address
6. get_network_upload_speed() - Returns upload speed
7. get_network_download_speed() - Returns download speed

CLASS: UserInfo
OBJECT: user
FUNCTIONS:
1. get_username() - Returns current username
2. get_computer_name() - Returns computer name
3. get_domain_name() - Returns domain name

CLASS: PerformanceInfo
OBJECT: perf
FUNCTIONS:
1. get_system_uptime() - Returns system uptime
2. get_cpu_usage_percent() - Returns CPU usage percentage
3. get_ram_usage_percent() - Returns RAM usage percentage
4. get_disk_usage_percent() - Returns disk usage percentage
5. get_gpu_usage_percent() - Returns GPU usage percentage

CLASS: DisplayInfo
OBJECT: di
FUNCTIONS:
1. getScreens() - Returns vector of screen information

STRUCT: ScreenInfo (returned by getScreens())
- name - Display name
- current_width - Current width resolution
- current_height - Current height resolution
- native_resolution - Native resolution
- refresh_rate - Refresh rate in Hz
- aspect_ratio - Aspect ratio
- scale_percent - Scaling percentage
- upscale - Upscale factor
- dsr_enabled - DSR/VSR enabled status
- dsr_type - DSR type

CLASS: ExtraInfo
OBJECT: extra
FUNCTIONS:
1. get_output_devices() - Returns vector of audio output devices
2. get_input_devices() - Returns vector of audio input devices
3. get_power_status() - Returns power/battery status

STRUCT: AudioDevice
- name - Device name
- isActive - Active status

STRUCT: PowerStatus
- hasBattery - Boolean for battery presence
- batteryPercent - Battery percentage
- isCharging - Charging status

CLASS: SystemInfo
OBJECT: sys
FUNCTIONS:
1. get_bios_vendor() - Returns BIOS vendor
2. get_bios_version() - Returns BIOS version
3. get_bios_date() - Returns BIOS date
4. get_motherboard_model() - Returns motherboard model
5. get_motherboard_manufacturer() - Returns motherboard manufacturer

CLASS: TimeInfo
OBJECT: time
FUNCTIONS:
1. getHour() - Returns current hour (0-23)
2. getMinute() - Returns current minute (0-59)
3. getSecond() - Returns current second (0-59)
4. getDay() - Returns current day of month (1-31)
5. getMonthName() - Returns month name
6. getMonthNumber() - Returns month number (1-12)
7. getYearNumber() - Returns current year
8. getWeekNumber() - Returns week number
9. getDayName() - Returns day name
10. getLeapYear() - Returns leap year status

--- COMPACT MODE CLASSES ---

CLASS: CompactAudio
OBJECT: c_audio
FUNCTIONS:
1. active_audio_input() - Returns active audio input device
2. active_audio_input_status() - Returns input device status
3. active_audio_output() - Returns active audio output device
4. active_audio_output_status() - Returns output device status

CLASS: CompactOS
OBJECT: c_os
FUNCTIONS:
1. getOSName() - Returns OS name
2. getOSBuild() - Returns OS build
3. getArchitecture() - Returns architecture
4. getUptime() - Returns uptime

CLASS: CompactCPU
OBJECT: c_cpu
FUNCTIONS:
1. getCPUName() - Returns CPU name
2. getCPUCores() - Returns number of cores
3. getCPUThreads() - Returns number of threads
4. getClockSpeed() - Returns clock speed in GHz

CLASS: CompactMemory
OBJECT: c_memory
FUNCTIONS:
1. get_total_memory() - Returns total RAM in GB
2. get_free_memory() - Returns free RAM in GB
3. get_used_memory_percent() - Returns RAM usage percentage

CLASS: CompactScreen
OBJECT: screenDetector (local)
FUNCTIONS:
1. getScreens() - Returns vector of screen info
STRUCT: CompactScreenInfo
- name - Display name
- native_width - Native width
- native_height - Native height
- scale_percent - Scaling percentage
- upscale - Upscale factor
- refresh_rate - Refresh rate

CLASS: CompactSystem
OBJECT: c_system
FUNCTIONS: (Not used in current implementation)

CLASS: CompactGPU
OBJECT: c_gpu
FUNCTIONS:
1. getGPUName() - Returns GPU name
2. getGPUUsagePercent() - Returns GPU usage percentage
3. getVRAMGB() - Returns VRAM in GB
4. getGPUFrequency() - Returns GPU frequency

CLASS: CompactPerformance
OBJECT: c_perf
FUNCTIONS:
1. getCPUUsage() - Returns CPU usage percentage
2. getGPUUsage() - Returns GPU usage percentage
3. getRAMUsage() - Returns RAM usage percentage
4. getDiskUsage() - Returns disk usage percentage

CLASS: CompactUser
OBJECT: c_user
FUNCTIONS:
1. getUsername() - Returns username
2. getDomain() - Returns domain
3. isAdmin() - Returns admin status

CLASS: CompactNetwork
OBJECT: c_net
FUNCTIONS:
1. get_network_name() - Returns network name
2. get_network_type() - Returns network type
3. get_network_ip() - Returns IP address

CLASS: DiskInfo
OBJECT: disk
FUNCTIONS:
1. getAllDiskUsage() - Returns map of drive letters to usage percentages
2. getDiskCapacity() - Returns map of drive letters to capacities in GB

--- JSON CONFIGURATION ---
OBJECT: config
TYPE: nlohmann::json
FUNCTIONS:
1. contains(section) - Checks if section exists
2. value(key, default) - Returns value with default
3. [section]["colors"][key] - Color configuration access

--- HELPER LAMBDA FUNCTIONS ---
1. getColor(section, key, default) - Returns ANSI color code
2. isEnabled(section) - Checks if section is enabled
3. isSubEnabled(section, key) - Checks if subsection is enabled
4. isSectionEnabled(module, section) - Checks nested section
5. isNestedEnabled(module, section, key) - Checks deeply nested config
```




===============================================================================
                             END OF DOCUMENTATION
===============================================================================
*/