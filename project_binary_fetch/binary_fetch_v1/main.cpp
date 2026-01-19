// Welcome to BinaryFetch entry point (main.cpp) 

#include <iostream>       // Standard input/output stream (cin, cout) 
#include <iomanip>        // Formatting utilities (setw, precision, setfill) 
#include <vector>         // Dynamic array container 
#include <functional>     // Function objects and wrappers (std::function) 
#include <sstream>        // String stream operations for parsing/conversion 
#include <fstream>        // File stream operations (reading/writing files) 
#include <string>         // Standard string class and methods ✍
#include <regex>          // Regular expressions for pattern matching 
#include <windows.h>      // Core Windows API functions (handles, processes) 
#include <shlobj.h>       // Shell object functions (folder paths, UI) 
#include <direct.h>       // Directory and file handling functions (_mkdir, _chdir) 
#include <comdef.h>       // Native C++ compiler COM support 
#include <Wbemidl.h>      // WMI (Windows Management Instrumentation) interfaces 


// ASCII Art functionality
#include "AsciiArt.h" // main.cpp (AsciiArt separated into header and implementation files)


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


int main(){

    // Initialize COM 
    /*
	 if you're a beginner and don't know what's com...here's a brief explanation:

     * 1. Provides "Native C++" wrappers for complex COM interfaces.
     * 2. Includes _com_ptr_t (Smart Pointers) for automatic memory management.
     * 3. Includes _com_error for C++ exception handling (try/catch) instead of HRESULTs.
     * 4. Simplifies BSTR (string) and VARIANT data type conversions.
     * 5. Makes COM code look like standard C++ rather than low-level C.

    

     why I used com here ?
     -> COM initialization required for WMI (Windows Management
     Instrumentation) queries used by system info modules to retrieve
     hardware/software data via Win32 classes
    
    */

	//com initialization
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        std::cout << "Failed to initialize COM library. Error: 0x"
            << std::hex << hr << std::endl;
        return 1;
    }



    
    // ========== SIMPLIFIED ASCII ART LOADING ==========
        // Just call loadFromFile() - it handles everything automatically!
        // - Checks C:\Users\<User>\AppData\BinaryFetch\BinaryArt.txt
        // - If missing, copies from Default_Ascii_Art.txt and creates it
        // - User can modify their art anytime in AppData folder

	SetConsoleOutputCP(CP_UTF8); // UTF-8 output on Windows console (for emoji printing)
    AsciiArt art;
    if (!art.loadFromFile()) {
        std::cout << "Warning: ASCII art could not be loaded. Continuing without art.\n";
        // Program continues even if art fails to load
    }

    // ========== AUTO CONFIG FILE SETUP ==========
    // true = dev mode (loads local file), false = production mode (extracts from EXE)
    bool LOAD_DEFAULT_CONFIG = false; // must be false for production releases

    std::string configDir = "C:\\Users\\Public\\BinaryFetch";
    std::string userConfigPath = configDir + "\\BinaryFetch_Config.json";
    std::string configPath;

    if (LOAD_DEFAULT_CONFIG) {
        // DEV MODE: Load directly from project folder for fast iteration 🧪
        configPath = "Default_BinaryFetch_Config.json";
    }
    else {
        // PRODUCTION MODE: Use constant public folder 🛰️
        configPath = userConfigPath;

        // 1. Create directory if it doesn't exist
        if (GetFileAttributesA(configDir.c_str()) == INVALID_FILE_ATTRIBUTES) {
            _mkdir(configDir.c_str());
        }

        // 2. Self-Healing: Check if user config exists, if not, extract from EXE memory
        std::ifstream checkConfig(userConfigPath);
        bool userConfigExists = checkConfig.good();
        checkConfig.close();

        if (!userConfigExists) {
            // IDR_DEFAULT_CONFIG is 101 in your resource.h
            HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(101), RT_RCDATA);
            if (hRes) {
                HGLOBAL hData = LoadResource(NULL, hRes);
                DWORD size = SizeofResource(NULL, hRes);
                const char* data = static_cast<const char*>(LockResource(hData));

                std::ofstream userConfig(userConfigPath, std::ios::binary);
                if (userConfig.is_open()) {
                    userConfig.write(data, size);
                    userConfig.close();
                }
            }
            else {
                std::cout << "Warning: Internal resource IDR_DEFAULT_CONFIG not found." << std::endl;
            }
        }
    } 

    // ========== CONFIG LOADING ==========
    json config;
    bool config_loaded = false; // must be false by default

    std::ifstream config_file(configPath);
    if (config_file.is_open()) {
        try {
            config = json::parse(config_file);
            config_loaded = true; // if the json is successfully loaded
        }
        catch (const std::exception& e) {
            std::cout << "Warning: Failed to parse config file. Using hardcoded defaults." << std::endl;

            // provide warning msg if the json parsing fails
        }
        config_file.close(); // close the file after reading 
    }
    else {
        std::cout << "Warning: Could not open config file: " << configPath << std::endl;
    }

	// Color map (for ANSI escape codes) 
    // for beginners, we're simply assign colors like how we 
    // assin vaules in variables 
    map<string, string> colors = {
        {"red", "\033[31m"}, {"green", "\033[32m"}, {"yellow", "\033[33m"},
        {"blue", "\033[34m"}, {"magenta", "\033[35m"}, {"cyan", "\033[36m"},
        {"white", "\033[37m"}, {"bright_red", "\033[91m"}, {"bright_green", "\033[92m"},
        {"bright_yellow", "\033[93m"}, {"bright_blue", "\033[94m"},
        {"bright_magenta", "\033[95m"}, {"bright_cyan", "\033[96m"},
        {"bright_white", "\033[97m"}, {"reset", "\033[0m"}
    };

    // Helper functions 
    // here, we've assigned the default color as white 
    auto getColor = [&](const std::string& section, const std::string& key, const std::string& defaultColor = "white") -> std::string 
     {
        if (!config_loaded || !config.contains(section)) return colors[defaultColor];

        // First...try to get the color from the nested "colors" object
        if (config[section].contains("colors") && config[section]["colors"].contains(key)) 
        {
            std::string colorName = config[section]["colors"][key].get<std::string>();
            return colors.count(colorName) ? colors[colorName] : colors[defaultColor];
        }
        // next...try to get the color directly from the section
        if (config[section].contains(key)) {
            std::string colorName = config[section][key].get<std::string>();
            return colors.count(colorName) ? colors[colorName] : colors[defaultColor];
        }

        return colors[defaultColor];
     };

    // check for each section, is it enabled or not (Aka Core-Module)
    // Example of core-module: CPU,GPU,OS,Netwrok....bla bla bla
    auto isEnabled = [&](const std::string& section) -> bool {
        if (!config_loaded || !config.contains(section)) return true;
        return config[section].value("enabled", true);
        };
    // check for each subsection inside a section,
    // is it enabled or not (Aka sub-module)
	// example of sub-module: CPU base speed, CPU cores, CPU threads...bla bla bla
    auto isSubEnabled = [&](const std::string& section, const std::string& key) -> bool {
        if (!config_loaded || !config.contains(section)) return true;
        return config[section].value(key, true);
        };
    // checks whether a specific section inside a module is enabled or not
     // example:
     // module  -> "network"
     // section -> "ipv4"
     //
     // logic:
     // - if config is not loaded, allow it (default ON)
     // - if the module does not exist in config, allow it
     // - if the module has no "sections" block, allow all sections
     // - otherwise, read the value from: config[module]["sections"][section]
     // - if the section key is missing, default to true...
    auto isSectionEnabled = [&](const std::string& module, const std::string& section) -> bool {
        if (!config_loaded || !config.contains(module)) return true;
        if (!config[module].contains("sections")) return true;
        return config[module]["sections"].value(section, true);
        };
     
   // checks whether a deeply nested key inside a module + section is enabled
   // example:
     // module  -> "network"
     // section -> "ipv4"
     // key     -> "public_ip"
     //
   // logic:
     // - if config is not loaded, allow it (default ON)
     // - if the module does not exist, allow it
     // - if the section does not exist inside the module, allow it
     // - otherwise, read the value from: config[module][section][key]
     // - if the key is missing, default to true
    auto isNestedEnabled = [&](const std::string& module, const std::string& section, const std::string& key) -> bool {
        if (!config_loaded || !config.contains(module)) return true;
        if (!config[module].contains(section)) return true;
        return config[module][section].value(key, true);
        };

    std::string r = colors["reset"];

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
        

        // BinaryFetch Header
        if (isEnabled("header")) {
            std::ostringstream ss;
            ss << getColor("header", "prefix_color", "bright_red") << "~>> " << r
                << getColor("header", "title_color", "green") << "BinaryFetch" << r
                << getColor("header", "line_color", "red") << r;

            if (isSubEnabled("header", "show_line")) ss << getColor("header", "line_color", "white") << "_____________________________________________________" << r << " ";

            lp.push(ss.str());
        }



        // Compact Time
        if (isEnabled("compact_time"))
        {
            TimeInfo time;
            std::ostringstream ss;

            if (isSubEnabled("compact_time", "show_emoji")) ss << getColor("compact_time", "emoji_color", "white") << u8"📅" << r << " ";

            // Helper to get colors from nested time structure
            auto getTimeColor = [&](const std::string& subsection, const std::string& key, const std::string& defaultColor = "white") -> std::string {
                if (!config_loaded || !config.contains("compact_time")) return colors[defaultColor];
                if (!config["compact_time"].contains(subsection)) return colors[defaultColor];
                if (!config["compact_time"][subsection].contains("colors")) return colors[defaultColor];
                if (!config["compact_time"][subsection]["colors"].contains(key)) return colors[defaultColor];

                std::string colorName = config["compact_time"][subsection]["colors"][key].get<std::string>();
                return colors.count(colorName) ? colors[colorName] : colors[defaultColor];
                };

            // ---------- TIME SECTION ----------
            if (isNestedEnabled("compact_time", "time_section", "enabled")) {
                ss << getTimeColor("time_section", "bracket", "white") << "(" << r;

                if (isNestedEnabled("compact_time", "time_section", "show_label")) {
                    ss << getTimeColor("time_section", "label", "white") << "Time: " << r;
                }

                bool wrote = false;

                if (isNestedEnabled("compact_time", "time_section", "show_hour")) {
                    ss << getTimeColor("time_section", "hour", "white")
                        << std::setw(2) << std::setfill('0') << time.getHour() << r;
                    wrote = true;
                }

                if (isNestedEnabled("compact_time", "time_section", "show_minute")) {
                    if (wrote) ss << getTimeColor("time_section", "sep", "white") << ":" << r;
                    ss << getTimeColor("time_section", "minute", "white")
                        << std::setw(2) << std::setfill('0') << time.getMinute() << r;
                    wrote = true;
                }

                if (isNestedEnabled("compact_time", "time_section", "show_second")) {
                    if (wrote) ss << getTimeColor("time_section", "sep", "white") << ":" << r;
                    ss << getTimeColor("time_section", "second", "white")
                        << std::setw(2) << std::setfill('0') << time.getSecond() << r;
                }

                ss << getTimeColor("time_section", "bracket", "white") << ") " << r;
            }

            // ---------- DATE SECTION ----------
            if (isNestedEnabled("compact_time", "date_section", "enabled"))
            {
                ss << getTimeColor("date_section", "bracket", "white") << "(" << r;

                if (isNestedEnabled("compact_time", "date_section", "show_label")) {
                    ss << getTimeColor("date_section", "label", "white") << "Date: " << r;
                }

                bool wrote = false;

                if (isNestedEnabled("compact_time", "date_section", "show_day")) {
                    ss << getTimeColor("date_section", "day", "white")
                        << std::setw(2) << std::setfill('0') << time.getDay() << r;
                    wrote = true;
                }

                if (isNestedEnabled("compact_time", "date_section", "show_month_name")) {
                    if (wrote) ss << getTimeColor("date_section", "sep", "white") << " : " << r;
                    ss << getTimeColor("date_section", "month_name", "white")
                        << time.getMonthName() << r;
                    wrote = true;
                }

                if (isNestedEnabled("compact_time", "date_section", "show_month_num")) {
                    if (wrote) ss << " ";
                    ss << getTimeColor("date_section", "month_num", "white")
                        << std::setw(2) << std::setfill('0') << time.getMonthNumber() << r;
                    wrote = true;
                }

                if (isNestedEnabled("compact_time", "date_section", "show_year")) {
                    if (wrote) ss << getTimeColor("date_section", "sep", "white") << " : " << r;
                    ss << getTimeColor("date_section", "year", "white")
                        << time.getYearNumber() << r;
                }

                ss << getTimeColor("date_section", "bracket", "white") << ") " << r;
            }

            // ---------- WEEK SECTION ----------
            if (isNestedEnabled("compact_time", "week_section", "enabled")) {
                ss << getTimeColor("week_section", "bracket", "white") << "(" << r;

                if (isNestedEnabled("compact_time", "week_section", "show_label")) {
                    ss << getTimeColor("week_section", "label", "white") << "Week: " << r;
                }

                bool wrote = false;

                if (isNestedEnabled("compact_time", "week_section", "show_num")) {
                    ss << getTimeColor("week_section", "num", "white")
                        << time.getWeekNumber() << r;
                    wrote = true;
                }

                if (isNestedEnabled("compact_time", "week_section", "show_day_name")) {
                    if (wrote) ss << getTimeColor("week_section", "sep", "white") << " - " << r;
                    ss << getTimeColor("week_section", "day_name", "white")
                        << time.getDayName() << r;
                }

                ss << getTimeColor("week_section", "bracket", "white") << ") " << r;
            }

            // ---------- LEAP YEAR SECTION ----------
            if (isNestedEnabled("compact_time", "leap_section", "enabled")) {
                ss << getTimeColor("leap_section", "bracket", "white") << "(" << r;

                if (isNestedEnabled("compact_time", "leap_section", "show_label")) {
                    ss << getTimeColor("leap_section", "label", "white") << "Leap Year: " << r;
                }

                if (isNestedEnabled("compact_time", "leap_section", "show_val")) {
                    ss << getTimeColor("leap_section", "val", "white")
                        << time.getLeapYear() << r;
                }

                ss << getTimeColor("leap_section", "bracket", "white") << ")" << r;
            }

            lp.push(ss.str());
        }

        // Compact OS
        if (isEnabled("compact_os")) {
            std::ostringstream ss;

            if (isSubEnabled("compact_os", "show_emoji")) ss << getColor("compact_os", "emoji_color", "white") << u8"🚀 " << r ;

            ss << getColor("compact_os", "OS", "white") << "OS" << r
                << getColor("compact_os", "OS_:", "white") << ": " << r;

            
            if (isSubEnabled("compact_os", "show_name")) ss << getColor("compact_os", "name_color", "white") << c_os.getOSName() << r << " ";
            if (isSubEnabled("compact_os", "show_build")) ss << getColor("compact_os", "build_color", "white") << c_os.getOSBuild() << r;

            if (isSubEnabled("compact_os", "show_arch")) {
                ss << getColor("compact_os", "(", "white") << " (" << r
                    << getColor("compact_os", "arch_color", "white") << c_os.getArchitecture() << r
                    << getColor("compact_os", ")", "white") << ")" << r;
            }

            if (isSubEnabled("compact_os", "show_uptime")) {
                ss << getColor("compact_os", "(", "white") << " (" << r
                    << getColor("compact_os", "uptime_label_color", "white") << "uptime: " << r
                    << getColor("compact_os", "uptime_value_color", "white") << c_os.getUptime() << r
                    << getColor("compact_os", ")", "white") << ")" << r;
            }
            lp.push(ss.str());
        }

        // Compact CPU
        if (isEnabled("compact_cpu")) {
            std::ostringstream ss;

            if (isSubEnabled("compact_cpu", "show_emoji")) ss << getColor("compact_cpu", "emoji_color", "white") << u8"🧠 " << r;

            ss << getColor("compact_cpu", "CPU", "white") << "CPU" << r
                << getColor("compact_cpu", "CPU_:", "white") << ": " << r;

            if (isSubEnabled("compact_cpu", "show_name")) ss << getColor("compact_cpu", "name_color", "white") << c_cpu.getCPUName() << r;

            if (isSubEnabled("compact_cpu", "show_cores") || isSubEnabled("compact_cpu", "show_threads")) {
                ss << getColor("compact_cpu", "(", "white") << " (" << r;
                if (isSubEnabled("compact_cpu", "show_cores")) ss << getColor("compact_cpu", "core_color", "white") << c_cpu.getCPUCores() << r << getColor("compact_cpu", "text_color", "white") << "C" << r;
                if (isSubEnabled("compact_cpu", "show_cores") && isSubEnabled("compact_cpu", "show_threads")) ss << getColor("compact_cpu", "separator_color", "white") << "/" << r;
                if (isSubEnabled("compact_cpu", "show_threads")) ss << getColor("compact_cpu", "thread_color", "white") << c_cpu.getCPUThreads() << r << getColor("compact_cpu", "text_color", "white") << "T" << r;
                ss << getColor("compact_cpu", ")", "white") << ")" << r;
            }
             
            if (isSubEnabled("compact_cpu", "show_clock")) {
                ss << std::fixed << std::setprecision(2)
                    << getColor("compact_cpu", "at_symbol_color", "white") << " @" << r
                    << getColor("compact_cpu", "clock_color", "white") << " " << c_cpu.getClockSpeed() << " GHz" << r;
            }
            lp.push(ss.str());
        }


        // Compact GPU
        if (isEnabled("compact_gpu")) {
            std::ostringstream ss;

            if (isSubEnabled("compact_gpu", "show_emoji")) ss << getColor("compact_gpu", "emoji_color", "white") << u8"🔥" << r << " ";

            ss << getColor("compact_gpu", "GPU", "white") << "GPU" << r
                << getColor("compact_gpu", "GPU_:", "white") << ": " << r;

            if (isSubEnabled("compact_gpu", "show_name")) ss << getColor("compact_gpu", "name_color", "white") << c_gpu.getGPUName() << r;

            if (isSubEnabled("compact_gpu", "show_usage")) {
                ss << getColor("compact_gpu", "(", "white") << " (" << r
                    << getColor("compact_gpu", "usage_color", "white") << c_gpu.getGPUUsagePercent() << "%" << r
                    << getColor("compact_gpu", ")", "white") << ")" << r;
            }

            if (isSubEnabled("compact_gpu", "show_vram")) {
                ss << getColor("compact_gpu", "(", "white") << " (" << r
                    << getColor("compact_gpu", "vram_color", "white") << c_gpu.getVRAMGB() << " GB" << r
                    << getColor("compact_gpu", ")", "white") << ")" << r;
            }

            if (isSubEnabled("compact_gpu", "show_freq")) {
                ss << getColor("compact_gpu", "(", "white") << " (" << r
                    << getColor("compact_gpu", "at_symbol_color", "white") << "@" << r
                    << getColor("compact_gpu", "freq_color", "white") << c_gpu.getGPUFrequency() << r
                    << getColor("compact_gpu", ")", "white") << ")" << r;
            }
            lp.push(ss.str());
        }


        // Compact Screen
        if (isEnabled("compact_screen")) {
            CompactScreen screenDetector;
            auto screens = screenDetector.getScreens();
            std::ostringstream ss;


            

            if (screens.empty()) {
                // No displays detected - show error message
                std::ostringstream ss;
                ss << getColor("compact_screen", "Display", "white") << "Display" << r
                    << getColor("compact_screen", "Display_:", "blue") << ": " << r
                    << getColor("compact_screen", "name_color", "white") << "No displays detected" << r;
                lp.push(ss.str());
            }
            else {
                // Display each detected screen
                for (size_t i = 0; i < screens.size(); ++i) {
                    const auto& screen = screens[i];
                    std::ostringstream ss;

                    if (isSubEnabled("compact_screen", "show_emoji")) ss << getColor("compact_screen", "emoji_color", "white") << u8"📺" << r << " ";

                    // Header: [Display N] ->
                    ss << getColor("compact_screen", "Display", "white")
                        << "Display " << (i + 1) << "" << r
                        << getColor("compact_screen", "Display_:", "white") << ": " << r;

                    // Display name
                    if (isSubEnabled("compact_screen", "show_name")) {
                        ss << getColor("compact_screen", "name_color", "white")
                            << screen.name << r << " ";
                    }

                    // Resolution: (3840 x 2160)
                    if (isSubEnabled("compact_screen", "show_resolution")) {
                        ss << getColor("compact_screen", "(", "white") << "(" << r
                            << getColor("compact_screen", "resolution_color", "White")
                            << screen.native_width << r
                            << getColor("compact_screen", "x", "white") << " x " << r
                            << getColor("compact_screen", "resolution_color", "white")
                            << screen.native_height << r
                            << getColor("compact_screen", ")", "white") << ") " << r;
                    }

                    // Scale: (scale: 175%)
                    if (isSubEnabled("compact_screen", "show_scale")) {
                        ss << getColor("compact_screen", "(", "white") << "(" << r
                            << getColor("compact_screen", "scale_label", "white") << "Scale: " << r
                            << getColor("compact_screen", "scale_value", "white")
                            << screen.scale_percent << "%" << r
                            << getColor("compact_screen", ")", "white") << ") " << r;
                    }

                    // Upscale: (upscale: 4x)
                    if (isSubEnabled("compact_screen", "show_upscale")) {
                        ss << getColor("compact_screen", "(", "white") << "(" << r
                            << getColor("compact_screen", "upscale_label", "white") << "upscale: " << r
                            << getColor("compact_screen", "upscale_value", "white")
                            << screen.upscale << r
                            << getColor("compact_screen", ")", "white") << ") " << r;
                    }

                    // Refresh rate: 
                    if (isSubEnabled("compact_screen", "show_refresh")) {
                        ss << getColor("compact_screen", "(", "white") << "(" << r
                            << getColor("compact_screen", "@", "white") << "@" << r
                            << getColor("compact_screen", "refresh_color", "white")
                            << screen.refresh_rate << "Hz" << r
                            << getColor("compact_screen", ")", "white") << ")" << r;
                    }

                    lp.push(ss.str());
                }
            }
        }
        /*
        
        ## 🎨 Output Examples

            ** With all options enabled : **
            ```
            [Display 1]->ASUS ROG(3840 x 2160) (scale: 175 %) (upscale: 4x) (@60Hz)
            [Display 2]->Samsung Odyssey(2560 x 1440) (scale: 100 %) (upscale: 1x) (@144Hz)
            ```

            ** With minimal options(name + refresh only) :**
            ```
            [Display 1]->ASUS ROG(@60Hz)
            [Display 2]->Samsung Odyssey(@144Hz)
            ```

            ** No displays detected : **
            ```
            [Display]->No displays detected
        
        */

            


        // Compact Memory
        if (isEnabled("compact_memory")) {
            std::ostringstream ss;

            if (isSubEnabled("compact_memory", "show_emoji")) ss << getColor("compact_memory", "emoji_color", "white") << u8"📟" << r << " ";

            ss << getColor("compact_memory", "Memory", "white") << "Memory" << r
                << getColor("compact_memory", "Memory_:", "white") << ": " << r;

            if (isSubEnabled("compact_memory", "show_total")) {
                ss << getColor("compact_memory", "(", "white") << "(" << r
                    << getColor("compact_memory", "label_color", "white") << "total: " << r
                    << getColor("compact_memory", "total_color", "white") << c_memory.get_total_memory() << " GB" << r
                    << getColor("compact_memory", ")", "white") << ")" << r;
            }
            if (isSubEnabled("compact_memory", "show_free")) {
                ss << " " << getColor("compact_memory", "(", "white") << "(" << r
                    << getColor("compact_memory", "label_color", "white") << "free: " << r
                    << getColor("compact_memory", "free_color", "white") << c_memory.get_free_memory() << " GB" << r
                    << getColor("compact_memory", ")", "white") << ")" << r;
            }
            if (isSubEnabled("compact_memory", "show_percent")) {
                ss << " " << getColor("compact_memory", "(", "white") << "(" << r
                    << getColor("compact_memory", "percent_color", "white") << c_memory.get_used_memory_percent() << "%" << r
                    << getColor("compact_memory", ")", "white") << ")" << r;
            }
            lp.push(ss.str());
        }

        // Compact Audio
        if (isEnabled("compact_audio")) {
            if (isSubEnabled("compact_audio", "show_input")) {
                std::ostringstream ss1;

                if (isSubEnabled("compact_audio", "show_audio_input_emoji")) ss1 << getColor("compact_audio", "audio_output_emoji_color", "white") << u8"🎙️" << r << " ";

                ss1 << getColor("compact_audio", "Audio Input", "white") << "Audio Input" << r
                    << getColor("compact_audio", "Audio_Input_:", "white") << ": " << r
                    << getColor("compact_audio", "device_color", "white") << c_audio.active_audio_input() << r << " "
                    << getColor("compact_audio", "(", "white") << "[" << r
                    << getColor("compact_audio", "status_color", "white") << c_audio.active_audio_input_status() << r
                    << getColor("compact_audio", ")", "white") << "]" << r;
                lp.push(ss1.str());
            }
            if (isSubEnabled("compact_audio", "show_output")) {
                std::ostringstream ss2;

                if (isSubEnabled("compact_audio", "show_audio_output_emoji")) ss2 << getColor("compact_audio", "audio_input_emoji_color", "white") << u8"🎧" << r << " ";

                ss2 << getColor("compact_audio", "Audio Output", "white") << "Audio Output" << r
                    << getColor("compact_audio", "Audio_Output_:", "white") << ": " << r
                    << getColor("compact_audio", "device_color", "white") << c_audio.active_audio_output() << r << " "
                    << getColor("compact_audio", "(", "white") << "[" << r
                    << getColor("compact_audio", "status_color", "white") << c_audio.active_audio_output_status() << r
                    << getColor("compact_audio", ")", "white") << "]" << r;
                lp.push(ss2.str());
            }
        }

        // Compact Performance
        if (isEnabled("compact_performance")) {
            std::ostringstream ss;

            if (isSubEnabled("compact_performancec", "show_emoji")) ss << getColor("compact_performance", "emoji_color", "white") << u8"🔋" << r << " ";

            ss << getColor("compact_performance", "Performance", "white") << "Performance" << r
                << getColor("compact_performance", "Performance_:", "white") << ": " << r;

            auto addP = [&](const std::string& subKey, const std::string& label, const std::string& colorKey, auto val) {
                if (isSubEnabled("compact_performance", subKey)) {
                    ss << getColor("compact_performance", "(", "white") << "(" << r
                        << getColor("compact_performance", "label_color", "white") << label << ": " << r
                        << getColor("compact_performance", colorKey, "white") << val << "%" << r
                        << getColor("compact_performance", ")", "white") << ") " << r;
                }
                };
            addP("show_cpu", "CPU", "cpu_color", c_perf.getCPUUsage());
            addP("show_gpu", "GPU", "gpu_color", c_perf.getGPUUsage());
            addP("show_ram", "RAM", "ram_color", c_perf.getRAMUsage());
            addP("show_disk", "Disk", "disk_color", c_perf.getDiskUsage());
            lp.push(ss.str());
        }

        // Compact User
        if (isEnabled("compact_user")) {
            std::ostringstream ss;

            if (isSubEnabled("compact_user", "show_emoji")) ss << getColor("compact_user", "emoji_color", "white") << u8"☕" << r << " ";

            ss << getColor("compact_user", "User", "white") << "User" << r
                << getColor("compact_user", "User_:", "white") << ": " << r;

            if (isSubEnabled("compact_user", "show_username")) ss << getColor("compact_user", "username_color", "white") << "@" << c_user.getUsername() << r;
            if (isSubEnabled("compact_user", "show_domain")) {
                ss << " " << getColor("compact_user", "(", "white") << "(" << r
                    << getColor("compact_user", "label_color", "white") << "Domain: " << r
                    << getColor("compact_user", "domain_color", "white") << c_user.getDomain() << r
                    << getColor("compact_user", ")", "white") << ")" << r;
            }
            if (isSubEnabled("compact_user", "show_type")) {
                ss << " " << getColor("compact_user", "(", "white") << "(" << r
                    << getColor("compact_user", "label_color", "white") << "Type: " << r
                    << getColor("compact_user", "type_color", "white") << c_user.isAdmin() << r
                    << getColor("compact_user", ")", "white") << ")" << r;
            }
            lp.push(ss.str());
        }



            // Compact Network (real)
            if (isEnabled("compact_network")) {
                std::ostringstream ss;

                if (isSubEnabled("compact_network", "show_emoji")) ss << getColor("compact_network", "emoji_color", "white") << u8"🌐" << r << " ";

                ss << getColor("compact_network", "Network", "white") << "Network" << r
                    << getColor("compact_network", "Network_:", "white") << ": " << r;

                if (isSubEnabled("compact_network", "show_name")) {
                    ss << getColor("compact_network", "(", "white") << "(" << r
                        << getColor("compact_network", "label_color", "white") << "Name: " << r
                        << getColor("compact_network", "name_color", "white") << c_net.get_network_name() << r
                        << getColor("compact_network", ")", "white") << ") " << r;
                }
                if (isSubEnabled("compact_network", "show_type")) {
                    ss << getColor("compact_network", "(", "white") << "(" << r
                        << getColor("compact_network", "label_color", "white") << "Type: " << r
                        << getColor("compact_network", "type_color", "white") << c_net.get_network_type() << r
                        << getColor("compact_network", ")", "white") << ") " << r;
                }
                if (isSubEnabled("compact_network", "show_ip")) {
                    ss << getColor("compact_network", "(", "white") << "(" << r
                        << getColor("compact_network", "label_color", "white") << "ip: " << r
                        << getColor("compact_network", "ip_color", "white") << c_net.get_network_ip() << r
                        << getColor("compact_network", ")", "white") << ")" << r;
                }
                lp.push(ss.str());
            }





            // Compact Network (dummy)
            if (isEnabled("dummy_compact_network")) {
                std::ostringstream ss;

                if (isSubEnabled("compact_network", "show_emoji")) ss << getColor("compact_network", "emoji_color", "white") << u8"🌐" << r << " ";

                ss << getColor("compact_network", "Network", "white") << "Network" << r
                    << getColor("compact_network", "Network_:", "white") << ": " << r;

                if (isSubEnabled("compact_network", "show_name")) {
                    ss << getColor("compact_network", "(", "white") << "(" << r
                        << getColor("compact_network", "label_color", "white") << "Name: " << r
                        << getColor("compact_network", "name_color", "white") << "InterCentury" << r
                        << getColor("compact_network", ")", "white") << ") " << r;
                }
                if (isSubEnabled("compact_network", "show_type")) {
                    ss << getColor("compact_network", "(", "white") << "(" << r
                        << getColor("compact_network", "label_color", "white") << "Type: " << r
                        << getColor("compact_network", "type_color", "white") << c_net.get_network_type() << r
                        << getColor("compact_network", ")", "white") << ") " << r;
                }
                if (isSubEnabled("compact_network", "show_ip")) {
                    ss << getColor("compact_network", "(", "white") << "(" << r
                        << getColor("compact_network", "label_color", "white") << "ip: " << r
                        << getColor("compact_network", "ip_color", "white") << "203.0.113.45" << r
                        << getColor("compact_network", ")", "white") << ")" << r;
                }
                lp.push(ss.str());
            }


        
        

        // Compact Disk
        if (isEnabled("compact_disk")) {
            if (isSubEnabled("compact_disk", "show_usage")) {
                auto disks = disk.getAllDiskUsage();
                std::ostringstream ss;

                if (isSubEnabled("compact_disk", "show_disk_usage_emoji")) ss << getColor("compact_disk", "disk_usage_emoji_color", "white") << u8"📂" << r << " ";

                ss << getColor("compact_disk", "Disk Usage", "white") << "Disk Usage" << r << getColor("compact_disk", "Disk_Usage_:", "white") << ": " << r;
                for (const auto& d : disks) {
                    ss << getColor("compact_disk", "(", "white") << "(" << r << getColor("compact_disk", "letter_color", "white") << d.first[0] << ":" << r
                        << " " << getColor("compact_disk", "percent_color", "white") << std::fixed << std::setprecision(1) << d.second << "%" << r
                        << getColor("compact_disk", ")", "white") << ") " << r;
                }
                lp.push(ss.str());
            }

            if (isSubEnabled("compact_disk", "show_capacity")) {
                auto caps = disk.getDiskCapacity();
                std::ostringstream sc;

                if (isSubEnabled("compact_disk", "show_disk_capacity_emoji")) sc << getColor("compact_disk", "disk_capacity_emoji_color", "white") << u8"📊" << r << " ";

                sc << getColor("compact_disk", "Disk Cap", "white") << "Disk Cap" << r << getColor("compact_disk", "Disk_Cap_:", "white") << ": " << r;
                for (const auto& c : caps) {
                    sc << getColor("compact_disk", "(", "white") << "(" << r << getColor("compact_disk", "letter_color", "white") << c.first[0] << r
                        << getColor("compact_disk", "separator_color", "white") << "-" << r << getColor("compact_disk", "capacity_color", "white") << c.second << "GB" << r
                        << getColor("compact_disk", ")", "white") << ")" << r;
                }
                lp.push(sc.str());
            }
        }

        //-----------------------------start of detailed modules----------------------//

        // ----------------- DETAILED MEMORY SECTION ----------------- //
        if (isEnabled("detailed_memory")) {
            lp.push(""); // blank line

            // ---------- HEADER ----------
            if (isSectionEnabled("detailed_memory", "header")) {
                std::ostringstream ss;
                ss << getColor("detailed_memory", ">>~", "white") << ">>~ " << r
                    << getColor("detailed_memory", "header_title", "white") << "Memory Info" << r
                    << getColor("detailed_memory", "-------------------------*", "white") << " -------------------------*" << r;
                lp.push(ss.str());
            }

            // ---------- SUMMARY (TOTAL, FREE, USED) ----------
            if (isSectionEnabled("detailed_memory", "total") ||
                isSectionEnabled("detailed_memory", "free") ||
                isSectionEnabled("detailed_memory", "used_percentage")) {
                std::ostringstream ss;

                // ---------- TOTAL ----------
                if (isSectionEnabled("detailed_memory", "total")) {
                    ss << getColor("detailed_memory", "~", "white") << "~" << r
                        << getColor("detailed_memory", "brackets", "white") << " (" << r
                        << getColor("detailed_memory", "label", "white") << "Total: " << r
                        << getColor("detailed_memory", "total_value", "white") << ram.getTotal() << " GB" << r
                        << getColor("detailed_memory", "brackets", "white") << ") " << r;
                }

                // ---------- FREE ----------
                if (isSectionEnabled("detailed_memory", "free")) {
                    ss << getColor("detailed_memory", "brackets", "white") << "(" << r
                        << getColor("detailed_memory", "label", "white") << "Free: " << r
                        << getColor("detailed_memory", "free_value", "white") << ram.getFree() << " GB" << r
                        << getColor("detailed_memory", "brackets", "white") << ") " << r;
                }

                // ---------- USED PERCENTAGE ----------
                if (isSectionEnabled("detailed_memory", "used_percentage")) {
                    ss << getColor("detailed_memory", "brackets", "white") << "(" << r
                        << getColor("detailed_memory", "label", "white") << "Used: " << r
                        << getColor("detailed_memory", "used_value", "white") << ram.getUsedPercentage() << "%" << r
                        << getColor("detailed_memory", "brackets", "white") << ")" << r;
                }

                lp.push(ss.str());
            }

            // ---------- MODULES ----------
            if (isSectionEnabled("detailed_memory", "modules")) {
                const auto& modules = ram.getModules();
                for (size_t i = 0; i < modules.size(); ++i) {
                    // --- Zero-pad capacity ---
                    std::string cap = modules[i].capacity;
                    int num = 0;
                    try { num = std::stoi(cap); }
                    catch (...) { num = 0; }
                    std::ostringstream capOut;
                    capOut << std::setw(2) << std::setfill('0') << num << "GB";

                    std::ostringstream ss;
                    // Structural Marker and Label
                    ss << getColor("detailed_memory", "~", "white") << "~ " << r
                        << getColor("detailed_memory", "module_label", "white") << "Memory " << i << r
                        << getColor("detailed_memory", " : ", "white") << ": " << r;

                    // Used Percentage in parentheses
                    ss << getColor("detailed_memory", "brackets", "white") << "(" << r
                        << getColor("detailed_memory", "label", "white") << "Used: " << r
                        << getColor("detailed_memory", "used_value", "white") << ram.getUsedPercentage() << "%" << r
                        << getColor("detailed_memory", "brackets", "white") << ") " << r;

                    // Capacity, Type, and Speed
                    ss << getColor("detailed_memory", "capacity", "white") << capOut.str() << r << " "
                        << getColor("detailed_memory", "type", "white") << modules[i].type << r << " "
                        << getColor("detailed_memory", "speed", "white") << modules[i].speed << r;

                    lp.push(ss.str());
                }
            }
        }


        // ----------------- DETAILED STORAGE SECTION (FIXED) ----------------- //
        if (isEnabled("detailed_storage")) {
            lp.push("");

            // Helper function to get nested color values - Defaulted to white
            auto getNestedColor = [&](const std::string& path, const std::string& defaultColor = "white") -> std::string {
                if (!config_loaded || !config.contains("detailed_storage")) return colors[defaultColor];

                std::vector<std::string> keys;
                std::stringstream ss(path);
                std::string key;
                while (std::getline(ss, key, '.')) {
                    keys.push_back(key);
                }

                json current = config["detailed_storage"];
                for (const auto& k : keys) {
                    if (!current.contains(k)) return colors[defaultColor];
                    current = current[k];
                }

                if (current.is_string()) {
                    std::string colorName = current.get<std::string>();
                    return colors.count(colorName) ? colors[colorName] : colors[defaultColor];
                }
                return colors[defaultColor];
                };

            // Helper to check nested boolean values
            auto getNestedBool = [&](const std::string& path, bool defaultValue = true) -> bool {
                if (!config_loaded || !config.contains("detailed_storage")) return defaultValue;

                std::vector<std::string> keys;
                std::stringstream ss(path);
                std::string key;
                while (std::getline(ss, key, '.')) {
                    keys.push_back(key);
                }

                json current = config["detailed_storage"];
                for (const auto& k : keys) {
                    if (!current.contains(k)) return defaultValue;
                    current = current[k];
                }

                if (current.is_boolean()) {
                    return current.get<bool>();
                }
                return defaultValue;
                };

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

            std::vector<storage_data> all_disks_captured;

            // STORAGE SUMMARY SECTION
            if (getNestedBool("sections.storage_summary", true)) {

                // Header
                if (getNestedBool("storage_summary.header.show_header", true)) {
                    std::ostringstream ss;
                    ss << getNestedColor("storage_summary.header.line_color", "white") << "------------------------- " << r
                        << getNestedColor("storage_summary.header.title_color", "white") << "STORAGE SUMMARY" << r
                        << getNestedColor("storage_summary.header.line_color", "white") << " --------------------------" << r;
                    lp.push(ss.str());
                }

                // Process each disk
                storage.process_storage_info([&](const storage_data& d) {
                    all_disks_captured.push_back(d);

                    std::ostringstream ss;

                    // Storage type
                    if (getNestedBool("storage_summary.show_storage_type", true)) {
                        ss << getNestedColor("storage_summary.storage_type_color", "white") << d.storage_type << r << " ";
                    }

                    // Drive letter
                    if (getNestedBool("storage_summary.show_drive_letter", true)) {
                        ss << getNestedColor("storage_summary.drive_letter_color", "white") << d.drive_letter << r;
                    }

                    // Opening bracket
                    ss << getNestedColor("storage_summary.[", "white") << " [" << r;

                    // (Used) label
                    if (getNestedBool("storage_summary.show_used_label", true)) {
                        ss << getNestedColor("storage_summary.(", "white") << " (" << r
                            << getNestedColor("storage_summary.used_label_color", "white") << "Used" << r
                            << getNestedColor("storage_summary.)", "white") << ") " << r;
                    }

                    // Used space
                    if (getNestedBool("storage_summary.show_used_space", true)) {
                        ss << getNestedColor("storage_summary.used_space_color", "white") << fmt_storage(d.used_space) << r;
                    }

                    ss << getNestedColor("storage_summary.used_GIB", "white") << " GiB " << r;

                    // Separator
                    ss << getNestedColor("storage_summary./", "white") << "/" << r;

                    // Total space
                    if (getNestedBool("storage_summary.show_total_space", true)) {
                        ss << getNestedColor("storage_summary.total_space_color", "white") << fmt_storage(d.total_space) << r;
                    }

                    ss << getNestedColor("storage_summary.total_GIB", "white") << " GiB  " << r;

                    // Percentage
                    if (getNestedBool("storage_summary.show_used_percentage", true)) {
                        ss << getNestedColor("storage_summary.used_percentage_color", "white") << d.used_percentage << r;
                    }

                    // Separator
                    ss << getNestedColor("storage_summary.-", "white") << " - " << r;

                    // File system
                    if (getNestedBool("storage_summary.show_file_system", true)) {
                        ss << getNestedColor("storage_summary.file_system_color", "white") << d.file_system << r << " ";
                    }

                    // External/Internal status
                    if (getNestedBool("storage_summary.show_external_status", true)) {
                        if (d.is_external) {
                            ss << getNestedColor("storage_summary.external_text_color", "white") << "Ext" << r;
                        }
                        else {
                            ss << getNestedColor("storage_summary.internal_text_color", "white") << "Int" << r;
                        }
                    }

                    // Closing bracket
                    ss << getNestedColor("storage_summary.]", "white") << " ]" << r;

                    lp.push(ss.str());
                    });
            }

            // DISK PERFORMANCE SECTION
            if (!all_disks_captured.empty() && getNestedBool("sections.disk_performance", true)) {

                lp.push("");

                // Header
                if (getNestedBool("disk_performance.header.show_header", true)) {
                    std::ostringstream ss;
                    ss << getNestedColor("disk_performance.header.line_color", "white") << "-------------------- " << r
                        << getNestedColor("disk_performance.header.title_color", "white") << "DISK PERFORMANCE & DETAILS" << r
                        << getNestedColor("disk_performance.header.line_color", "white") << " --------------------" << r;
                    lp.push(ss.str());
                }

                for (const auto& d : all_disks_captured) {
                    std::ostringstream ss;

                    // Drive letter
                    if (getNestedBool("disk_performance.show_drive_letter", true)) {
                        ss << getNestedColor("disk_performance.drive_letter_color", "white") << d.drive_letter << r;
                    }

                    ss << getNestedColor("storage_summary.[", "white") << " [" << r << " ";

                    // Read speed
                    if (getNestedBool("disk_performance.show_read_speed", true)) {
                        ss << getNestedColor("disk_performance.read_label_color", "white") << "Read:" << r << " "
                            << getNestedColor("disk_performance.read_speed_color", "white") << fmt_speed(d.read_speed) << r;
                    }

                    ss << getNestedColor("disk_performance.speed_unit_color", "white") << " MB/s " << r
                        << getNestedColor("disk_performance.|", "white") << "|" << r << " ";

                    // Write speed
                    if (getNestedBool("disk_performance.show_write_speed", true)) {
                        ss << getNestedColor("disk_performance.write_label_color", "white") << "Write:" << r << " "
                            << getNestedColor("disk_performance.write_speed_color", "white") << fmt_speed(d.write_speed) << r;
                    }

                    ss << getNestedColor("disk_performance.speed_unit_color", "white") << " MB/s " << r
                        << getNestedColor("disk_performance.|", "white") << "|" << r << " ";

                    // Serial number
                    if (getNestedBool("disk_performance.show_serial_number", true)) {
                        ss << getNestedColor("disk_performance.serial_number_color", "white") << d.serial_number << r;
                    }

                    // External/Internal status
                    if (getNestedBool("disk_performance.show_external_status", true)) {
                        if (d.is_external) {
                            ss << getNestedColor("storage_summary.external_text_color", "white") << " Ext" << r;
                        }
                        else {
                            ss << getNestedColor("storage_summary.internal_text_color", "white") << " Int" << r;
                        }
                    }

                    ss << getNestedColor("storage_summary.]", "white") << " ]" << r;

                    lp.push(ss.str());
                }
            }

            // DISK PERFORMANCE PREDICTED
            if (!all_disks_captured.empty() && getNestedBool("sections.disk_performance_predicted", true)) {

                lp.push("");

                // Header
                if (getNestedBool("disk_performance_predicted.header.show_header", true)) {
                    std::ostringstream ss;
                    ss << getNestedColor("disk_performance_predicted.header.line_color", "white") << "---------------- " << r
                        << getNestedColor("disk_performance_predicted.header.title_color", "white") << "DISK PERFORMANCE & DETAILS (Predicted)" << r
                        << getNestedColor("disk_performance_predicted.header.line_color", "white") << " ------------" << r;
                    lp.push(ss.str());
                }

                for (const auto& d : all_disks_captured) {
                    std::ostringstream ss;

                    // Drive letter
                    if (getNestedBool("disk_performance_predicted.show_drive_letter", true)) {
                        ss << getNestedColor("disk_performance_predicted.drive_letter_color", "white") << d.drive_letter << r;
                    }

                    ss << getNestedColor("storage_summary.[", "white") << " [" << r << " ";

                    // Read speed
                    if (getNestedBool("disk_performance_predicted.show_read_speed", true)) {
                        ss << getNestedColor("disk_performance_predicted.read_label_color", "white") << "Read: " << r
                            << getNestedColor("disk_performance_predicted.read_speed_color", "white") << fmt_speed(d.predicted_read_speed) << r;
                    }

                    ss << getNestedColor("disk_performance_predicted.speed_unit_color", "white") << " MB/s " << r
                        << getNestedColor("disk_performance_predicted.|", "white") << "|" << r << " ";

                    // Write speed
                    if (getNestedBool("disk_performance_predicted.show_write_speed", true)) {
                        ss << getNestedColor("disk_performance_predicted.write_label_color", "white") << "Write: " << r
                            << getNestedColor("disk_performance_predicted.write_speed_color", "white") << fmt_speed(d.predicted_write_speed) << r;
                    }

                    ss << getNestedColor("disk_performance_predicted.speed_unit_color", "white") << " MB/s " << r
                        << getNestedColor("disk_performance_predicted.|", "white") << "|" << r << " ";

                    // Serial number
                    if (getNestedBool("disk_performance_predicted.show_serial_number", true)) {
                        ss << getNestedColor("disk_performance_predicted.serial_number_color", "white") << d.serial_number << r;
                    }

                    // External/Internal status
                    if (getNestedBool("disk_performance_predicted.show_external_status", true)) {
                        if (d.is_external) {
                            ss << getNestedColor("storage_summary.external_text_color", "white") << " Ext" << r;
                        }
                        else {
                            ss << getNestedColor("storage_summary.internal_text_color", "white") << " Int" << r;
                        }
                    }

                    ss << getNestedColor("storage_summary.]", "white") << " ]" << r;

                    lp.push(ss.str());
                }
            }

            // No drives detected
            if (all_disks_captured.empty()) {
                lp.push("No drives detected.");
            }
        }
        // ----------------- END DETAILED STORAGE ----------------- //



            // Network Info (Compact + Extra) (real)
            if (isEnabled("network_info")) 
            {

                lp.push("");//blank line....don't use cout !!! it might break the allignment

                // Header
                if (isSubEnabled("network_info", "show_header")) {
                    std::ostringstream ss;
                    ss << getColor("network_info", "#-", "white") << "#- " << r
                        << getColor("network_info", "header_text_color", "white") << "Network Info " << r
                        << getColor("network_info", "separator_line", "white")
                        << "---------------------------------------------------#" << r;
                    lp.push(ss.str());
                }

                // Network Name
                if (isSubEnabled("network_info", "show_name")) {
                    std::ostringstream ss;
                    ss << getColor("network_info", "~", "white") << "~ " << r
                        << getColor("network_info", "name_label_color", "white") // Fixed level color
                        << "Network Name              " << r
                        << getColor("network_info", ":", "white") << ": " << r
                        << getColor("network_info", "name_value_color", "white")
                        << net.get_network_name() << r;
                    lp.push(ss.str());
                }

                // Network Type
                if (isSubEnabled("network_info", "show_type")) {
                    std::ostringstream ss;
                    ss << getColor("network_info", "~", "white") << "~ " << r
                        << getColor("network_info", "type_label_color", "white") // Fixed level color
                        << "Network Type              " << r
                        << getColor("network_info", ":", "white") << ": " << r
                        << getColor("network_info", "type_value_color", "white")
                        << c_net.get_network_type() << r;
                    lp.push(ss.str());
                }

                // local IP 
                if (isSubEnabled("network_info", "show_local_ip")) {
                    std::ostringstream ss;
                    ss << getColor("network_info", "~", "white") << "~ " << r
                        << getColor("network_info", "local_ip_label_color", "white") // Fixed level color
                        << "Local IP                  " << r
                        << getColor("network_info", ":", "white") << ": " << r
                        << getColor("network_info", "local_ip_color", "white")
                        << net.get_local_ip() << r;
                    lp.push(ss.str());
                }

                // public ip
                if (isSubEnabled("network_info", "show_public_ip")) {
                    std::ostringstream ss;
                    ss << getColor("network_info", "~", "white") << "~ " << r
                        << getColor("network_info", "public_ip_label_color", "white") // Fixed level color
                        << "Public IP:                " << r
                        << getColor("network_info", ":", "white") << ": " << r
                        << getColor("network_info", "public_ip_color", "white")
                        << net.get_public_ip() << r;
                    lp.push(ss.str());
                }

                // Locale
                if (isSubEnabled("network_info", "show_locale")) {
                    std::ostringstream ss;
                    ss << getColor("network_info", "~", "white") << "~ " << r
                        << getColor("network_info", "locale_label_color", "white") // Fixed level color
                        << "Locale                    " << r
                        << getColor("network_info", ":", "white") << ": " << r
                        << getColor("network_info", "locale_value_color", "white")
                        << net.get_locale() << r;
                    lp.push(ss.str());
                }

                // MAC Address
                if (isSubEnabled("network_info", "show_mac")) {
                    std::ostringstream ss;
                    ss << getColor("network_info", "~", "white") << "~ " << r
                        << getColor("network_info", "mac_label_color", "white") // Fixed level color
                        << "Mac address               " << r
                        << getColor("network_info", ":", "white") << ": " << r
                        << getColor("network_info", "mac_value_color", "white")
                        << net.get_mac_address() << r;
                    lp.push(ss.str());
                }

                // Upload Speed
                if (isSubEnabled("network_info", "show_upload")) {
                    std::ostringstream ss;
                    ss << getColor("network_info", "~", "white") << "~ " << r
                        << getColor("network_info", "upload_label_color", "white") // Fixed level color
                        << "avg upload speed          " << r
                        << getColor("network_info", ":", "white") << ": " << r
                        << getColor("network_info", "upload_value_color", "white")
                        << net.get_network_upload_speed() << r;
                    lp.push(ss.str());
                }

                // Download Speed
                if (isSubEnabled("network_info", "show_download")) {
                    std::ostringstream ss;
                    ss << getColor("network_info", "~", "white") << "~ " << r
                        << getColor("network_info", "download_label_color", "white") // Fixed level color
                        << "avg download speed        " << r
                        << getColor("network_info", ":", "white") << ": " << r
                        << getColor("network_info", "download_value_color", "white")
                        << net.get_network_download_speed() << r;
                    lp.push(ss.str());
                }
            }

       
        
            // Network Info (Compact + Extra) (dummy)
            if (isEnabled("dummy_network_info")) {

                lp.push("");//blank line....don't use cout !!! it might break the allignment

                // Header
                if (isSubEnabled("network_info", "show_header")) {
                    std::ostringstream ss;
                    ss << getColor("network_info", "#-", "white") << "#- " << r
                        << getColor("network_info", "header_text_color", "white") << "Network Info " << r
                        << getColor("network_info", "separator_line", "white")
                        << "---------------------------------------------------#" << r;
                    lp.push(ss.str());
                }

                // Network Name
                if (isSubEnabled("network_info", "show_name")) {
                    std::ostringstream ss;
                    ss << getColor("network_info", "~", "white") << "~ " << r
                        << getColor("network_info", "name_label_color", "white") // Fixed level color
                        << "Network Name              " << r
                        << getColor("network_info", ":", "white") << ": " << r
                        << getColor("network_info", "name_value_color", "white")
                        << "InterCentury" << r;
                    lp.push(ss.str());
                }

                // Network Type
                if (isSubEnabled("network_info", "show_type")) {
                    std::ostringstream ss;
                    ss << getColor("network_info", "~", "white") << "~ " << r
                        << getColor("network_info", "type_label_color", "white") // Fixed level color
                        << "Network Type              " << r
                        << getColor("network_info", ":", "white") << ": " << r
                        << getColor("network_info", "type_value_color", "white")
                        << c_net.get_network_type() << r;
                    lp.push(ss.str());
                }

                // local IP 
                if (isSubEnabled("network_info", "show_local_ip")) {
                    std::ostringstream ss;
                    ss << getColor("network_info", "~", "white") << "~ " << r
                        << getColor("network_info", "local_ip_label_color", "white") // Fixed level color
                        << "Local IP                  " << r
                        << getColor("network_info", ":", "white") << ": " << r
                        << getColor("network_info", "local_ip_color", "white")
                        << "192.168.1.42" << r;
                    lp.push(ss.str());
                }

                // public ip
                if (isSubEnabled("network_info", "show_public_ip")) {
                    std::ostringstream ss;
                    ss << getColor("network_info", "~", "white") << "~ " << r
                        << getColor("network_info", "public_ip_label_color", "white") // Fixed level color
                        << "Public IP:                " << r
                        << getColor("network_info", ":", "white") << ": " << r
                        << getColor("network_info", "public_ip_color", "white")
                        << "203.0.113.45" << r;
                    lp.push(ss.str());
                }

                // Locale
                if (isSubEnabled("network_info", "show_locale")) {
                    std::ostringstream ss;
                    ss << getColor("network_info", "~", "white") << "~ " << r
                        << getColor("network_info", "locale_label_color", "white") // Fixed level color
                        << "Locale                    " << r
                        << getColor("network_info", ":", "white") << ": " << r
                        << getColor("network_info", "locale_value_color", "white")
                        << net.get_locale() << r;
                    lp.push(ss.str());
                }

                // MAC Address
                if (isSubEnabled("network_info", "show_mac")) {
                    std::ostringstream ss;
                    ss << getColor("network_info", "~", "white") << "~ " << r
                        << getColor("network_info", "mac_label_color", "white") // Fixed level color
                        << "Mac address               " << r
                        << getColor("network_info", ":", "white") << ": " << r
                        << getColor("network_info", "mac_value_color", "white")
                        << "AA:BB:CC:DD:EE:FF" << r;
                    lp.push(ss.str());
                }

                // Upload Speed
                if (isSubEnabled("network_info", "show_upload")) {
                    std::ostringstream ss;
                    ss << getColor("network_info", "~", "white") << "~ " << r
                        << getColor("network_info", "upload_label_color", "white") // Fixed level color
                        << "avg upload speed          " << r
                        << getColor("network_info", ":", "white") << ": " << r
                        << getColor("network_info", "upload_value_color", "white")
                        << net.get_network_upload_speed() << r;
                    lp.push(ss.str());
                }

                // Download Speed
                if (isSubEnabled("network_info", "show_download")) {
                    std::ostringstream ss;
                    ss << getColor("network_info", "~", "white") << "~ " << r
                        << getColor("network_info", "download_label_color", "white") // Fixed level color
                        << "avg download speed        " << r
                        << getColor("network_info", ":", "white") << ": " << r
                        << getColor("network_info", "download_value_color", "white")
                        << net.get_network_download_speed() << r;
                    lp.push(ss.str());
                }
            }


        

 
        // end of the detailed network section////////////////////////////////////////////////

        // OS Info (JSON Driven)
        if (isEnabled("os_info")) {
            lp.push("");

            // Header
            if (isSubEnabled("os_info", "show_header")) {
                std::ostringstream ss;
                ss << getColor("os_info", "#-", "white") << "#- " << r
                    << getColor("os_info", "header_text_color", "white") << "Operating System " << r
                    << getColor("os_info", "separator_line", "white")
                    << "-----------------------------------------------#" << r;
                lp.push(ss.str());
            }

            // Name
            if (isSubEnabled("os_info", "show_name")) {
                std::ostringstream ss;
                ss << getColor("os_info", "~", "white") << "~ " << r
                    << getColor("os_info", "name_label_color", "white") << "Name                      " << r
                    << getColor("os_info", ":", "white") << ": " << r
                    << getColor("os_info", "name_value_color", "white") << os.GetOSName() << r;
                lp.push(ss.str());
            }

            // Build
            if (isSubEnabled("os_info", "show_build")) {
                std::ostringstream ss;
                ss << getColor("os_info", "~", "white") << "~ " << r
                    << getColor("os_info", "build_label_color", "white") << "Build                     " << r
                    << getColor("os_info", ":", "white") << ": " << r
                    << getColor("os_info", "build_value_color", "white") << os.GetOSVersion() << r;
                lp.push(ss.str());
            }

            // Architecture
            if (isSubEnabled("os_info", "show_architecture")) {
                std::ostringstream ss;
                ss << getColor("os_info", "~", "white") << "~ " << r
                    << getColor("os_info", "arch_label_color", "white") << "Architecture              " << r
                    << getColor("os_info", ":", "white") << ": " << r
                    << getColor("os_info", "arch_value_color", "white") << os.GetOSArchitecture() << r;
                lp.push(ss.str());
            }

            // Kernel
            if (isSubEnabled("os_info", "show_kernel")) {
                std::ostringstream ss;
                ss << getColor("os_info", "~", "white") << "~ " << r
                    << getColor("os_info", "kernel_label_color", "white") << "Kernel                    " << r
                    << getColor("os_info", ":", "white") << ": " << r
                    << getColor("os_info", "kernel_value_color", "white") << os.get_os_kernel_info() << r;
                lp.push(ss.str());
            }

            // Uptime
            if (isSubEnabled("os_info", "show_uptime")) {
                std::ostringstream ss;
                ss << getColor("os_info", "~", "white") << "~ " << r
                    << getColor("os_info", "uptime_label_color", "white") << "Uptime                    " << r
                    << getColor("os_info", ":", "white") << ": " << r
                    << getColor("os_info", "uptime_value_color", "white") << os.get_os_uptime() << r;
                lp.push(ss.str());
            }

            // Install Date
            if (isSubEnabled("os_info", "show_install_date")) {
                std::ostringstream ss;
                ss << getColor("os_info", "~", "white") << "~ " << r
                    << getColor("os_info", "install_date_label_color", "white") << "Install Date              " << r
                    << getColor("os_info", ":", "white") << ": " << r
                    << getColor("os_info", "install_date_value_color", "white")
                    << os.get_os_install_date() << r;
                lp.push(ss.str());
            }

            // Serial
            if (isSubEnabled("os_info", "show_serial")) {
                std::ostringstream ss;
                ss << getColor("os_info", "~", "white") << "~ " << r
                    << getColor("os_info", "serial_label_color", "white") << "Serial                    " << r
                    << getColor("os_info", ":", "white") << ": " << r
                    << getColor("os_info", "serial_value_color", "white")
                    << os.get_os_serial_number() << r;
                lp.push(ss.str());
            }
        }

        //end of the OS info section////////////////////////////////////////////////


        // CPU Info (JSON Driven)
        if (isEnabled("cpu_info")) {
            lp.push("");

            // Header
            if (isSubEnabled("cpu_info", "show_header")) {
                std::ostringstream ss;
                ss << getColor("cpu_info", "#-", "white") << "#- " << r
                    << getColor("cpu_info", "header_text_color", "white") << "CPU Info " << r
                    << getColor("cpu_info", "separator_line", "white")
                    << "-------------------------------------------------------#" << r;
                lp.push(ss.str());
            }

            // Brand
            if (isSubEnabled("cpu_info", "show_brand")) {
                std::ostringstream ss;
                ss << getColor("cpu_info", "~", "white") << "~ " << r
                    << getColor("cpu_info", "brand_label_color", "white") << "Brand                     " << r
                    << getColor("cpu_info", ":", "white") << ": " << r
                    << getColor("cpu_info", "brand_value_color", "white") << cpu.get_cpu_info() << r;
                lp.push(ss.str());
            }

            // Utilization
            if (isSubEnabled("cpu_info", "show_utilization")) {
                std::ostringstream ss;
                ss << getColor("cpu_info", "~", "white") << "~ " << r
                    << getColor("cpu_info", "utilization_label_color", "white") << "Utilization               " << r
                    << getColor("cpu_info", ":", "white") << ": " << r
                    << getColor("cpu_info", "utilization_value_color", "white") << cpu.get_cpu_utilization() << r
                    << getColor("cpu_info", "%", "white") << "%" << r;
                lp.push(ss.str());
            }

            // Speed
            if (isSubEnabled("cpu_info", "show_speed")) {
                std::ostringstream ss;
                ss << getColor("cpu_info", "~", "white") << "~ " << r
                    << getColor("cpu_info", "speed_label_color", "white") << "Speed                     " << r
                    << getColor("cpu_info", ":", "white") << ": " << r
                    << getColor("cpu_info", "speed_value_color", "white") << cpu.get_cpu_speed() << r;
                lp.push(ss.str());
            }

            // Base Speed
            if (isSubEnabled("cpu_info", "show_base_speed")) {
                std::ostringstream ss;
                ss << getColor("cpu_info", "~", "white") << "~ " << r
                    << getColor("cpu_info", "base_speed_label_color", "white") << "Base Speed                " << r
                    << getColor("cpu_info", ":", "white") << ": " << r
                    << getColor("cpu_info", "base_speed_value_color", "white") << cpu.get_cpu_base_speed() << r;
                lp.push(ss.str());
            }

            // Cores
            if (isSubEnabled("cpu_info", "show_cores")) {
                std::ostringstream ss;
                ss << getColor("cpu_info", "~", "white") << "~ " << r
                    << getColor("cpu_info", "cores_label_color", "white") << "Cores                     " << r
                    << getColor("cpu_info", ":", "white") << ": " << r
                    << getColor("cpu_info", "cores_value_color", "white") << cpu.get_cpu_cores() << r;
                lp.push(ss.str());
            }

            // Logical Processors
            if (isSubEnabled("cpu_info", "show_logical_processors")) {
                std::ostringstream ss;
                ss << getColor("cpu_info", "~", "white") << "~ " << r
                    << getColor("cpu_info", "logical_processors_label_color", "white") << "Logical Processors        " << r
                    << getColor("cpu_info", ":", "white") << ": " << r
                    << getColor("cpu_info", "logical_processors_value_color", "white") << cpu.get_cpu_logical_processors() << r;
                lp.push(ss.str());
            }

            // Sockets
            if (isSubEnabled("cpu_info", "show_sockets")) {
                std::ostringstream ss;
                ss << getColor("cpu_info", "~", "white") << "~ " << r
                    << getColor("cpu_info", "sockets_label_color", "white") << "Sockets                   " << r
                    << getColor("cpu_info", ":", "white") << ": " << r
                    << getColor("cpu_info", "sockets_value_color", "white") << cpu.get_cpu_sockets() << r;
                lp.push(ss.str());
            }

            // Virtualization
            if (isSubEnabled("cpu_info", "show_virtualization")) {
                std::ostringstream ss;
                ss << getColor("cpu_info", "~", "white") << "~ " << r
                    << getColor("cpu_info", "virtualization_label_color", "white") << "Virtualization            " << r
                    << getColor("cpu_info", ":", "white") << ": " << r
                    << getColor("cpu_info", "virtualization_value_color", "white") << cpu.get_cpu_virtualization() << r;
                lp.push(ss.str());
            }

            // L1 Cache
            if (isSubEnabled("cpu_info", "show_l1_cache")) {
                std::ostringstream ss;
                ss << getColor("cpu_info", "~", "white") << "~ " << r
                    << getColor("cpu_info", "l1_cache_label_color", "white") << "L1 Cache                  " << r
                    << getColor("cpu_info", ":", "white") << ": " << r
                    << getColor("cpu_info", "l1_cache_value_color", "white") << cpu.get_cpu_l1_cache() << r;
                lp.push(ss.str());
            }

            // L2 Cache
            if (isSubEnabled("cpu_info", "show_l2_cache")) {
                std::ostringstream ss;
                ss << getColor("cpu_info", "~", "white") << "~ " << r
                    << getColor("cpu_info", "l2_cache_label_color", "white") << "L2 Cache                  " << r
                    << getColor("cpu_info", ":", "white") << ": " << r
                    << getColor("cpu_info", "l2_cache_value_color", "white") << cpu.get_cpu_l2_cache() << r;
                lp.push(ss.str());
            }

            // L3 Cache
            if (isSubEnabled("cpu_info", "show_l3_cache")) {
                std::ostringstream ss;
                ss << getColor("cpu_info", "~", "white") << "~ " << r
                    << getColor("cpu_info", "l3_cache_label_color", "white") << "L3 Cache                  " << r
                    << getColor("cpu_info", ":", "white") << ": " << r
                    << getColor("cpu_info", "l3_cache_value_color", "white") << cpu.get_cpu_l3_cache() << r;
                lp.push(ss.str());
            }
        }

        //end of the CPU info section////////////////////////////////////////////////
 

        // GPU Info (JSON Driven)
        if (isEnabled("gpu_info")) {
            lp.push("");
            auto all_gpu_info = obj_gpu.get_all_gpu_info();

            if (all_gpu_info.empty()) {
                if (isSubEnabled("gpu_info", "show_header")) {
                    std::ostringstream ss;
                    ss << getColor("gpu_info", "#-", "white") << "#- " << r
                        << getColor("gpu_info", "header_text_color", "white") << "GPU Info " << r
                        << getColor("gpu_info", "separator_line", "white")
                        << "--------------------------------------------------------#" << r;
                    lp.push(ss.str());
                }
                lp.push(getColor("gpu_info", "error_color", "white") + "No GPU detected." + r);
            }
            else {
                // Main Header
                if (isSubEnabled("gpu_info", "show_header")) {
                    std::ostringstream ss;
                    ss << getColor("gpu_info", "#-", "white") << "#- " << r
                        << getColor("gpu_info", "header_text_color", "white") << "GPU Info " << r
                        << getColor("gpu_info", "separator_line", "white")
                        << "-------------------------------------------------------#" << r;
                    lp.push(ss.str());
                }

                for (size_t i = 0; i < all_gpu_info.size(); ++i) {
                    auto& g = all_gpu_info[i];

                    // GPU index line
                    if (isSubEnabled("gpu_info", "show_gpu_index")) {
                        std::ostringstream label;
                        if (i == 0) {
                            label << getColor("gpu_info", "gpu_index_label_color", "white") << "GPU " << (i + 1) << r;
                        }
                        else {
                            label << getColor("gpu_info", "#-", "white") << "#- " << r
                                << getColor("gpu_info", "gpu_index_label_color", "white") << "GPU " << (i + 1) << r
                                << getColor("gpu_info", "separator_line", "white")
                                << " ----------------------------------------------------------#" << r;
                        }

                        std::string lbl = label.str();
                        if (lbl.length() < 27) lbl += std::string(27 - lbl.length(), ' ');
                        lp.push(lbl);
                    }

                    if (isSubEnabled("gpu_info", "show_name")) {
                        std::ostringstream ss;
                        ss << getColor("gpu_info", "|->", "white") << "|-> " << r
                            << getColor("gpu_info", "name_label_color", "white") << "Name                   " << r
                            << getColor("gpu_info", ":", "white") << ": " << r
                            << getColor("gpu_info", "name_value_color", "white") << g.gpu_name << r;
                        lp.push(ss.str());
                    }

                    if (isSubEnabled("gpu_info", "show_memory")) {
                        std::ostringstream ss;
                        ss << getColor("gpu_info", "|->", "white") << "|-> " << r
                            << getColor("gpu_info", "memory_label_color", "white") << "Memory                 " << r
                            << getColor("gpu_info", ":", "white") << ": " << r
                            << getColor("gpu_info", "memory_value_color", "white") << g.gpu_memory << r;
                        lp.push(ss.str());
                    }

                    if (isSubEnabled("gpu_info", "show_usage")) {
                        std::ostringstream ss;
                        ss << getColor("gpu_info", "|->", "white") << "|-> " << r
                            << getColor("gpu_info", "usage_label_color", "white") << "Usage                  " << r
                            << getColor("gpu_info", ":", "white") << ": " << r
                            << getColor("gpu_info", "usage_value_color", "white") << g.gpu_usage << r
                            << getColor("gpu_info", "%", "white") << "%" << r;
                        lp.push(ss.str());
                    }

                    if (isSubEnabled("gpu_info", "show_vendor")) {
                        std::ostringstream ss;
                        ss << getColor("gpu_info", "|->", "white") << "|-> " << r
                            << getColor("gpu_info", "vendor_label_color", "white") << "Vendor                 " << r
                            << getColor("gpu_info", ":", "white") << ": " << r
                            << getColor("gpu_info", "vendor_value_color", "white") << g.gpu_vendor << r;
                        lp.push(ss.str());
                    }

                    if (isSubEnabled("gpu_info", "show_driver")) {
                        std::ostringstream ss;
                        ss << getColor("gpu_info", "|->", "white") << "|-> " << r
                            << getColor("gpu_info", "driver_label_color", "white") << "Driver Version         " << r
                            << getColor("gpu_info", ":", "white") << ": " << r
                            << getColor("gpu_info", "driver_value_color", "white") << g.gpu_driver_version << r;
                        lp.push(ss.str());
                    }

                    if (isSubEnabled("gpu_info", "show_temperature")) {
                        std::ostringstream ss;
                        ss << getColor("gpu_info", "|->", "white") << "|-> " << r
                            << getColor("gpu_info", "temp_label_color", "white") << "Temperature            " << r
                            << getColor("gpu_info", ":", "white") << ": " << r
                            << getColor("gpu_info", "temp_value_color", "white") << g.gpu_temperature << r
                            << getColor("gpu_info", "unit_color", "white") << " C" << r;
                        lp.push(ss.str());
                    }

                    if (isSubEnabled("gpu_info", "show_cores")) {
                        std::ostringstream ss;
                        ss << getColor("gpu_info", "#->", "white") << "#-> " << r
                            << getColor("gpu_info", "cores_label_color", "white") << "Core Count             " << r
                            << getColor("gpu_info", ":", "white") << ": " << r
                            << getColor("gpu_info", "cores_value_color", "white") << g.gpu_core_count << r;
                        lp.push(ss.str());
                    }
                }

                // Primary GPU Details
                auto primary = detailed_gpu_info.primary_gpu_info();
                if (isSubEnabled("gpu_info", "show_primary_details")) {
                    lp.push("");
                    std::ostringstream ss;
                    ss << getColor("gpu_info", "#-", "white") << "#- " << r
                        << getColor("gpu_info", "primary_header_color", "white") << "Primary GPU Details" << r
                        << getColor("gpu_info", "separator_line", "white")
                        << "---------------------------------------------#" << r;
                    lp.push(ss.str());

                    // Primary Name
                    {
                        std::ostringstream ss;
                        ss << getColor("gpu_info", "|->", "white") << "|-> " << r
                            << getColor("gpu_info", "p_name_label_color", "white") << "Name                   " << r
                            << getColor("gpu_info", ":", "white") << ": " << r
                            << getColor("gpu_info", "name_value_color", "white") << primary.name << r;
                        lp.push(ss.str());
                    }
                    // Primary VRAM
                    {
                        std::ostringstream ss;
                        ss << getColor("gpu_info", "|->", "white") << "|-> " << r
                            << getColor("gpu_info", "p_vram_label_color", "white") << "VRAM                   " << r
                            << getColor("gpu_info", ":", "white") << ": " << r
                            << getColor("gpu_info", "memory_value_color", "white") << primary.vram_gb << r
                            << getColor("gpu_info", "unit_color", "white") << " GiB" << r;
                        lp.push(ss.str());
                    }
                    // Primary Frequency
                    {
                        std::ostringstream ss;
                        ss << getColor("gpu_info", "#->", "white") << "#-> " << r
                            << getColor("gpu_info", "p_freq_label_color", "white") << "Frequency              " << r
                            << getColor("gpu_info", ":", "white") << ": " << r
                            << getColor("gpu_info", "freq_value_color", "white") << primary.frequency_ghz << r
                            << getColor("gpu_info", "unit_color", "white") << " GHz" << r;
                        lp.push(ss.str());
                    }
                }
            }
        }
		// end of the GPU info section////////////////////////////////////////////////
         
        // ================= DISPLAY INFO (FULLY JSON DRIVEN) =================
        if (isEnabled("display_info")) {
            lp.push("");

            const auto& screens = di.getScreens();

            for (size_t i = 0; i < screens.size(); ++i) {
                const auto& s = screens[i];

                // ---------- Display Banner ----------
                if (isSubEnabled("display_info", "show_display_banner")) {
                    std::ostringstream ss;
                    ss << getColor("display_info", "#-", "blue") << "#- " << r
                        << getColor("display_info", "display_banner_text", "cyan")
                        << "Display " << (i + 1) << " " << r
                        << getColor("display_info", "display_banner_line", "red")
                        << "------------------------------------------------------#" << r;
                    lp.push(ss.str());
                }

                // ---------- Name ----------
                if (isSubEnabled("display_info", "show_name")) {
                    lp.push(
                        getColor("display_info", "|->", "cyan") + "|-> " + r +
                        getColor("display_info", "name_label_color", "blue") + "Name                   " + r +
                        getColor("display_info", ":", "blue") + ": " + r +
                        getColor("display_info", "name_value_color", "cyan") + s.name + r
                    );
                }

                // ---------- Applied Resolution ----------
                if (isSubEnabled("display_info", "show_applied_resolution")) {
                    std::ostringstream ss;
                    ss << getColor("display_info", "|->", "cyan") << "|-> " << r
                        << getColor("display_info", "applied_res_label_color", "blue")
                        << "Applied Resolution     " << r
                        << getColor("display_info", ":", "blue") << ": " << r
                        << getColor("display_info", "applied_res_value_color", "cyan")
                        << s.current_width
                        << getColor("display_info", "x", "blue") << "x"
                        << s.current_height
                        << getColor("display_info", "@", "blue") << " @"
                        << s.refresh_rate
                        << getColor("display_info", "hz_color", "red") << "Hz" << r;
                    lp.push(ss.str());
                }

                // ---------- Native Resolution ----------
                if (isSubEnabled("display_info", "show_native_resolution")) {
                    lp.push(
                        getColor("display_info", "|->", "cyan") + "|-> " + r +
                        getColor("display_info", "native_res_label_color", "blue")
                        + "Native Resolution      " + r +
                        getColor("display_info", ":", "blue") + ": " + r +
                        getColor("display_info", "native_res_value_color", "cyan")
                        + s.native_resolution + r
                    );
                }

                // ---------- Aspect Ratio ----------
                if (isSubEnabled("display_info", "show_aspect_ratio")) {
                    lp.push(
                        getColor("display_info", "|->", "cyan") + "|-> " + r +
                        getColor("display_info", "aspect_ratio_label_color", "blue")
                        + "Aspect Ratio           " + r +
                        getColor("display_info", ":", "blue") + ": " + r +
                        getColor("display_info", "aspect_ratio_value_color", "cyan")
                        + s.aspect_ratio + r
                    );
                }

                // ---------- Scaling ----------
                if (isSubEnabled("display_info", "show_scaling")) {
                    std::ostringstream ss;
                    ss << getColor("display_info", "|->", "cyan") << "|-> " << r
                        << getColor("display_info", "scaling_label_color", "blue")
                        << "Scaling                " << r
                        << getColor("display_info", ":", "blue") << ": " << r
                        << getColor("display_info", "scaling_value_color", "cyan")
                        << s.scale_percent
                        << getColor("display_info", "%", "blue") << "%" << r;
                    lp.push(ss.str());
                }

                // ---------- Upscale ----------
                if (isSubEnabled("display_info", "show_upscale")) {
                    lp.push(
                        getColor("display_info", "|->", "cyan") + "|-> " + r +
                        getColor("display_info", "upscale_label_color", "blue")
                        + "Upscale                " + r +
                        getColor("display_info", ":", "blue") + ": " + r +
                        getColor("display_info", "upscale_value_color", "cyan")
                        + s.upscale + r
                    );
                }

                // ---------- DSR / VSR ----------
                if (isSubEnabled("display_info", "show_dsr")) {
                    std::ostringstream ss;
                    ss << getColor("display_info", "|->", "cyan") << "|-> " << r
                        << getColor("display_info", "dsr_label_color", "blue")
                        << "DSR / VSR              " << r
                        << getColor("display_info", ":", "blue") << ": " << r
                        << getColor(
                            "display_info",
                            s.dsr_enabled ? "dsr_enabled_color" : "dsr_disabled_color",
                            s.dsr_enabled ? "green" : "red"
                        )
                        << (s.dsr_enabled ? "Enabled" : "Disabled") << r
                        << getColor("display_info", "dsr_brackets_color", "blue")
                        << " (" << r
                        << getColor("display_info", "dsr_type_color", "cyan")
                        << s.dsr_type
                        << getColor("display_info", "dsr_brackets_color", "blue")
                        << ")" << r;
                    lp.push(ss.str());
                }

                lp.push("");
            }
        }



        //END of gpu section/////////////////////////////////////////////////////////////////

        // BIOS & Motherboard Info (JSON Driven)
        if (isEnabled("bios_mb_info")) {
            lp.push("");

            // Header
            if (isSubEnabled("bios_mb_info", "show_header")) {
                std::ostringstream ss;
                ss << getColor("bios_mb_info", "#-", "white") << "#- " << r
                    << getColor("bios_mb_info", "header_text_color", "white") << "BIOS & Motherboard Info " << r
                    << getColor("bios_mb_info", "separator_line", "white")
                    << "----------------------------------------#" << r;
                lp.push(ss.str());
            }

            // Bios Vendor
            if (isSubEnabled("bios_mb_info", "show_bios_vendor")) {
                std::ostringstream ss;
                ss << getColor("bios_mb_info", "~", "white") << "~ " << r
                    << getColor("bios_mb_info", "vendor_label_color", "white") << "Bios Vendor               " << r
                    << getColor("bios_mb_info", ":", "white") << ": " << r
                    << getColor("bios_mb_info", "vendor_value_color", "white") << sys.get_bios_vendor() << r;
                lp.push(ss.str());
            }

            // Bios Version
            if (isSubEnabled("bios_mb_info", "show_bios_version")) {
                std::ostringstream ss;
                ss << getColor("bios_mb_info", "~", "white") << "~ " << r
                    << getColor("bios_mb_info", "version_label_color", "white") << "Bios Version              " << r
                    << getColor("bios_mb_info", ":", "white") << ": " << r
                    << getColor("bios_mb_info", "version_value_color", "white") << sys.get_bios_version() << r;
                lp.push(ss.str());
            }

            // Bios Date
            if (isSubEnabled("bios_mb_info", "show_bios_date")) {
                std::ostringstream ss;
                ss << getColor("bios_mb_info", "~", "white") << "~ " << r
                    << getColor("bios_mb_info", "date_label_color", "white") << "Bios Date                 " << r
                    << getColor("bios_mb_info", ":", "white") << ": " << r
                    << getColor("bios_mb_info", "date_value_color", "white") << sys.get_bios_date() << r;
                lp.push(ss.str());
            }

            // Motherboard Model
            if (isSubEnabled("bios_mb_info", "show_mb_model")) {
                std::ostringstream ss;
                ss << getColor("bios_mb_info", "~", "white") << "~ " << r
                    << getColor("bios_mb_info", "model_label_color", "white") << "Motherboard Model         " << r
                    << getColor("bios_mb_info", ":", "white") << ": " << r
                    << getColor("bios_mb_info", "model_value_color", "white") << sys.get_motherboard_model() << r;
                lp.push(ss.str());
            }

            // Motherboard Manufacturer
            if (isSubEnabled("bios_mb_info", "show_mb_manufacturer")) {
                std::ostringstream ss;
                ss << getColor("bios_mb_info", "~", "white") << "~ " << r
                    << getColor("bios_mb_info", "mfg_label_color", "white") << "Motherboard Manufacturer  " << r
                    << getColor("bios_mb_info", ":", "white") << ": " << r
                    << getColor("bios_mb_info", "mfg_value_color", "white") << sys.get_motherboard_manufacturer() << r;
                lp.push(ss.str());
            }
        }

		//end of the BIOS & Motherboard info section///////////////////////////////

        // User Info (JSON Driven)
        if (isEnabled("user_info")) {
            lp.push("");

            // Header
            if (isSubEnabled("user_info", "show_header")) {
                std::ostringstream ss;
                ss << getColor("user_info", "#-", "white") << "#- " << r
                    << getColor("user_info", "header_text_color", "white") << "User Info " << r
                    << getColor("user_info", "separator_line", "white")
                    << "------------------------------------------------------#" << r;
                lp.push(ss.str());
            }

            // Username
            if (isSubEnabled("user_info", "show_username")) {
                std::ostringstream ss;
                ss << getColor("user_info", "~", "white") << "~ " << r
                    << getColor("user_info", "username_label_color", "white") << "Username                  " << r
                    << getColor("user_info", ":", "white") << ": " << r
                    << getColor("user_info", "username_value_color", "white") << user.get_username() << r;
                lp.push(ss.str());
            }

            // Computer Name
            if (isSubEnabled("user_info", "show_computer_name")) {
                std::ostringstream ss;
                ss << getColor("user_info", "~", "white") << "~ " << r
                    << getColor("user_info", "computer_name_label_color", "white") << "Computer Name             " << r
                    << getColor("user_info", ":", "white") << ": " << r
                    << getColor("user_info", "computer_name_value_color", "white") << user.get_computer_name() << r;
                lp.push(ss.str());
            }

            // Domain
            if (isSubEnabled("user_info", "show_domain")) {
                std::ostringstream ss;
                ss << getColor("user_info", "~", "white") << "~ " << r
                    << getColor("user_info", "domain_label_color", "white") << "Domain                    " << r
                    << getColor("user_info", ":", "white") << ": " << r
                    << getColor("user_info", "domain_value_color", "white") << user.get_domain_name() << r;
                lp.push(ss.str());
            }
        }

		// end of the User info section////////////////////////////////////////////////

        // Performance Info (JSON Driven)
        if (isEnabled("performance_info")) {
            lp.push("");

            // Header
            if (isSubEnabled("performance_info", "show_header")) {
                std::ostringstream ss;
                ss << getColor("performance_info", "#-", "white") << "#- " << r
                    << getColor("performance_info", "header_text_color", "white") << "Performance Info " << r
                    << getColor("performance_info", "separator_line", "white")
                    << "-----------------------------------------------#" << r;
                lp.push(ss.str());
            }

            // System Uptime
            if (isSubEnabled("performance_info", "show_uptime")) {
                std::ostringstream ss;
                ss << getColor("performance_info", "~", "white") << "~ " << r
                    << getColor("performance_info", "uptime_label_color", "white") << "System Uptime            " << r
                    << getColor("performance_info", ":", "white") << ": " << r
                    << getColor("performance_info", "uptime_value_color", "white") << perf.get_system_uptime() << r;
                lp.push(ss.str());
            }

            // CPU Usage
            if (isSubEnabled("performance_info", "show_cpu_usage")) {
                std::ostringstream ss;
                ss << getColor("performance_info", "~", "white") << "~ " << r
                    << getColor("performance_info", "cpu_usage_label_color", "white") << "CPU Usage                " << r
                    << getColor("performance_info", ":", "white") << ": " << r
                    << getColor("performance_info", "usage_value_color", "white") << perf.get_cpu_usage_percent() << r
                    << getColor("performance_info", "%", "white") << "%" << r;
                lp.push(ss.str());
            }

            // RAM Usage
            if (isSubEnabled("performance_info", "show_ram_usage")) {
                std::ostringstream ss;
                ss << getColor("performance_info", "~", "white") << "~ " << r
                    << getColor("performance_info", "ram_usage_label_color", "white") << "RAM Usage                " << r
                    << getColor("performance_info", ":", "white") << ": " << r
                    << getColor("performance_info", "usage_value_color", "white") << perf.get_ram_usage_percent() << r
                    << getColor("performance_info", "%", "white") << "%" << r;
                lp.push(ss.str());
            }

            // Disk Usage
            if (isSubEnabled("performance_info", "show_disk_usage")) {
                std::ostringstream ss;
                ss << getColor("performance_info", "~", "white") << "~ " << r
                    << getColor("performance_info", "disk_usage_label_color", "white") << "Disk Usage               " << r
                    << getColor("performance_info", ":", "white") << ": " << r
                    << getColor("performance_info", "usage_value_color", "white") << perf.get_disk_usage_percent() << r
                    << getColor("performance_info", "%", "white") << "%" << r;
                lp.push(ss.str());
            }

            // GPU Usage
            if (isSubEnabled("performance_info", "show_gpu_usage")) {
                std::ostringstream ss;
                ss << getColor("performance_info", "~", "white") << "~ " << r
                    << getColor("performance_info", "gpu_usage_label_color", "white") << "GPU Usage                " << r
                    << getColor("performance_info", ":", "white") << ": " << r
                    << getColor("performance_info", "usage_value_color", "white") << perf.get_gpu_usage_percent() << r
                    << getColor("performance_info", "%", "white") << "%" << r;
                lp.push(ss.str());
            }
        }

		// end of the Performance info section////////////////////////////////////////

 
        // Audio & Power Info (JSON Driven)
        if (isEnabled("audio_power_info")) {
            lp.push("");
            ExtraInfo audio;

            // --- Output devices ---
            vector<AudioDevice> outputDevices = audio.get_output_devices();

            if (isSubEnabled("audio_power_info", "show_output_header")) {
                std::ostringstream ss;
                ss << getColor("audio_power_info", "#-", "white") << "#- " << r
                    << getColor("audio_power_info", "header_text_color", "white") << "Audio Output " << r
                    << getColor("audio_power_info", "separator_line", "white")
                    << "---------------------------------------------------#" << r;
                lp.push(ss.str());
            }

            int audio_output_device_count = 0;
            for (const auto& device : outputDevices) {
                audio_output_device_count++;
                ostringstream oss;
                oss << getColor("audio_power_info", "~", "white") << "~ " << r
                    << getColor("audio_power_info", "index_color", "white") << audio_output_device_count << r << " "
                    << getColor("audio_power_info", "device_name_color", "white") << device.name << r;

                if (device.isActive && isSubEnabled("audio_power_info", "show_active_status")) {
                    oss << " " << getColor("audio_power_info", "active_label_color", "white") << "(active)" << r;
                }
                lp.push(oss.str());
            }

            // --- Input devices ---
            vector<AudioDevice> inputDevices = audio.get_input_devices();

            if (isSubEnabled("audio_power_info", "show_input_header")) {
                std::ostringstream ss;
                ss << getColor("audio_power_info", "#-", "white") << "#- " << r
                    << getColor("audio_power_info", "header_text_color", "white") << "Audio Input " << r
                    << getColor("audio_power_info", "separator_line", "white")
                    << "----------------------------------------------------#" << r;
                lp.push(ss.str());
            }

            int audio_input_device_count = 0;
            for (const auto& device : inputDevices) {
                audio_input_device_count++;
                ostringstream oss;
                oss << getColor("audio_power_info", "~", "white") << "~ " << r
                    << getColor("audio_power_info", "index_color", "white") << audio_input_device_count << r << " "
                    << getColor("audio_power_info", "device_name_color", "white") << device.name << r;

                if (device.isActive && isSubEnabled("audio_power_info", "show_active_status")) {
                    oss << " " << getColor("audio_power_info", "active_label_color", "white") << "(active)" << r;
                }
                lp.push(oss.str());
            }

            // --- Power Status ---
            if (isSubEnabled("audio_power_info", "show_power_info")) {
                lp.push("");
                PowerStatus power = audio.get_power_status();

                if (isSubEnabled("audio_power_info", "show_power_header")) {
                    std::ostringstream ss;
                    ss << getColor("audio_power_info", "#-", "white") << "#- " << r
                        << getColor("audio_power_info", "header_text_color", "white") << "Power  " << r
                        << getColor("audio_power_info", "separator_line", "white")
                        << "---------------------------------------------------------#" << r;
                    lp.push(ss.str());
                }

                ostringstream ossPower;
                if (!power.hasBattery) {
                    ossPower << getColor("audio_power_info", "bracket_color", "white") << "[" << r
                        << getColor("audio_power_info", "wired_text_color", "white") << "Wired connection" << r
                        << getColor("audio_power_info", "bracket_color", "white") << "]" << r;
                }
                else {
                    ossPower << getColor("audio_power_info", "~", "white") << "~ " << r
                        << getColor("audio_power_info", "battery_label_color", "white") << "Battery powered " << r
                        << getColor("audio_power_info", "bracket_color", "white") << "(" << r
                        << getColor("audio_power_info", "battery_percent_color", "white") << power.batteryPercent << r
                        << getColor("audio_power_info", "unit_color", "white") << "%)" << r;

                    if (power.isCharging) {
                        ossPower << " " << getColor("audio_power_info", "charging_status_color", "white") << "(Charging)" << r;
                    }
                    else {
                        ossPower << " " << getColor("audio_power_info", "not_charging_status_color", "white") << "(Not Charging)" << r;
                    }
                }
                lp.push(ossPower.str());
            }
        }


        /*
        
        
        
        
        
	lp.push(""); // blank line
    lp.push(""); // blank line
    lp.push(""); // blank line
    lp.push(""); // blank line
    lp.push(""); // blank line
    lp.push(""); // blank line
    lp.push(""); // blank line
    lp.push(""); // blank line
    lp.push(""); // blank line
    lp.push(""); // blank line
    lp.push(""); // blank line
    lp.push(""); // blank line
    lp.push(""); // blank line
    lp.push(""); // blank line
    lp.push(""); // blank line
    lp.push(""); // blank line
    lp.push(""); // blank line
    lp.push(""); // blank line
    lp.push(""); // blank line
    lp.push(""); // blank line
    lp.push(""); // blank line
    lp.push(""); // blank line
    lp.push(""); // blank line
    lp.push(""); // blank line
    lp.push(""); // blank line
    lp.push(""); // blank line
    lp.push(""); // blank line
    lp.push(""); // blank line
    lp.push(""); // blank line
    lp.push(""); // blank line
    lp.push(""); // blank line
        
        
        
        
        */



    //----------------- END OF JSON-CONTROLLED COMPACT SECTIONS -----------------//

 

    // ---------------- End of info lines ----------------

    // Print remaining ASCII art lines (if art is taller than info)
    lp.finish();

    std::cout << std::endl;

    // End of CoUninitialize 
    CoUninitialize();
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
1. push(std::string) - Adds formatted line to output queue
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