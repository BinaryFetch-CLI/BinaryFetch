// main.cpp (AsciiArt separated into header and implementation files)

#include <iostream>
#include <iomanip>        // Formatting utilities (setw, precision)
#include <vector>
#include <functional>
#include <sstream>        // For string stream operations
#include <fstream>
#include <string>
#include <fstream>
#include <regex>
#include <windows.h>
#include <shlobj.h>
#include <direct.h>
// ASCII Art functionality
#include "AsciiArt.h" // main.cpp (AsciiArt separated into header and implementation files)

#include <iostream>
#include <iomanip>        // Formatting utilities (setw, precision)
#include <vector>
#include <functional>
#include <sstream>        // For string stream operations
#include <fstream>
#include <string>
#include <fstream>
#include <regex>
#include <windows.h>
#include <shlobj.h>
#include <direct.h>
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
#include "DetailedScreen.h"     // Detailed screen information (EDID, PPI, HDR, etc)


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
    // ========== SIMPLIFIED ASCII ART LOADING ==========
        // Just call loadFromFile() - it handles everything automatically!
        // - Checks C:\Users\<User>\AppData\BinaryFetch\BinaryArt.txt
        // - If missing, copies from Default_Ascii_Art.txt and creates it
        // - User can modify their art anytime in AppData folder

    AsciiArt art;
    if (!art.loadFromFile()) {
        std::cout << "Warning: ASCII art could not be loaded. Continuing without art.\n";
        // Program continues even if art fails to load
    }



    // ========== AUTO CONFIG FILE SETUP (Windows only) ==========
    std::string userConfigPath;
    std::string configDir;

    // Windows: C:\Users\Default\AppData\Local\BinaryFetch\BinaryFetch_Config.json
    configDir = "C:\\Users\\Default\\AppData\\Local\\BinaryFetch";
    userConfigPath = configDir + "\\BinaryFetch_Config.json";

    // Create directory if it doesn't exist
    DWORD attribs = GetFileAttributesA(configDir.c_str());
    bool dirExists = (attribs != INVALID_FILE_ATTRIBUTES && (attribs & FILE_ATTRIBUTE_DIRECTORY));
    if (!dirExists) {
        _mkdir(configDir.c_str());
    }

    // Check if user config exists, if not copy from default
    std::ifstream checkConfig(userConfigPath);
    bool userConfigExists = checkConfig.good();
    checkConfig.close();

    if (!userConfigExists) {
        std::ifstream defaultConfig("Default_BinaryFetch_Config.json", std::ios::binary);
        if (defaultConfig.is_open()) {
            std::ofstream userConfig(userConfigPath, std::ios::binary);
            if (userConfig.is_open()) {
                userConfig << defaultConfig.rdbuf();
                userConfig.close();
            }
            defaultConfig.close();
        }
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

    //-----------------------------testing site end-------------------------



    //lp.push("[Second] -> " + std::to_string(time.getSecond()));
    //lp.push("[Minute] -> " + std::to_string(time.getMinute()));
    //lp.push("[Hour] -> " + std::to_string(time.getHour()));
    //lp.push("[Day] -> " + std::to_string(time.getDay()));
    //lp.push("[Week] -> Week " + std::to_string(time.getWeekNumber()));
    //lp.push("[Day Name] -> " + time.getDayName());
    //lp.push("[Month] -> " + std::to_string(time.getMonthNumber()));
    //lp.push("[Month Name] -> " + time.getMonthName());
    //lp.push("[Year] -> " + std::to_string(time.getYearNumber()));
    //lp.push("[Leap Year] -> " + time.getLeapYear());

    //----------------- JSON CONFIG SYSTEM -----------------//
    // 1. Color Map
    std::map<std::string, std::string> colors = {
        {"red", "\033[31m"}, {"green", "\033[32m"}, {"yellow", "\033[33m"},
        {"blue", "\033[34m"}, {"magenta", "\033[35m"}, {"cyan", "\033[36m"},
        {"white", "\033[37m"}, {"bright_red", "\033[91m"}, {"bright_green", "\033[92m"},
        {"bright_yellow", "\033[93m"}, {"bright_blue", "\033[94m"},
        {"bright_magenta", "\033[95m"}, {"bright_cyan", "\033[96m"},
        {"bright_white", "\033[97m"}, {"reset", "\033[0m"}
    };

    /*
    
    // 2. Load Config from USER LOCATION (not Default_ file)
    std::ifstream config_file(userConfigPath);
    json config;
    bool config_loaded = false;
    if (config_file.is_open()) {
        try {
            config = json::parse(config_file);
            config_loaded = true;
        }
        catch (const std::exception& e) {}
        config_file.close();
    }
    
    */


/*
    By default, BinaryFetch loads its configuration from the BinaryFetch system folder.
    For developers, repeatedly navigating to this location during testing can quickly become a nightmare.

    To solve this, I added an option that lets developers choose where the config is loaded from.
    Now, you can switch between a direct method (loading the config locally) and the standard method (loading from the system folder).

    For development and continuous testing, the direct method is far more convenient and efficient.
 */

// ================= CONFIG LOAD SWITCH =================
// true  = load Default_BinaryFetch_Config.json
// false = load user config from AppData
// appdata path: C:\Users\Default\AppData\Local\BinaryFetch

    bool LOAD_DEFAULT_CONFIG = true;
    // ======================================================


    // 2. Load Config (user OR default based on flag)
    json config;
    bool config_loaded = false;

    std::string configPath = LOAD_DEFAULT_CONFIG
        ? "Default_BinaryFetch_Config.json"
        : userConfigPath;

    std::ifstream config_file(configPath);
    if (config_file.is_open()) {
        try {
            config = json::parse(config_file);
            config_loaded = true;
        }
        catch (const std::exception&) {
            // silently fail, fallback behavior handled elsewhere
        }
        config_file.close();
    }


    // 3. Helper functions
    auto getColor = [&](const std::string& section, const std::string& key, const std::string& defaultColor = "white") -> std::string {
        if (!config_loaded || !config.contains(section)) return colors[defaultColor];

        // First, try to find color in "colors" sub-object (for detailed_memory style)
        if (config[section].contains("colors") && config[section]["colors"].contains(key)) {
            std::string colorName = config[section]["colors"][key].get<std::string>();
            return colors.count(colorName) ? colors[colorName] : colors[defaultColor];
        }

        // Otherwise, look for color directly under section (for compact modules style)
        if (config[section].contains(key)) {
            std::string colorName = config[section][key].get<std::string>();
            return colors.count(colorName) ? colors[colorName] : colors[defaultColor];
        }

        return colors[defaultColor];
        };

    auto isEnabled = [&](const std::string& section) -> bool {
        if (!config_loaded || !config.contains(section)) return true;
        return config[section].value("enabled", true);
        };

    // Helper for sub-module toggles
    auto isSubEnabled = [&](const std::string& section, const std::string& key) -> bool {
        if (!config_loaded || !config.contains(section)) return true;
        return config[section].value(key, true);
        };

    // Helper for sections toggles (for detailed modules)
    auto isSectionEnabled = [&](const std::string& module, const std::string& section) -> bool {
        if (!config_loaded || !config.contains(module)) return true;
        if (!config[module].contains("sections")) return true;
        return config[module]["sections"].value(section, true);
        };

    // Helper for nested section enabled status (for compact_time style)
    auto isNestedEnabled = [&](const std::string& module, const std::string& section, const std::string& key) -> bool {
        if (!config_loaded || !config.contains(module)) return true;
        if (!config[module].contains(section)) return true;
        return config[module][section].value(key, true);
        };

    std::string r = colors["reset"];


    



        //----------------- SECTIONS -----------------//

        // BinaryFetch Header
        if (isEnabled("header")) {
            std::ostringstream ss;
            ss << getColor("header", "prefix_color", "bright_red") << "~>> " << r
                << getColor("header", "title_color", "green") << "BinaryFetch" << r
                << getColor("header", "line_color", "red") << "_____________________________________________________" << r;
            lp.push(ss.str());
        }



        // Compact Time
        if (isEnabled("compact_time"))
        {
            TimeInfo time;
            std::ostringstream ss;

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
            ss << getColor("compact_os", "[OS]", "white") << "[OS]" << r
                << getColor("compact_os", "->", "white") << "  -> " << r;

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
            ss << getColor("compact_cpu", "[CPU]", "white") << "[CPU]" << r
                << getColor("compact_cpu", "->", "white") << " -> " << r;

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


        // Compact Screen
        if (isEnabled("compact_screen")) {
            CompactScreen screenDetector;
            auto screens = screenDetector.getScreens();

            if (screens.empty()) {
                // No displays detected - show error message
                std::ostringstream ss;
                ss << getColor("compact_screen", "[Display]", "white") << "[Display]" << r
                    << getColor("compact_screen", "->", "blue") << " -> " << r
                    << getColor("compact_screen", "name_color", "white") << "No displays detected" << r;
                lp.push(ss.str());
            }
            else {
                // Display each detected screen
                for (size_t i = 0; i < screens.size(); ++i) {
                    const auto& screen = screens[i];
                    std::ostringstream ss;

                    // Header: [Display N] ->
                    ss << getColor("compact_screen", "[Display]", "white")
                        << "[Display " << (i + 1) << "]" << r
                        << getColor("compact_screen", "->", "white") << " -> " << r;

                    // Display name
                    if (isSubEnabled("compact_screen", "show_name")) {
                        ss << getColor("compact_screen", "name_color", "white")
                            << screen.name << r << " ";
                    }

                    // Resolution: (3840 x 2160)
                    if (isSubEnabled("compact_screen", "show_resolution")) {
                        ss << getColor("compact_screen", "(", "white") << "(" << r
                            << getColor("compact_screen", "resolution_color", "white")
                            << screen.native_width << r
                            << getColor("compact_screen", "x", "white") << " x " << r
                            << getColor("compact_screen", "resolution_color", "white")
                            << screen.native_height << r
                            << getColor("compact_screen", ")", "white") << ") " << r;
                    }

                    // Scale: (scale: 175%)
                    if (isSubEnabled("compact_screen", "show_scale")) {
                        ss << getColor("compact_screen", "(", "white") << "(" << r
                            << getColor("compact_screen", "scale_label", "white") << "white: " << r
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

                    // Refresh rate: (60Hz)
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
            ss << getColor("compact_memory", "[Memory]", "white") << "[Memory]" << r
                << getColor("compact_memory", "->", "white") << " -> " << r;

            if (isSubEnabled("compact_memory", "show_total")) {
                ss << getColor("compact_memory", "(", "white") << "(" << r
                    << getColor("compact_memory", "label_color", "white") << "total: " << r
                    << getColor("compact_memory", "total_color", "white") << c_memory.get_total_memory() << " GB" << r
                    << getColor("compact_memory", ")", "white") << ")" << r;
            }
            if (isSubEnabled("compact_memory", "show_free")) {
                ss << " " << getColor("compact_memory", "(", "red") << "(" << r
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
                ss1 << getColor("compact_audio", "[Audio Input]", "white") << "[Audio Input]" << r
                    << getColor("compact_audio", "->", "white") << " -> " << r
                    << getColor("compact_audio", "device_color", "white") << c_audio.active_audio_input() << r << " "
                    << getColor("compact_audio", "(", "white") << "[" << r
                    << getColor("compact_audio", "status_color", "white") << c_audio.active_audio_input_status() << r
                    << getColor("compact_audio", ")", "white") << "]" << r;
                lp.push(ss1.str());
            }
            if (isSubEnabled("compact_audio", "show_output")) {
                std::ostringstream ss2;
                ss2 << getColor("compact_audio", "[Audio Output]", "white") << "[Audio Output]" << r
                    << getColor("compact_audio", "->", "white") << " -> " << r
                    << getColor("compact_audio", "device_color", "white") << c_audio.active_audio_output() << r << " "
                    << getColor("compact_audio", "(", "white") << "[" << r
                    << getColor("compact_audio", "status_color", "white") << c_audio.active_audio_output_status() << r
                    << getColor("compact_audio", ")", "white") << "]" << r;
                lp.push(ss2.str());
            }
        }

        // Compact GPU
        if (isEnabled("compact_gpu")) {
            std::ostringstream ss;
            ss << getColor("compact_gpu", "[GPU]", "white") << "[GPU]" << r
                << getColor("compact_gpu", "->", "white") << " -> " << r;

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

        // Compact Performance
        if (isEnabled("compact_performance")) {
            std::ostringstream ss;
            ss << getColor("compact_performance", "[Performance]", "red") << "[Performance]" << r
                << getColor("compact_performance", "->", "blue") << " -> " << r;

            auto addP = [&](const std::string& subKey, const std::string& label, const std::string& colorKey, auto val) {
                if (isSubEnabled("compact_performance", subKey)) {
                    ss << getColor("compact_performance", "(", "red") << "(" << r
                        << getColor("compact_performance", "label_color", "green") << label << ": " << r
                        << getColor("compact_performance", colorKey, "cyan") << val << "%" << r
                        << getColor("compact_performance", ")", "red") << ") " << r;
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
            ss << getColor("compact_user", "[User]", "red") << "[User]" << r
                << getColor("compact_user", "->", "blue") << " -> " << r;

            if (isSubEnabled("compact_user", "show_username")) ss << getColor("compact_user", "username_color", "green") << "@" << c_user.getUsername() << r;
            if (isSubEnabled("compact_user", "show_domain")) {
                ss << " " << getColor("compact_user", "(", "red") << "(" << r
                    << getColor("compact_user", "label_color", "green") << "Domain: " << r
                    << getColor("compact_user", "domain_color", "yellow") << c_user.getDomain() << r
                    << getColor("compact_user", ")", "red") << ")" << r;
            }
            if (isSubEnabled("compact_user", "show_type")) {
                ss << " " << getColor("compact_user", "(", "red") << "(" << r
                    << getColor("compact_user", "label_color", "green") << "Type: " << r
                    << getColor("compact_user", "type_color", "cyan") << c_user.isAdmin() << r
                    << getColor("compact_user", ")", "red") << ")" << r;
            }
            lp.push(ss.str());
        }

        // Compact Network
        if (isEnabled("compact_network")) {
            std::ostringstream ss;
            ss << getColor("compact_network", "[Network]", "red") << "[Network]" << r
                << getColor("compact_network", "->", "blue") << " -> " << r;

            if (isSubEnabled("compact_network", "show_name")) {
                ss << getColor("compact_network", "(", "red") << "(" << r
                    << getColor("compact_network", "label_color", "green") << "Name: " << r
                    << getColor("compact_network", "name_color", "green") << c_net.get_network_name()<< r
                    << getColor("compact_network", ")", "red") << ") " << r;
            }
            if (isSubEnabled("compact_network", "show_type")) {
                ss << getColor("compact_network", "(", "red") << "(" << r
                    << getColor("compact_network", "label_color", "green") << "Type: " << r
                    << getColor("compact_network", "type_color", "yellow") << c_net.get_network_type() << r
                    << getColor("compact_network", ")", "red") << ") " << r;
            }
            if (isSubEnabled("compact_network", "show_ip")) {
                ss << getColor("compact_network", "(", "red") << "(" << r
                    << getColor("compact_network", "label_color", "green") << "ip: " << r
                    << getColor("compact_network", "ip_color", "magenta") << c_net.get_network_ip() << r
                    << getColor("compact_network", ")", "red") << ")" << r;
            }
            lp.push(ss.str());
        }

        // Compact Disk
        if (isEnabled("compact_disk")) {
            if (isSubEnabled("compact_disk", "show_usage")) {
                auto disks = disk.getAllDiskUsage();
                std::ostringstream ss;
                ss << getColor("compact_disk", "[Disk Usage]", "red") << "[Disk Usage]" << r << getColor("compact_disk", "->", "blue") << " -> " << r;
                for (const auto& d : disks) {
                    ss << getColor("compact_disk", "(", "red") << "(" << r << getColor("compact_disk", "letter_color", "green") << d.first[0] << ":" << r
                        << " " << getColor("compact_disk", "percent_color", "yellow") << std::fixed << std::setprecision(1) << d.second << "%" << r
                        << getColor("compact_disk", ")", "red") << ") " << r;
                }
                lp.push(ss.str());
            }

            if (isSubEnabled("compact_disk", "show_capacity")) {
                auto caps = disk.getDiskCapacity();
                std::ostringstream sc;
                sc << getColor("compact_disk", "[Disk Cap]", "red") << "[Disk Cap]" << r << getColor("compact_disk", "->", "blue") << " -> " << r;
                for (const auto& c : caps) {
                    sc << getColor("compact_disk", "(", "red") << "(" << r << getColor("compact_disk", "letter_color", "green") << c.first[0] << r
                        << getColor("compact_disk", "separator_color", "blue") << "-" << r << getColor("compact_disk", "capacity_color", "yellow") << c.second << "GB" << r
                        << getColor("compact_disk", ")", "red") << ")" << r;
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
                ss << getColor("detailed_memory", ">>~", "blue") << ">>~ " << r
                    << getColor("detailed_memory", "header_title", "red") << "Memory Info" << r
                    << getColor("detailed_memory", "-------------------------*", "blue") << " -------------------------*" << r;
                lp.push(ss.str());
            }

            // ---------- SUMMARY (TOTAL, FREE, USED) ----------
            if (isSectionEnabled("detailed_memory", "total") ||
                isSectionEnabled("detailed_memory", "free") ||
                isSectionEnabled("detailed_memory", "used_percentage")) {
                std::ostringstream ss;

                // ---------- TOTAL ----------
                if (isSectionEnabled("detailed_memory", "total")) {
                    ss << getColor("detailed_memory", "~", "green") << "~" << r
                        << getColor("detailed_memory", "brackets", "blue") << " (" << r
                        << getColor("detailed_memory", "label", "green") << "Total: " << r
                        << getColor("detailed_memory", "total_value", "yellow") << ram.getTotal() << " GB" << r
                        << getColor("detailed_memory", "brackets", "blue") << ") " << r;
                }

                // ---------- FREE ----------
                if (isSectionEnabled("detailed_memory", "free")) {
                    ss << getColor("detailed_memory", "brackets", "blue") << "(" << r
                        << getColor("detailed_memory", "label", "green") << "Free: " << r
                        << getColor("detailed_memory", "free_value", "cyan") << ram.getFree() << " GB" << r
                        << getColor("detailed_memory", "brackets", "blue") << ") " << r;
                }

                // ---------- USED PERCENTAGE ----------
                if (isSectionEnabled("detailed_memory", "used_percentage")) {
                    ss << getColor("detailed_memory", "brackets", "blue") << "(" << r
                        << getColor("detailed_memory", "label", "green") << "Used: " << r
                        << getColor("detailed_memory", "used_value", "red") << ram.getUsedPercentage() << "%" << r
                        << getColor("detailed_memory", "brackets", "blue") << ")" << r;
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
                    ss << getColor("detailed_memory", "~", "blue") << "~ " << r
                        << getColor("detailed_memory", "module_label", "magenta") << "Memory " << i << r
                        << getColor("detailed_memory", " : ", "blue") << ": " << r;

                    // Used Percentage in parentheses
                    ss << getColor("detailed_memory", "brackets", "blue") << "(" << r
                        << getColor("detailed_memory", "label", "green") << "Used: " << r
                        << getColor("detailed_memory", "used_value", "red") << ram.getUsedPercentage() << "%" << r
                        << getColor("detailed_memory", "brackets", "blue") << ") " << r;

                    // Capacity, Type, and Speed
                    ss << getColor("detailed_memory", "capacity", "green") << capOut.str() << r << " "
                        << getColor("detailed_memory", "type", "cyan") << modules[i].type << r << " "
                        << getColor("detailed_memory", "speed", "yellow") << modules[i].speed << r;

                    lp.push(ss.str());
                }
            }
        }


        // ----------------- DETAILED STORAGE SECTION (FIXED) ----------------- //
        if (isEnabled("detailed_storage")) {
            lp.push("");

           // DEBUG: Check if config is loaded
           // std::cout << "[DEBUG] Config loaded: " << (config_loaded ? "YES" : "NO") << std::endl;
           //  if (config_loaded && config.contains("detailed_storage")) {
           //std::cout << "[DEBUG] detailed_storage section found!" << std::endl;
           //}

            // Helper function to get nested color values
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
                    ss << getNestedColor("storage_summary.header.line_color", "red") << "------------------------- " << r
                        << getNestedColor("storage_summary.header.title_color", "blue") << "STORAGE SUMMARY" << r
                        << getNestedColor("storage_summary.header.line_color", "red") << " --------------------------" << r;
                    lp.push(ss.str());
                }

                // Process each disk
                storage.process_storage_info([&](const storage_data& d) {
                    all_disks_captured.push_back(d);

                    std::ostringstream ss;

                    // Storage type
                    if (getNestedBool("storage_summary.show_storage_type", true)) {
                        ss << getNestedColor("storage_summary.storage_type_color", "yellow") << d.storage_type << r << " ";
                    }

                    // Drive letter
                    if (getNestedBool("storage_summary.show_drive_letter", true)) {
                        ss << getNestedColor("storage_summary.drive_letter_color", "cyan") << d.drive_letter << r;
                    }

                    // Opening bracket
                    ss << getNestedColor("storage_summary.[", "green") << " [" << r;

                    // (Used) label
                    if (getNestedBool("storage_summary.show_used_label", true)) {
                        ss << getNestedColor("storage_summary.(", "white") << " (" << r
                            << getNestedColor("storage_summary.used_label_color", "green") << "Used" << r
                            << getNestedColor("storage_summary.)", "white") << ") " << r;
                    }

                    // Used space
                    if (getNestedBool("storage_summary.show_used_space", true)) {
                        ss << getNestedColor("storage_summary.used_space_color", "green") << fmt_storage(d.used_space) << r;
                    }

                    ss << getNestedColor("storage_summary.used_GIB", "green") << " GiB " << r;

                    // Separator
                    ss << getNestedColor("storage_summary./", "green") << "/" << r;

                    // Total space
                    if (getNestedBool("storage_summary.show_total_space", true)) {
                        ss << getNestedColor("storage_summary.total_space_color", "green") << fmt_storage(d.total_space) << r;
                    }

                    ss << getNestedColor("storage_summary.total_GIB", "green") << " GiB  " << r;

                    // Percentage
                    if (getNestedBool("storage_summary.show_used_percentage", true)) {
                        ss << getNestedColor("storage_summary.used_percentage_color", "red") << d.used_percentage << r;
                    }

                    // Separator
                    ss << getNestedColor("storage_summary.-", "green") << " - " << r;

                    // File system
                    if (getNestedBool("storage_summary.show_file_system", true)) {
                        ss << getNestedColor("storage_summary.file_system_color", "magenta") << d.file_system << r << " ";
                    }

                    // External/Internal status
                    if (getNestedBool("storage_summary.show_external_status", true)) {
                        if (d.is_external) {
                            ss << getNestedColor("storage_summary.external_text_color", "blue") << "Ext" << r;
                        }
                        else {
                            ss << getNestedColor("storage_summary.internal_text_color", "white") << "Int" << r;
                        }
                    }

                    // Closing bracket
                    ss << getNestedColor("storage_summary.]", "green") << " ]" << r;

                    lp.push(ss.str());
                    });
            }

            // DISK PERFORMANCE SECTION ////////////////////////////////////////////////////////////////////////////////////////
            if (!all_disks_captured.empty() && getNestedBool("sections.disk_performance", true)) {

                lp.push("");

                // Header
                if (getNestedBool("disk_performance.header.show_header", true)) {
                    std::ostringstream ss;
                    ss << getNestedColor("disk_performance.header.line_color", "red") << "-------------------- " << r
                        << getNestedColor("disk_performance.header.title_color", "blue") << "DISK PERFORMANCE & DETAILS" << r
                        << getNestedColor("disk_performance.header.line_color", "red") << " --------------------" << r;
                    lp.push(ss.str());
                }

                for (const auto& d : all_disks_captured) {
                    std::ostringstream ss;

                    // Drive letter
                    if (getNestedBool("disk_performance.show_drive_letter", true)) {
                        ss << getNestedColor("disk_performance.drive_letter_color", "cyan") << d.drive_letter << r;
                    }

                    ss << getNestedColor("storage_summary.[", "green") << " [" << r << " ";

                    // Read speed
                    if (getNestedBool("disk_performance.show_read_speed", true)) {
                        ss << getNestedColor("disk_performance.read_label_color", "green") << "Read:" << r << " "
                            << getNestedColor("disk_performance.read_speed_color", "yellow") << fmt_speed(d.read_speed) << r;
                    }

                    ss << getNestedColor("disk_performance.speed_unit_color", "green") << " MB/s " << r
                        << getNestedColor("disk_performance.|", "green") << "|" << r << " ";

                    // Write speed
                    if (getNestedBool("disk_performance.show_write_speed", true)) {
                        ss << getNestedColor("disk_performance.write_label_color", "green") << "Write:" << r << " "
                            << getNestedColor("disk_performance.write_speed_color", "yellow") << fmt_speed(d.write_speed) << r;
                    }

                    ss << getNestedColor("disk_performance.speed_unit_color", "green") << " MB/s " << r
                        << getNestedColor("disk_performance.|", "green") << "|" << r << " ";

                    // Serial number
                    if (getNestedBool("disk_performance.show_serial_number", true)) {
                        ss << getNestedColor("disk_performance.serial_number_color", "magenta") << d.serial_number << r;
                    }

                    // External/Internal status
                    if (getNestedBool("disk_performance.show_external_status", true)) {
                        if (d.is_external) {
                            ss << getNestedColor("storage_summary.external_text_color", "blue") << " Ext" << r;
                        }
                        else {
                            ss << getNestedColor("storage_summary.internal_text_color", "white") << " Int" << r;
                        }
                    }

                    ss << getNestedColor("storage_summary.]", "green") << " ]" << r;

                    lp.push(ss.str());
                }
            }

            // DISK PERFORMANCE PREDICTED ////////////////////////////////////////////////////////////////////////////////////////////////////////
            if (!all_disks_captured.empty() && getNestedBool("sections.disk_performance_predicted", true)) {

                lp.push("");

                // Header
                if (getNestedBool("disk_performance_predicted.header.show_header", true)) {
                    std::ostringstream ss;
                    ss << getNestedColor("disk_performance_predicted.header.line_color", "red") << "---------------- " << r
                        << getNestedColor("disk_performance_predicted.header.title_color", "blue") << "DISK PERFORMANCE & DETAILS (Predicted)" << r
                        << getNestedColor("disk_performance_predicted.header.line_color", "red") << " ------------" << r;
                    lp.push(ss.str());
                }

                for (const auto& d : all_disks_captured) {
                    std::ostringstream ss;

                    // Drive letter
                    if (getNestedBool("disk_performance_predicted.show_drive_letter", true)) {
                        ss << getNestedColor("disk_performance_predicted.drive_letter_color", "cyan") << d.drive_letter << r;
                    }

                    ss << getNestedColor("storage_summary.[", "green") << " [" << r << " ";

                    // Read speed
                    if (getNestedBool("disk_performance_predicted.show_read_speed", true)) {
                        ss << getNestedColor("disk_performance_predicted.read_label_color", "green") << "Read: " << r
                            << getNestedColor("disk_performance_predicted.read_speed_color", "yellow") << fmt_speed(d.predicted_read_speed) << r;
                    }

                    ss << getNestedColor("disk_performance_predicted.speed_unit_color", "green") << " MB/s " << r
                        << getNestedColor("disk_performance_predicted.|", "green") << "|" << r << " ";

                    // Write speed
                    if (getNestedBool("disk_performance_predicted.show_write_speed", true)) {
                        ss << getNestedColor("disk_performance_predicted.write_label_color", "green") << "Write: " << r
                            << getNestedColor("disk_performance_predicted.write_speed_color", "yellow") << fmt_speed(d.predicted_write_speed) << r;
                    }

                    ss << getNestedColor("disk_performance_predicted.speed_unit_color", "green") << " MB/s " << r
                        << getNestedColor("disk_performance_predicted.|", "green") << "|" << r << " ";

                    // Serial number
                    if (getNestedBool("disk_performance_predicted.show_serial_number", true)) {
                        ss << getNestedColor("disk_performance_predicted.serial_number_color", "magenta") << d.serial_number << r;
                    }

                    // External/Internal status
                    if (getNestedBool("disk_performance_predicted.show_external_status", true)) {
                        if (d.is_external) {
                            ss << getNestedColor("storage_summary.external_text_color", "blue") << " Ext" << r;
                        }
                        else {
                            ss << getNestedColor("storage_summary.internal_text_color", "white") << " Int" << r;
                        }
                    }

                    ss << getNestedColor("storage_summary.]", "green") << " ]" << r;

                    lp.push(ss.str());
                }
            }

            // No drives detected
            if (all_disks_captured.empty()) {
                lp.push("No drives detected.");
            }
        }
        // ----------------- END DETAILED STORAGE ----------------- //



        // Network Info (Compact + Extra)
        if (isEnabled("network_info")) {

            // Header
            if (isSubEnabled("network_info", "show_header")) {
                std::ostringstream ss;
                ss << getColor("network_info", "#-", "blue") << "#- " << r
                    << getColor("network_info", "header_text_color", "green") << "Network Info " << r
                    << getColor("network_info", "separator_line", "blue")
                    << "---------------------------------------------------#" << r;
                lp.push(ss.str());
            }

            // Network Name
            if (isSubEnabled("network_info", "show_name")) {
                std::ostringstream ss;
                ss << getColor("network_info", "~", "blue") << "~ " << r
                    << getColor("network_info", "label_color", "green")
                    << "Network Name              " << r
                    << getColor("network_info", ":", "blue") << ": " << r
                    << getColor("network_info", "name_value_color", "yellow")
                    << net.get_network_name() << r;
                lp.push(ss.str());
            }

            // Network Type
            if (isSubEnabled("network_info", "show_type")) {
                std::ostringstream ss;
                ss << getColor("network_info", "~", "blue") << "~ " << r
                    << getColor("network_info", "label_color", "green")
                    << "Network Type              " << r
                    << getColor("network_info", ":", "blue") << ": " << r
                    << getColor("network_info", "type_value_color", "yellow")
                    << c_net.get_network_type() << r;
                lp.push(ss.str());
            }

            // local IP 
            if (isSubEnabled("network_info", "show_local_ip")) {
                std::ostringstream ss;
                ss << getColor("network_info", "~", "blue") << "~ " << r
                    << getColor("network_info", "label_color", "green")
                    << "Local IP                  " << r
                    << getColor("network_info", ":", "blue") << ": " << r
                    << getColor("network_info", "local_ip_color", "magenta")
                    << net.get_local_ip() << r;
                lp.push(ss.str());
            }

            // public ip
            if (isSubEnabled("network_info", "show_public_ip")) {
                std::ostringstream ss;
                ss << getColor("network_info", "~", "blue") << "~ " << r
                    << getColor("network_info", "label_color", "green")
                    << "Public IP:                " << r
                    << getColor("network_info", ":", "blue") << ": " << r
                    << getColor("network_info", "public_ip_color", "yellow")
                    << net.get_public_ip() << r;
                lp.push(ss.str());
            }

            // Locale
            if (isSubEnabled("network_info", "show_locale")) {
                std::ostringstream ss;
                ss << getColor("network_info", "~", "blue") << "~ " << r
                    << getColor("network_info", "label_color", "green")
                    << "Locale                    " << r
                    << getColor("network_info", ":", "blue") << ": " << r
                    << getColor("network_info", "locale_value_color", "cyan")
                    << net.get_locale() << r;
                lp.push(ss.str());
            }

            // MAC Address
            if (isSubEnabled("network_info", "show_mac")) {
                std::ostringstream ss;
                ss << getColor("network_info", "~", "blue") << "~ " << r
                    << getColor("network_info", "label_color", "green")
                    << "Mac address               " << r
                    << getColor("network_info", ":", "blue") << ": " << r
                    << getColor("network_info", "mac_value_color", "cyan")
                    << net.get_mac_address() << r;
                lp.push(ss.str());
            }

            // Upload Speed
            if (isSubEnabled("network_info", "show_upload")) {
                std::ostringstream ss;
                ss << getColor("network_info", "~", "blue") << "~ " << r
                    << getColor("network_info", "label_color", "green")
                    << "avg upload speed          " << r
                    << getColor("network_info", ":", "blue") << ": " << r
                    << getColor("network_info", "upload_value_color", "yellow")
                    << net.get_network_upload_speed() << r;
                lp.push(ss.str());
            }


            // Download Speed
            if (isSubEnabled("network_info", "show_download")) {
                std::ostringstream ss;
                ss << getColor("network_info", "~", "blue") << "~ " << r
                    << getColor("network_info", "label_color", "green")
                    << "avg download speed        " << r
                    << getColor("network_info", ":", "blue") << ": " << r
                    << getColor("network_info", "download_value_color", "yellow")
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
                ss << getColor("os_info", "#-", "blue") << "#- " << r
                    << getColor("os_info", "header_text_color", "green") << "Operating System " << r
                    << getColor("os_info", "separator_line", "blue")
                    << "-----------------------------------------------#" << r;
                lp.push(ss.str());
            }

            // Name
            if (isSubEnabled("os_info", "show_name")) {
                std::ostringstream ss;
                ss << getColor("os_info", "~", "blue") << "~ " << r
                    << getColor("os_info", "label_color", "green") << "Name                      " << r
                    << getColor("os_info", ":", "blue") << ": " << r
                    << getColor("os_info", "name_value_color", "yellow") << os.GetOSName() << r;
                lp.push(ss.str());
            }

            // Build
            if (isSubEnabled("os_info", "show_build")) {
                std::ostringstream ss;
                ss << getColor("os_info", "~", "blue") << "~ " << r
                    << getColor("os_info", "label_color", "green") << "Build                     " << r
                    << getColor("os_info", ":", "blue") << ": " << r
                    << getColor("os_info", "build_value_color", "yellow") << os.GetOSVersion() << r;
                lp.push(ss.str());
            }

            // Architecture
            if (isSubEnabled("os_info", "show_architecture")) {
                std::ostringstream ss;
                ss << getColor("os_info", "~", "blue") << "~ " << r
                    << getColor("os_info", "label_color", "green") << "Architecture              " << r
                    << getColor("os_info", ":", "blue") << ": " << r
                    << getColor("os_info", "arch_value_color", "cyan") << os.GetOSArchitecture() << r;
                lp.push(ss.str());
            }

            // Kernel
            if (isSubEnabled("os_info", "show_kernel")) {
                std::ostringstream ss;
                ss << getColor("os_info", "~", "blue") << "~ " << r
                    << getColor("os_info", "label_color", "green") << "Kernel                    " << r
                    << getColor("os_info", ":", "blue") << ": " << r
                    << getColor("os_info", "kernel_value_color", "cyan") << os.get_os_kernel_info() << r;
                lp.push(ss.str());
            }

            // Uptime
            if (isSubEnabled("os_info", "show_uptime")) {
                std::ostringstream ss;
                ss << getColor("os_info", "~", "blue") << "~ " << r
                    << getColor("os_info", "label_color", "green") << "Uptime                    " << r
                    << getColor("os_info", ":", "blue") << ": " << r
                    << getColor("os_info", "uptime_value_color", "magenta") << os.get_os_uptime() << r;
                lp.push(ss.str());
            }

            // Install Date
            if (isSubEnabled("os_info", "show_install_date")) {
                std::ostringstream ss;
                ss << getColor("os_info", "~", "blue") << "~ " << r
                    << getColor("os_info", "label_color", "green") << "Install Date              " << r
                    << getColor("os_info", ":", "blue") << ": " << r
                    << getColor("os_info", "install_date_value_color", "magenta")
                    << os.get_os_install_date() << r;
                lp.push(ss.str());
            }

            // Serial
            if (isSubEnabled("os_info", "show_serial")) {
                std::ostringstream ss;
                ss << getColor("os_info", "~", "blue") << "~ " << r
                    << getColor("os_info", "label_color", "green") << "Serial                    " << r
                    << getColor("os_info", ":", "blue") << ": " << r
                    << getColor("os_info", "serial_value_color", "yellow")
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
                    << getColor("cpu_info", "label_color", "white") << "Brand                    " << r
                    << getColor("cpu_info", ":", "white") << ": " << r
                    << getColor("cpu_info", "brand_value_color", "white") << cpu.get_cpu_info() << r;
                lp.push(ss.str());
            }

            // Utilization
            if (isSubEnabled("cpu_info", "show_utilization")) {
                std::ostringstream ss;
                ss << getColor("cpu_info", "~", "white") << "~ " << r
                    << getColor("cpu_info", "label_color", "white") << "Utilization              " << r
                    << getColor("cpu_info", ":", "white") << ": " << r
                    << getColor("cpu_info", "utilization_value_color", "white") << cpu.get_cpu_utilization() << r
                    << getColor("cpu_info", "%", "white") << "%" << r;
                lp.push(ss.str());
            }

            // Speed
            if (isSubEnabled("cpu_info", "show_speed")) {
                std::ostringstream ss;
                ss << getColor("cpu_info", "~", "white") << "~ " << r
                    << getColor("cpu_info", "label_color", "white") << "Speed                    " << r
                    << getColor("cpu_info", ":", "white") << ": " << r
                    << getColor("cpu_info", "speed_value_color", "white") << cpu.get_cpu_speed() << r;
                lp.push(ss.str());
            }

            // Base Speed
            if (isSubEnabled("cpu_info", "show_base_speed")) {
                std::ostringstream ss;
                ss << getColor("cpu_info", "~", "white") << "~ " << r
                    << getColor("cpu_info", "label_color", "white") << "Base Speed               " << r
                    << getColor("cpu_info", ":", "white") << ": " << r
                    << getColor("cpu_info", "base_speed_value_color", "white") << cpu.get_cpu_base_speed() << r;
                lp.push(ss.str());
            }

            // Cores
            if (isSubEnabled("cpu_info", "show_cores")) {
                std::ostringstream ss;
                ss << getColor("cpu_info", "~", "white") << "~ " << r
                    << getColor("cpu_info", "label_color", "white") << "Cores                    " << r
                    << getColor("cpu_info", ":", "white") << ": " << r
                    << getColor("cpu_info", "cores_value_color", "white") << cpu.get_cpu_cores() << r;
                lp.push(ss.str());
            }

            // Logical Processors
            if (isSubEnabled("cpu_info", "show_logical_processors")) {
                std::ostringstream ss;
                ss << getColor("cpu_info", "~", "white") << "~ " << r
                    << getColor("cpu_info", "label_color", "white") << "Logical Processors       " << r
                    << getColor("cpu_info", ":", "white") << ": " << r
                    << getColor("cpu_info", "logical_processors_value_color", "white") << cpu.get_cpu_logical_processors() << r;
                lp.push(ss.str());
            }

            // Sockets
            if (isSubEnabled("cpu_info", "show_sockets")) {
                std::ostringstream ss;
                ss << getColor("cpu_info", "~", "white") << "~ " << r
                    << getColor("cpu_info", "label_color", "white") << "Sockets                  " << r
                    << getColor("cpu_info", ":", "white") << ": " << r
                    << getColor("cpu_info", "sockets_value_color", "white") << cpu.get_cpu_sockets() << r;
                lp.push(ss.str());
            }

            // Virtualization
            if (isSubEnabled("cpu_info", "show_virtualization")) {
                std::ostringstream ss;
                ss << getColor("cpu_info", "~", "white") << "~ " << r
                    << getColor("cpu_info", "label_color", "white") << "Virtualization           " << r
                    << getColor("cpu_info", ":", "white") << ": " << r
                    << getColor("cpu_info", "virtualization_value_color", "white") << cpu.get_cpu_virtualization() << r;
                lp.push(ss.str());
            }

            // L1 Cache
            if (isSubEnabled("cpu_info", "show_l1_cache")) {
                std::ostringstream ss;
                ss << getColor("cpu_info", "~", "white") << "~ " << r
                    << getColor("cpu_info", "label_color", "white") << "L1 Cache                 " << r
                    << getColor("cpu_info", ":", "white") << ": " << r
                    << getColor("cpu_info", "l1_cache_value_color", "white") << cpu.get_cpu_l1_cache() << r;
                lp.push(ss.str());
            }

            // L2 Cache
            if (isSubEnabled("cpu_info", "show_l2_cache")) {
                std::ostringstream ss;
                ss << getColor("cpu_info", "~", "white") << "~ " << r
                    << getColor("cpu_info", "label_color", "white") << "L2 Cache                 " << r
                    << getColor("cpu_info", ":", "white") << ": " << r
                    << getColor("cpu_info", "l2_cache_value_color", "white") << cpu.get_cpu_l2_cache() << r;
                lp.push(ss.str());
            }

            // L3 Cache
            if (isSubEnabled("cpu_info", "show_l3_cache")) {
                std::ostringstream ss;
                ss << getColor("cpu_info", "~", "white") << "~ " << r
                    << getColor("cpu_info", "label_color", "white") << "L3 Cache                 " << r
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
                            label << getColor("gpu_info", "gpu_label_color", "white") << "GPU " << (i + 1) << r;
                        }
                        else {
                            label << getColor("gpu_info", "#-", "white") << "#-" << r
                                << getColor("gpu_info", "gpu_label_color", "white") << "GPU " << (i + 1) << r
                                << getColor("gpu_info", "separator_line", "white")
                                << "------------------------------------------------------------#" << r;
                        }

                        std::string lbl = label.str();
                        // Padding logic (adjusted for dynamic colors)
                        if (lbl.length() < 27) lbl += std::string(27 - lbl.length(), ' ');
                        lp.push(lbl);
                    }

                    if (isSubEnabled("gpu_info", "show_name")) {
                        std::ostringstream ss;
                        ss << getColor("gpu_info", "|->", "white") << "|-> " << r
                            << getColor("gpu_info", "label_color", "white") << "Name                   " << r
                            << getColor("gpu_info", ":", "white") << ": " << r
                            << getColor("gpu_info", "name_value_color", "white") << g.gpu_name << r;
                        lp.push(ss.str());
                    }

                    if (isSubEnabled("gpu_info", "show_memory")) {
                        std::ostringstream ss;
                        ss << getColor("gpu_info", "|->", "white") << "|-> " << r
                            << getColor("gpu_info", "label_color", "white") << "Memory                 " << r
                            << getColor("gpu_info", ":", "white") << ": " << r
                            << getColor("gpu_info", "memory_value_color", "white") << g.gpu_memory << r;
                        lp.push(ss.str());
                    }

                    if (isSubEnabled("gpu_info", "show_usage")) {
                        std::ostringstream ss;
                        ss << getColor("gpu_info", "|->", "white") << "|-> " << r
                            << getColor("gpu_info", "label_color", "white") << "Usage                  " << r
                            << getColor("gpu_info", ":", "white") << ": " << r
                            << getColor("gpu_info", "usage_value_color", "white") << g.gpu_usage << r
                            << getColor("gpu_info", "%", "white") << "%" << r;
                        lp.push(ss.str());
                    }

                    if (isSubEnabled("gpu_info", "show_vendor")) {
                        std::ostringstream ss;
                        ss << getColor("gpu_info", "|->", "white") << "|-> " << r
                            << getColor("gpu_info", "label_color", "white") << "Vendor                 " << r
                            << getColor("gpu_info", ":", "white") << ": " << r
                            << getColor("gpu_info", "vendor_value_color", "white") << g.gpu_vendor << r;
                        lp.push(ss.str());
                    }

                    if (isSubEnabled("gpu_info", "show_driver")) {
                        std::ostringstream ss;
                        ss << getColor("gpu_info", "|->", "white") << "|-> " << r
                            << getColor("gpu_info", "label_color", "white") << "Driver Version         " << r
                            << getColor("gpu_info", ":", "white") << ": " << r
                            << getColor("gpu_info", "driver_value_color", "white") << g.gpu_driver_version << r;
                        lp.push(ss.str());
                    }

                    if (isSubEnabled("gpu_info", "show_temperature")) {
                        std::ostringstream ss;
                        ss << getColor("gpu_info", "|->", "white") << "|-> " << r
                            << getColor("gpu_info", "label_color", "white") << "Temperature            " << r
                            << getColor("gpu_info", ":", "white") << ": " << r
                            << getColor("gpu_info", "temp_value_color", "white") << g.gpu_temperature << r
                            << getColor("gpu_info", "unit_color", "white") << " C" << r;
                        lp.push(ss.str());
                    }

                    if (isSubEnabled("gpu_info", "show_cores")) {
                        std::ostringstream ss;
                        ss << getColor("gpu_info", "#->", "white") << "#-> " << r
                            << getColor("gpu_info", "label_color", "white") << "Core Count             " << r
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
                            << getColor("gpu_info", "label_color", "white") << "Name                   " << r
                            << getColor("gpu_info", ":", "white") << ": " << r
                            << getColor("gpu_info", "name_value_color", "white") << primary.name << r;
                        lp.push(ss.str());
                    }
                    // Primary VRAM
                    {
                        std::ostringstream ss;
                        ss << getColor("gpu_info", "|->", "white") << "|-> " << r
                            << getColor("gpu_info", "label_color", "white") << "VRAM                   " << r
                            << getColor("gpu_info", ":", "white") << ": " << r
                            << getColor("gpu_info", "memory_value_color", "white") << primary.vram_gb << r
                            << getColor("gpu_info", "unit_color", "white") << " GiB" << r;
                        lp.push(ss.str());
                    }
                    // Primary Frequency
                    {
                        std::ostringstream ss;
                        ss << getColor("gpu_info", "#->", "white") << "#-> " << r
                            << getColor("gpu_info", "label_color", "white") << "Frequency              " << r
                            << getColor("gpu_info", ":", "white") << ": " << r
                            << getColor("gpu_info", "freq_value_color", "white") << primary.frequency_ghz << r
                            << getColor("gpu_info", "unit_color", "white") << " GHz" << r;
                        lp.push(ss.str());
                    } 
                } 
            }  
        } 

		// end of the GPU info section////////////////////////////////////////////////
         

        // Display Info (JSON Driven)
        if (isEnabled("display_info")) {
            lp.push("");
            auto monitors = display.get_all_displays();

            if (monitors.empty()) {
                if (isSubEnabled("display_info", "show_header")) {
                    std::ostringstream ss;
                    ss << getColor("display_info", "#-", "white") << "#- " << r
                        << getColor("display_info", "error_header_color", "white") << "Display" << r
                        << getColor("display_info", "separator_line", "white")
                        << " ----------------------------------------------------------#" << r;
                    lp.push(ss.str());
                }
                lp.push(getColor("display_info", "error_color", "white") + "No monitors detected." + r);
            }
            else {
                for (size_t i = 0; i < monitors.size(); ++i) {
                    auto& m = monitors[i];

                    // Monitor Header
                    if (isSubEnabled("display_info", "show_header")) {
                        std::ostringstream ss;
                        ss << getColor("display_info", "#-", "white") << "#- " << r
                            << getColor("display_info", "header_text_color", "white") << "Monitor " << (i + 1) << " " << r
                            << getColor("display_info", "separator_line", "white")
                            << "--------------------------------------------------------#" << r;
                        lp.push(ss.str());
                    }

                    // Brand
                    if (isSubEnabled("display_info", "show_brand")) {
                        std::ostringstream ss;
                        ss << getColor("display_info", "~", "white") << "~ " << r
                            << getColor("display_info", "label_color", "white") << "Brand                    " << r
                            << getColor("display_info", ":", "white") << ": " << r
                            << getColor("display_info", "brand_value_color", "white") << m.brand_name << r;
                        lp.push(ss.str());
                    }

                    // Resolution
                    if (isSubEnabled("display_info", "show_resolution")) {
                        std::ostringstream ss;
                        ss << getColor("display_info", "~", "white") << "~ " << r
                            << getColor("display_info", "label_color", "white") << "Resolution               " << r
                            << getColor("display_info", ":", "white") << ": " << r
                            << getColor("display_info", "resolution_value_color", "white") << m.resolution << r;
                        lp.push(ss.str());
                    }

                    // Refresh Rate
                    if (isSubEnabled("display_info", "show_refresh_rate")) {
                        std::ostringstream ss;
                        ss << getColor("display_info", "~", "white") << "~ " << r
                            << getColor("display_info", "label_color", "white") << "Refresh Rate             " << r
                            << getColor("display_info", ":", "white") << ": " << r
                            << getColor("display_info", "refresh_rate_value_color", "white") << m.refresh_rate << r
                            << getColor("display_info", "unit_color", "white") << " Hz" << r;
                        lp.push(ss.str());
                    }
                }
            }
        }

		// end of the Display info section////////////////////////////////////////////////


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
                    << getColor("bios_mb_info", "label_color", "white") << "Bios Vendor              " << r
                    << getColor("bios_mb_info", ":", "white") << ": " << r
                    << getColor("bios_mb_info", "vendor_value_color", "white") << sys.get_bios_vendor() << r;
                lp.push(ss.str());
            }

            // Bios Version
            if (isSubEnabled("bios_mb_info", "show_bios_version")) {
                std::ostringstream ss;
                ss << getColor("bios_mb_info", "~", "white") << "~ " << r
                    << getColor("bios_mb_info", "label_color", "white") << "Bios Version             " << r
                    << getColor("bios_mb_info", ":", "white") << ": " << r
                    << getColor("bios_mb_info", "version_value_color", "white") << sys.get_bios_version() << r;
                lp.push(ss.str());
            }

            // Bios Date
            if (isSubEnabled("bios_mb_info", "show_bios_date")) {
                std::ostringstream ss;
                ss << getColor("bios_mb_info", "~", "white") << "~ " << r
                    << getColor("bios_mb_info", "label_color", "white") << "Bios Date                " << r
                    << getColor("bios_mb_info", ":", "white") << ": " << r
                    << getColor("bios_mb_info", "date_value_color", "white") << sys.get_bios_date() << r;
                lp.push(ss.str());
            }

            // Motherboard Model
            if (isSubEnabled("bios_mb_info", "show_mb_model")) {
                std::ostringstream ss;
                ss << getColor("bios_mb_info", "~", "white") << "~ " << r
                    << getColor("bios_mb_info", "label_color", "white") << "Motherboard Model        " << r
                    << getColor("bios_mb_info", ":", "white") << ": " << r
                    << getColor("bios_mb_info", "model_value_color", "white") << sys.get_motherboard_model() << r;
                lp.push(ss.str());
            }

            // Motherboard Manufacturer
            if (isSubEnabled("bios_mb_info", "show_mb_manufacturer")) {
                std::ostringstream ss;
                ss << getColor("bios_mb_info", "~", "white") << "~ " << r
                    << getColor("bios_mb_info", "label_color", "white") << "Motherboard Manufacturer " << r
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
                    << "----------------------------------------------------#" << r;
                lp.push(ss.str());
            }

            // Username
            if (isSubEnabled("user_info", "show_username")) {
                std::ostringstream ss;
                ss << getColor("user_info", "~", "white") << "~ " << r
                    << getColor("user_info", "label_color", "white") << "Username                  " << r
                    << getColor("user_info", ":", "white") << ": " << r
                    << getColor("user_info", "username_value_color", "white") << user.get_username() << r;
                lp.push(ss.str());
            }

            // Computer Name
            if (isSubEnabled("user_info", "show_computer_name")) {
                std::ostringstream ss;
                ss << getColor("user_info", "~", "white") << "~ " << r
                    << getColor("user_info", "label_color", "white") << "Computer Name             " << r
                    << getColor("user_info", ":", "white") << ": " << r
                    << getColor("user_info", "computer_name_value_color", "white") << user.get_computer_name() << r;
                lp.push(ss.str());
            }

            // Domain
            if (isSubEnabled("user_info", "show_domain")) {
                std::ostringstream ss;
                ss << getColor("user_info", "~", "white") << "~ " << r
                    << getColor("user_info", "label_color", "white") << "Domain                    " << r
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
                    << getColor("performance_info", "label_color", "white") << "System Uptime            " << r
                    << getColor("performance_info", ":", "white") << ": " << r
                    << getColor("performance_info", "uptime_value_color", "white") << perf.get_system_uptime() << r;
                lp.push(ss.str());
            }

            // CPU Usage
            if (isSubEnabled("performance_info", "show_cpu_usage")) {
                std::ostringstream ss;
                ss << getColor("performance_info", "~", "white") << "~ " << r
                    << getColor("performance_info", "label_color", "white") << "CPU Usage                " << r
                    << getColor("performance_info", ":", "white") << ": " << r
                    << getColor("performance_info", "usage_value_color", "white") << perf.get_cpu_usage_percent() << r
                    << getColor("performance_info", "%", "white") << "%" << r;
                lp.push(ss.str());
            }

            // RAM Usage
            if (isSubEnabled("performance_info", "show_ram_usage")) {
                std::ostringstream ss;
                ss << getColor("performance_info", "~", "white") << "~ " << r
                    << getColor("performance_info", "label_color", "white") << "RAM Usage                " << r
                    << getColor("performance_info", ":", "white") << ": " << r
                    << getColor("performance_info", "usage_value_color", "white") << perf.get_ram_usage_percent() << r
                    << getColor("performance_info", "%", "white") << "%" << r;
                lp.push(ss.str());
            }

            // Disk Usage
            if (isSubEnabled("performance_info", "show_disk_usage")) {
                std::ostringstream ss;
                ss << getColor("performance_info", "~", "white") << "~ " << r
                    << getColor("performance_info", "label_color", "white") << "Disk Usage               " << r
                    << getColor("performance_info", ":", "white") << ": " << r
                    << getColor("performance_info", "usage_value_color", "white") << perf.get_disk_usage_percent() << r
                    << getColor("performance_info", "%", "white") << "%" << r;
                lp.push(ss.str());
            }

            // GPU Usage
            if (isSubEnabled("performance_info", "show_gpu_usage")) {
                std::ostringstream ss;
                ss << getColor("performance_info", "~", "white") << "~ " << r
                    << getColor("performance_info", "label_color", "white") << "GPU Usage                " << r
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
            // Declare this here so it's accessible to the loop regardless of header status
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
            // FIX: Declare inputDevices here so the loop below can find it!
            vector<AudioDevice> inputDevices = audio.get_input_devices();

            if (isSubEnabled("audio_power_info", "show_input_header")) {
                std::ostringstream ss;
                ss << getColor("audio_power_info", "#-", "white") << "#- " << r
                    << getColor("audio_power_info", "header_text_color", "white") << "Audio Input " << r
                    << getColor("audio_power_info", "separator_line", "white")
                    << "-----------------------------------------------------#" << r;
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
                        << getColor("audio_power_info", "label_color", "white") << "Battery powered " << r
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


    //----------------- END OF JSON-CONTROLLED COMPACT SECTIONS -----------------//

 

    // ---------------- End of info lines ----------------

    // Print remaining ASCII art lines (if art is taller than info)
    lp.finish();

    std::cout << std::endl;
    return 0;
}


/*




# BinaryFetch Main System - Comprehensive Documentation

## 📁 Project Overview

**BinaryFetch** is a Windows-based system information tool that displays detailed hardware/software specifications in an ASCII-art decorated terminal interface. The main orchestrator (`main.cpp`) integrates multiple specialized modules to gather and present system data with configurable visual formatting.

## 📊 Module Inventory

### **Core System Info Modules (12)**
1. **OSInfo** - Operating system details
2. **CPUInfo** - Processor specifications
3. **MemoryInfo** - RAM capacity, usage, and modules
4. **GPUInfo** - Basic graphics card information
5. **DetailedGPUInfo** - Advanced GPU metrics
6. **StorageInfo** - Disk drives and partitions
7. **NetworkInfo** - Network adapters and connectivity
8. **UserInfo** - User account and PC details
9. **PerformanceInfo** - Real-time system performance
10. **DisplayInfo** - Monitor configurations
11. **ExtraInfo** - Miscellaneous system data (audio, power)
12. **SystemInfo** - Motherboard and BIOS information

### **Compact Mode Modules (12)**
13. **CompactAudio** - Audio device summary
14. **CompactOS** - Lightweight OS summary
15. **CompactCPU** - Lightweight CPU summary
16. **CompactMemory** - Lightweight RAM summary
17. **CompactScreen** - Display resolution summary
18. **CompactSystem** - System board summary
19. **CompactGPU** - Graphics card summary
20. **CompactPerformance** - Performance statistics
21. **CompactUser** - User information summary
22. **CompactNetwork** - Network adapter summary
23. **DiskInfo** (compact_disk_info.h) - Storage summary
24. **TimeInfo** - Current date/time information

### **Utility Modules (3)**
25. **AsciiArt** - ASCII art loading and rendering
26. **LivePrinter** - Progressive output streaming
27. **nlohmann::json** - Configuration parsing

## 🏗️ Architecture Summary

| Component Type | Count | Purpose |
|---------------|-------|---------|
| Header Files | 28 | Modular system separation |
| Class Objects | 28+ | Instantiated information providers |
| JSON Config Sections | 20+ | Visual/behavior customization |
| Lambda Functions | 6+ | Configuration helpers |
| Stream Objects | 40+ | Output formatting |

## 🔧 Configuration System

### **Configuration Loading Strategy**
```cpp
bool LOAD_DEFAULT_CONFIG = true;  // Switch between development/production
// true = "Default_BinaryFetch_Config.json" (development)
// false = "%APPDATA%\\BinaryFetch\\BinaryFetch_Config.json" (production)
```

### **Configuration Path Management**
- **Production Path**: `C:\Users\Default\AppData\Local\BinaryFetch\`
- **Auto-creation**: Directory and config file created if missing
- **Fallback**: Copies from default if user config doesn't exist

### **Color System**
16 ANSI color codes mapped to human-readable names:
```cpp
std::map<std::string, std::string> colors = {
    {"red", "\033[31m"}, {"green", "\033[32m"}, // ... 14 more
    {"bright_white", "\033[97m"}, {"reset", "\033[0m"}
};
```

## 🎯 Configuration Helper Functions

### **Color Retrieval Lambda**
```cpp
auto getColor = [&](section, key, defaultColor) -> std::string
```
**Purpose**: Fetch color from JSON config with fallback
**Parameters**:
- `section`: JSON section name (e.g., "compact_os")
- `key`: Specific color key (e.g., "[OS]")
- `defaultColor`: Fallback if not found

### **Enablement Check Lambdas**
```cpp
auto isEnabled = [&](section) -> bool            // Main section toggle
auto isSubEnabled = [&](section, key) -> bool    // Sub-component toggle
auto isSectionEnabled = [&](module, section) -> bool  // Detailed module sections
auto isNestedEnabled = [&](module, section, key) -> bool  // Nested config
```

## 📋 Instantiated Objects (28 Total)

### **Information Providers**
```cpp
// Core System Info
OSInfo os;                    CPUInfo cpu;
MemoryInfo ram;               GPUInfo obj_gpu;
DetailedGPUInfo detailed_gpu_info; StorageInfo storage;
NetworkInfo net;              UserInfo user;
PerformanceInfo perf;         DisplayInfo display;
ExtraInfo extra;              SystemInfo sys;

// Compact Modules
CompactAudio c_audio;         CompactOS c_os;
CompactCPU c_cpu;             CompactScreen c_screen;
CompactMemory c_memory;       CompactSystem c_system;
CompactGPU c_gpu;             CompactPerformance c_perf;
CompactUser c_user;           CompactNetwork c_net;
DiskInfo disk;                TimeInfo time;

// UI/Utility
AsciiArt art;                 LivePrinter lp(art);
```

## 🎨 Output Sections Architecture

### **Section Processing Order**
1. **ASCII Art Loading** (`art.loadFromFile()`)
2. **Configuration Setup** (JSON parsing and helpers)
3. **Header** (BinaryFetch title)
4. **Compact Time** (Dynamic date/time display)
5. **Compact Modules** (12 summary sections)
6. **Detailed Modules** (In-depth system analysis)
7. **Footer** (Blank spacing and art completion)

### **Compact Sections (12)**
Each follows pattern: `[LABEL] -> data (subdata) @ units`
- **Time**: Dynamic HH:MM:SS, Date, Week, Leap year
- **OS**: Name, Build, Architecture, Uptime
- **CPU**: Model, Cores/Threads, Clock speed
- **Display**: Monitor brand, resolution, refresh rate
- **Memory**: Total, Free, Used percentage
- **Audio**: Input/Output devices with status
- **GPU**: Name, Usage%, VRAM, Frequency
- **Performance**: CPU/GPU/RAM/Disk usage percentages
- **User**: Username, Domain, Admin status
- **Network**: Name, Type, IP address
- **Disk**: Usage% per drive, Capacity per drive

### **Detailed Sections (9)**
Each with hierarchical formatting:
1. **Memory Info**: Module-by-module breakdown
2. **Storage Info**: Multi-section disk analysis
3. **Network Info**: Adapter details and speeds
4. **OS Info**: Kernel, install date, serial
5. **CPU Info**: Cache levels, virtualization, sockets
6. **GPU Info**: Multi-GPU support with primary details
7. **Display Info**: Per-monitor specifications
8. **BIOS & MB**: Vendor, version, model, manufacturer
9. **User Info**: Username, computer name, domain
10. **Performance**: System uptime and component usage
11. **Audio & Power**: Device lists and battery status

## 🔄 Data Flow Pattern

### **Standard Output Pipeline**
```
Module Data → String Stream → Color Formatting → LivePrinter → Terminal
```

### **Example: Compact CPU Section**
```cpp
if (isEnabled("compact_cpu")) {
    std::ostringstream ss;
    ss << getColor("compact_cpu", "[CPU]", "red") << "[CPU]" << r
       << getColor("compact_cpu", "->", "blue") << " -> " << r
       << getColor("compact_cpu", "name_color", "green")
       << c_cpu.getCPUName() << r;
    lp.push(ss.str());
}
```

## 🎪 LivePrinter Integration

### **Key Methods**
- `lp.push(string)`: Queues line for progressive output
- `lp.finish()`: Outputs remaining ASCII art lines
- Automatic ASCII art alignment with system info

### **Visual Layout**
```
┌─────────────────────────────────────────────────────┐
│  ██████╗ ██╗███╗   ██╗██╗   ██╗███████╗             │  [OS] -> Windows 11 Pro (64-bit)
│  ██╔══██╗██║████╗  ██║██║   ██║██╔════╝             │  [CPU] -> Intel i9-13900K (24C/32T) @ 5.8 GHz
│  ██████╔╝██║██╔██╗ ██║██║   ██║█████╗               │  [Memory] -> (total: 64GB) (free: 42GB) (87%)
│  ██╔══██╗██║██║╚██╗██║██║   ██║██╔══╝               │  ---
│  ██████╔╝██║██║ ╚████║╚██████╔╝███████╗             │
│  ╚═════╝ ╚═╝╚═╝  ╚═══╝ ╚═════╝ ╚══════╝             │
└─────────────────────────────────────────────────────┘
```

## ⚙️ JSON Configuration Structure

### **Top-Level Sections**
```json
{
  "header": { "enabled": true, "colors": {...} },
  "compact_time": {
    "enabled": true,
    "time_section": { "enabled": true, "colors": {...} },
    "date_section": { "enabled": true, "colors": {...} }
  },
  "compact_os": { "enabled": true, "show_name": true, ... },
  "detailed_memory": {
    "enabled": true,
    "sections": { "header": true, "total": true, ... },
    "colors": { "header_title": "red", "total_value": "yellow" }
  }
  // ... 20+ additional sections
}
```

## 📈 Performance Characteristics

### **Module Initialization Cost**
- **Heavy**: `StorageInfo`, `NetworkInfo` (WMI queries)
- **Medium**: `CPUInfo`, `GPUInfo` (performance counters)
- **Light**: `UserInfo`, `TimeInfo` (API calls)

### **Output Generation**
- **Progressive**: LivePrinter streams as sections complete
- **Buffered**: Each section builds complete string before output
- **Color-coded**: ANSI sequences inserted before streaming

## 🛠️ Development Guidelines

### **Adding New Sections**
1. Create module class with `getX()` methods
2. Add include in main.cpp header section
3. Instantiate object before JSON parsing
4. Add configuration section in JSON
5. Implement output block with color helpers
6. Follow existing formatting patterns

### **Code Organization Rules**
- **NO logic in main()**: Only orchestration calls
- **Module independence**: Each class self-contained
- **Configuration-driven**: All visuals via JSON
- **Error resilience**: Continue if module fails
- **Resource cleanup**: Automatic RAII management

### **Testing Considerations**
- Toggle `LOAD_DEFAULT_CONFIG` for development
- Validate JSON syntax after changes
- Test with/without admin privileges
- Verify multi-GPU/multi-monitor scenarios
- Check color support in target terminals

## 🔍 Key Variables Reference

| Variable | Type | Purpose | Scope |
|----------|------|---------|-------|
| `config` | `nlohmann::json` | Parsed configuration | Main |
| `colors` | `map<string,string>` | ANSI color mapping | Main |
| `r` | `string` | Reset color shorthand | Main |
| `config_loaded` | `bool` | JSON parse success flag | Main |
| `userConfigPath` | `string` | Production config path | Main |
| `configDir` | `string` | AppData directory path | Main |
| `LOAD_DEFAULT_CONFIG` | `bool` | Dev/production toggle | Main |

## 🚨 Error Handling Strategy

### **Graceful Degradation**
- ASCII art failure → Continue without art
- Module failure → Skip section with warning
- JSON parse failure → Use default colors
- Permission issues → Show partial information

### **Recovery Points**
1. ASCII art loading (`art.loadFromFile()`)
2. Configuration loading (`config_file.is_open()`)
3. Module initialization (try-catch in modules)
4. Output streaming (continue on stream failure)

## 📁 File Dependencies

### **Required Headers (31 total)**
- System: `<iostream>`, `<iomanip>`, `<vector>`, `<functional>`
- Windows: `<windows.h>`, `<shlobj.h>`, `<direct.h>`
- JSON: `"nlohmann/json.hpp"`
- Project modules: `"AsciiArt.h"`, `"OSInfo.h"`, ... (28 modules)

### **Required Libraries**
- Standard C++17 runtime
- Windows API libraries
- nlohmann JSON (header-only)

## 🔮 Extension Points

### **Easy Additions**
1. **New compact module**: Follow `CompactX.h` pattern
2. **New detailed section**: Add to JSON and output block
3. **New data points**: Extend existing modules
4. **Color schemes**: JSON color mapping additions

### **Architectural Extensions**
1. **Export formats**: JSON/XML output methods
2. **Historical tracking**: Database logging
3. **Remote monitoring**: Network transmission
4. **Plugin system**: Dynamic module loading

## 📝 Maintenance Notes

### **Common Issues**
- WMI query permissions require admin rights
- ANSI colors may not work in all terminals
- Very wide ASCII art can overflow terminal
- Disk speed measurement may slow startup

### **Optimization Opportunities**
- Lazy initialization of heavy modules
- Cache results for repeated queries
- Parallel module data gathering
- Progressive output streaming

This architecture supports team development through clear separation of concerns, standardized interfaces, and comprehensive configuration-driven customization. Each team member can own specific modules while maintaining consistent integration patterns.















*/