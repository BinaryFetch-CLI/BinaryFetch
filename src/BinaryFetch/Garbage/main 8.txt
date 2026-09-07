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


int main() {
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
    CompactScreen c_screen;
    CompactMemory c_memory;
    CompactSystem c_system;
    CompactGPU c_gpu;
    CompactPerformance c_perf;
    CompactUser c_user;
    CompactNetwork c_net;
    DiskInfo disk;


    TimeInfo time;

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

    // 3. Helper functions - FIXED VERSION
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
        if (isEnabled("compact_time")) {
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
            if (isSubEnabled("compact_time", "time_section.enabled")) {
                ss << getTimeColor("time_section", "bracket", "white") << "(" << r;

                if (isSubEnabled("compact_time", "time_section.show_label")) {
                    ss << getTimeColor("time_section", "label", "white") << "Time: " << r;
                }

                bool wrote = false;

                if (isSubEnabled("compact_time", "time_section.show_hour")) {
                    ss << getTimeColor("time_section", "hour", "white")
                        << std::setw(2) << std::setfill('0') << time.getHour() << r;
                    wrote = true;
                }

                if (isSubEnabled("compact_time", "time_section.show_minute")) {
                    if (wrote) ss << getTimeColor("time_section", "sep", "white") << ":" << r;
                    ss << getTimeColor("time_section", "minute", "white")
                        << std::setw(2) << std::setfill('0') << time.getMinute() << r;
                    wrote = true;
                }

                if (isSubEnabled("compact_time", "time_section.show_second")) {
                    if (wrote) ss << getTimeColor("time_section", "sep", "white") << ":" << r;
                    ss << getTimeColor("time_section", "second", "white")
                        << std::setw(2) << std::setfill('0') << time.getSecond() << r;
                }

                ss << getTimeColor("time_section", "bracket", "white") << ") " << r;
            }

            // ---------- DATE SECTION ----------
            if (isSubEnabled("compact_time", "date_section.enabled")) {
                ss << getTimeColor("date_section", "bracket", "white") << "(" << r;

                if (isSubEnabled("compact_time", "date_section.show_label")) {
                    ss << getTimeColor("date_section", "label", "white") << "Date: " << r;
                }

                if (isSubEnabled("compact_time", "date_section.show_day")) {
                    ss << getTimeColor("date_section", "day", "white")
                        << std::setw(2) << std::setfill('0') << time.getDay() << r << " ";
                }

                if (isSubEnabled("compact_time", "date_section.show_day_name")) {
                    ss << getTimeColor("date_section", "day_name", "white")
                        << time.getDayName() << r;
                }

                if (isSubEnabled("compact_time", "date_section.show_month_name")) {
                    ss << getTimeColor("date_section", "sep", "white") << ":" << r
                        << getTimeColor("date_section", "month_name", "white")
                        << time.getMonthName() << r << " ";
                }

                if (isSubEnabled("compact_time", "date_section.show_month_num")) {
                    ss << getTimeColor("date_section", "month_num", "white")
                        << time.getMonthNumber() << r;
                }

                if (isSubEnabled("compact_time", "date_section.show_year")) {
                    ss << getTimeColor("date_section", "sep", "white") << ":" << r
                        << getTimeColor("date_section", "year", "white")
                        << time.getYearNumber() << r;
                }

                ss << getTimeColor("date_section", "bracket", "white") << ") " << r;
            }

            // ---------- WEEK SECTION ----------
            if (isSubEnabled("compact_time", "week_section.enabled")) {
                ss << getTimeColor("week_section", "bracket", "white") << "(" << r;

                if (isSubEnabled("compact_time", "week_section.show_label")) {
                    ss << getTimeColor("week_section", "label", "white") << "Week: " << r;
                }

                if (isSubEnabled("compact_time", "week_section.show_num")) {
                    ss << getTimeColor("week_section", "num", "white")
                        << time.getWeekNumber() << r;
                }

                ss << getTimeColor("week_section", "bracket", "white") << ") " << r;
            }

            // ---------- LEAP YEAR SECTION ----------
            if (isSubEnabled("compact_time", "leap_section.enabled")) {
                ss << getTimeColor("leap_section", "bracket", "white") << "(" << r;

                if (isSubEnabled("compact_time", "leap_section.show_label")) {
                    ss << getTimeColor("leap_section", "label", "white") << "Leap Year: " << r;
                }

                if (isSubEnabled("compact_time", "leap_section.show_val")) {
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
            ss << getColor("compact_os", "[OS]", "red") << "[OS]" << r
                << getColor("compact_os", "->", "blue") << "  -> " << r;

            if (isSubEnabled("compact_os", "show_name")) ss << getColor("compact_os", "name_color", "green") << c_os.getOSName() << r << " ";
            if (isSubEnabled("compact_os", "show_build")) ss << getColor("compact_os", "build_color", "yellow") << c_os.getOSBuild() << r;

            if (isSubEnabled("compact_os", "show_arch")) {
                ss << getColor("compact_os", "(", "red") << " (" << r
                    << getColor("compact_os", "arch_color", "cyan") << c_os.getArchitecture() << r
                    << getColor("compact_os", ")", "red") << ")" << r;
            }

            if (isSubEnabled("compact_os", "show_uptime")) {
                ss << getColor("compact_os", "(", "red") << " (" << r
                    << getColor("compact_os", "uptime_label_color", "green") << "uptime: " << r
                    << getColor("compact_os", "uptime_value_color", "magenta") << c_os.getUptime() << r
                    << getColor("compact_os", ")", "red") << ")" << r;
            }
            lp.push(ss.str());
        }

        // Compact CPU
        if (isEnabled("compact_cpu")) {
            std::ostringstream ss;
            ss << getColor("compact_cpu", "[CPU]", "red") << "[CPU]" << r
                << getColor("compact_cpu", "->", "blue") << " -> " << r;

            if (isSubEnabled("compact_cpu", "show_name")) ss << getColor("compact_cpu", "name_color", "green") << c_cpu.getCPUName() << r;

            if (isSubEnabled("compact_cpu", "show_cores") || isSubEnabled("compact_cpu", "show_threads")) {
                ss << getColor("compact_cpu", "(", "red") << " (" << r;
                if (isSubEnabled("compact_cpu", "show_cores")) ss << getColor("compact_cpu", "core_color", "yellow") << c_cpu.getCPUCores() << r << getColor("compact_cpu", "text_color", "green") << "C" << r;
                if (isSubEnabled("compact_cpu", "show_cores") && isSubEnabled("compact_cpu", "show_threads")) ss << getColor("compact_cpu", "separator_color", "blue") << "/" << r;
                if (isSubEnabled("compact_cpu", "show_threads")) ss << getColor("compact_cpu", "thread_color", "yellow") << c_cpu.getCPUThreads() << r << getColor("compact_cpu", "text_color", "green") << "T" << r;
                ss << getColor("compact_cpu", ")", "red") << ")" << r;
            }
             
            if (isSubEnabled("compact_cpu", "show_clock")) {
                ss << std::fixed << std::setprecision(2)
                    << getColor("compact_cpu", "at_symbol_color", "green") << " @" << r
                    << getColor("compact_cpu", "clock_color", "cyan") << " " << c_cpu.getClockSpeed() << " GHz" << r;
            }
            lp.push(ss.str());
        }

        // Compact Display
        if (isEnabled("compact_display")) {
            auto screens = c_screen.get_screens();
            int idx = 1;
            if (screens.empty()) {
                lp.push(getColor("compact_display", "[Display]", "red") + "[Display]" + r +
                    getColor("compact_display", "->", "blue") + " -> " + r + "No displays detected");
            }
            else {
                for (const auto& s : screens) {
                    std::ostringstream ss;
                    ss << getColor("compact_display", "[Display]", "red") << "[Display " << idx++ << "]" << r
                        << getColor("compact_display", "->", "blue") << " -> " << r;

                    if (isSubEnabled("compact_display", "show_name")) ss << getColor("compact_display", "name_color", "green") << s.brand_name << r;
                    if (isSubEnabled("compact_display", "show_resolution")) {
                        ss << getColor("compact_display", "(", "red") << " (" << r
                            << getColor("compact_display", "resolution_color", "yellow") << s.resolution << r
                            << getColor("compact_display", ")", "red") << ") " << r;
                    }
                    if (isSubEnabled("compact_display", "show_refresh")) {
                        ss << getColor("compact_display", "at_symbol_color", "green") << "@" << r
                            << getColor("compact_display", "refresh_color", "cyan") << s.refresh_rate << "Hz" << r;
                    }
                    lp.push(ss.str());
                }
            }
        }

        // Compact Memory
        if (isEnabled("compact_memory")) {
            std::ostringstream ss;
            ss << getColor("compact_memory", "[Memory]", "red") << "[Memory]" << r
                << getColor("compact_memory", "->", "blue") << " -> " << r;

            if (isSubEnabled("compact_memory", "show_total")) {
                ss << getColor("compact_memory", "(", "red") << "(" << r
                    << getColor("compact_memory", "label_color", "green") << "total: " << r
                    << getColor("compact_memory", "total_color", "green") << c_memory.get_total_memory() << " GB" << r
                    << getColor("compact_memory", ")", "red") << ")" << r;
            }
            if (isSubEnabled("compact_memory", "show_free")) {
                ss << " " << getColor("compact_memory", "(", "red") << "(" << r
                    << getColor("compact_memory", "label_color", "green") << "free: " << r
                    << getColor("compact_memory", "free_color", "yellow") << c_memory.get_free_memory() << " GB" << r
                    << getColor("compact_memory", ")", "red") << ")" << r;
            }
            if (isSubEnabled("compact_memory", "show_percent")) {
                ss << " " << getColor("compact_memory", "(", "red") << "(" << r
                    << getColor("compact_memory", "percent_color", "magenta") << c_memory.get_used_memory_percent() << "%" << r
                    << getColor("compact_memory", ")", "red") << ")" << r;
            }
            lp.push(ss.str());
        }

        // Compact Audio
        if (isEnabled("compact_audio")) {
            if (isSubEnabled("compact_audio", "show_input")) {
                std::ostringstream ss1;
                ss1 << getColor("compact_audio", "[Audio Input]", "red") << "[Audio Input]" << r
                    << getColor("compact_audio", "->", "blue") << " -> " << r
                    << getColor("compact_audio", "device_color", "green") << c_audio.active_audio_input() << r << " "
                    << getColor("compact_audio", "(", "red") << "[" << r
                    << getColor("compact_audio", "status_color", "yellow") << c_audio.active_audio_input_status() << r
                    << getColor("compact_audio", ")", "red") << "]" << r;
                lp.push(ss1.str());
            }
            if (isSubEnabled("compact_audio", "show_output")) {
                std::ostringstream ss2;
                ss2 << getColor("compact_audio", "[Audio Output]", "red") << "[Audio Output]" << r
                    << getColor("compact_audio", "->", "blue") << " -> " << r
                    << getColor("compact_audio", "device_color", "green") << c_audio.active_audio_output() << r << " "
                    << getColor("compact_audio", "(", "red") << "[" << r
                    << getColor("compact_audio", "status_color", "yellow") << c_audio.active_audio_output_status() << r
                    << getColor("compact_audio", ")", "red") << "]" << r;
                lp.push(ss2.str());
            }
        }

        // Compact GPU
        if (isEnabled("compact_gpu")) {
            std::ostringstream ss;
            ss << getColor("compact_gpu", "[GPU]", "red") << "[GPU]" << r
                << getColor("compact_gpu", "->", "blue") << " -> " << r;

            if (isSubEnabled("compact_gpu", "show_name")) ss << getColor("compact_gpu", "name_color", "blue") << c_gpu.getGPUName() << r;

            if (isSubEnabled("compact_gpu", "show_usage")) {
                ss << getColor("compact_gpu", "(", "red") << " (" << r
                    << getColor("compact_gpu", "usage_color", "yellow") << c_gpu.getGPUUsagePercent() << "%" << r
                    << getColor("compact_gpu", ")", "red") << ")" << r;
            }

            if (isSubEnabled("compact_gpu", "show_vram")) {
                ss << getColor("compact_gpu", "(", "red") << " (" << r
                    << getColor("compact_gpu", "vram_color", "cyan") << c_gpu.getVRAMGB() << " GB" << r
                    << getColor("compact_gpu", ")", "red") << ")" << r;
            }

            if (isSubEnabled("compact_gpu", "show_freq")) {
                ss << getColor("compact_gpu", "(", "red") << " (" << r
                    << getColor("compact_gpu", "at_symbol_color", "green") << "@" << r
                    << getColor("compact_gpu", "freq_color", "magenta") << c_gpu.getGPUFrequency() << r
                    << getColor("compact_gpu", ")", "red") << ")" << r;
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
                    << getColor("compact_network", "name_color", "green") << "Interblink" << r
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
                    << getColor("compact_network", "ip_color", "magenta") << "123.23.423.1" << r
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

            // IP (dummy / real handled elsewhere)
            if (isSubEnabled("network_info", "show_ip")) {
                std::ostringstream ss;
                ss << getColor("network_info", "~", "blue") << "~ " << r
                    << getColor("network_info", "label_color", "green")
                    << "IP                        " << r
                    << getColor("network_info", ":", "blue") << ": " << r
                    << getColor("network_info", "ip_value_color", "magenta")
                    << "12.23.34.5.345" << r;
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
                    << "------------------------------------------------------#" << r;
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

    



    // ---------------- Now stream info line-by-line ----------------
    // please don't use "cout" directly - use lp.push() instead!
    // "cout" could break the ascii art alignment.
    // Each lp.push() call prints the next art line + info line.
    // Don't use: cout << "some info" << endl;
    // Instead use: (example)
    // {
    //    std::ostringstream ss;
    //    ss << "~>> " << "BinaryFetch" << "_____________________________________________________";
    //    lp.push(ss.str());
    // }

    // Header
    //lp.push(""); // keep art line similar as cout << endl; 


    //comment the previous header of binaryfetch
    //lp.push("~>> BinaryFetch_____________________________________________________");

    // ANSI color definitions---------------------------------------------------

    const std::string reset = "\033[0m";
    // -----------------------------
    // ANSI RESET CODE
    // -----------------------------
    // The 'reset' string holds the ANSI escape sequence to reset
    // all terminal text formatting, including colors, bold, underline, etc.
    // Usage:
    //     std::cout << "\033[31mRed Text" << reset << " Normal Text\n";
    // This ensures that after printing colored text, subsequent output
    // returns to the terminal's default style.
    // ---------------------------------------------------------------------------


    const std::string red = "\033[31m";
    const std::string green = "\033[32m";
    const std::string yellow = "\033[33m";
    const std::string blue = "\033[34m";
    const std::string magenta = "\033[35m";
    const std::string cyan = "\033[36m";
    const std::string white = "\033[37m";
    const std::string bright_red = "\033[91m";
    const std::string bright_green = "\033[92m";
    const std::string bright_yellow = "\033[93m";
    const std::string bright_blue = "\033[94m";
    const std::string bright_magenta = "\033[95m";
    const std::string bright_cyan = "\033[96m";
    const std::string bright_white = "\033[97m";


    //BinaryFetch Header-----------------------------------------------#
    {
        // Colored header
        std::ostringstream ss;
        ss << bright_red << "~>> " << reset
            << green << "BinaryFetch" << reset
            << red << "_____________________________________________________" << reset;
        lp.push(ss.str());
    }
//---------------------beginning of compact section-----------------------

/**/


// compact  OS----------------------------------------------------------
    {
        std::ostringstream ss;
        ss << red << "[OS]" << reset << blue << "  -> " << reset
            << green << c_os.getOSName() << reset << " "
            << yellow << c_os.getOSBuild() << reset
            << red << " (" << reset << cyan << c_os.getArchitecture() << reset << red << ")" << reset
            << red << " (" << reset << green << "uptime: " << reset << magenta << c_os.getUptime() << reset << red << ")" << reset;
        lp.push(ss.str());
    }

    // compact CPU ----------------------------------------------------------
    {
        std::ostringstream ss;
        ss << red << "[CPU]" << reset << blue << " -> " << reset
            << green << c_cpu.getCPUName() << reset
            << red << " (" << reset << yellow << c_cpu.getCPUCores() << reset << green << "C" << reset << blue << "/" << reset
            << yellow << c_cpu.getCPUThreads() << reset << green << "T" << reset << red << ")" << reset
            << std::fixed << std::setprecision(2)
            << green << " @" << reset << cyan << " " << c_cpu.getClockSpeed() << " GHz" << reset;
        lp.push(ss.str());
    }

    // compact Displays ----------------------------------------------------------
    {
        auto screens = c_screen.get_screens();
        int idx = 1;
        if (screens.empty()) {
            lp.push(red + "[Display]" + reset + blue + " -> " + reset + "No displays detected");
        }
        else {
            for (const auto& s : screens) {
                std::ostringstream ss;
                ss << red << "[Display " << idx++ << "]" << reset << blue << " -> " << reset
                    << green << s.brand_name << reset
                    << red << " (" << reset << yellow << s.resolution << reset << red << ") " << reset
                    << green << "@" << reset << cyan << s.refresh_rate << "Hz" << reset;
                lp.push(ss.str());
            }
        }
    }

    // compact memory (ram) ----------------------------------------------------------
    {
        std::ostringstream ss;
        ss << red << "[Memory]" << reset << blue << " -> " << reset
            << red << "(" << reset << green << "total: " << reset << green << c_memory.get_total_memory() << " GB" << reset << red << ")" << reset
            << red << " (" << reset << green << "free: " << reset << yellow << c_memory.get_free_memory() << " GB" << reset << red << ")" << reset
            << red << " (" << reset << magenta << c_memory.get_used_memory_percent() << "%" << reset << red << ")" << reset;
        lp.push(ss.str());
    }

    //compact Audio----------------------------------------------------------
    {
        std::ostringstream ss1, ss2;
        ss1 << red << "[Audio Input]" << reset << blue << " -> " << reset
            << green << c_audio.active_audio_input() << reset << " "
            << red << "[" << reset << yellow << c_audio.active_audio_input_status() << reset << red << "]" << reset;
        ss2 << red << "[Audio Output]" << reset << blue << " -> " << reset
            << green << c_audio.active_audio_output() << reset << " "
            << red << "[" << reset << yellow << c_audio.active_audio_output_status() << reset << red << "]" << reset;
        lp.push(ss1.str());
        lp.push(ss2.str());
    }

    // GPU minimal----------------------------------------------------------
    {
        std::ostringstream ss;
        ss << red << "[GPU]" << reset << blue << " -> " << reset
            << blue << c_gpu.getGPUName() << reset
            << red << " (" << reset << yellow << c_gpu.getGPUUsagePercent() << "%" << reset << red << ")" << reset
            << red << " (" << reset << cyan << c_gpu.getVRAMGB() << " GB" << reset << red << ")" << reset
            << red << " (" << reset << green << "@" << reset << magenta << c_gpu.getGPUFrequency() << reset << red << ")" << reset;
        lp.push(ss.str());
    }

    // Minimal Performance----------------------------------------------------------
    {
        std::ostringstream ss;
        ss << red << "[Performance]" << reset << blue << " -> " << reset
            << red << "(" << reset << green << "CPU: " << reset << green << c_perf.getCPUUsage() << "%" << reset << red << ") " << reset
            << red << "(" << reset << green << "GPU: " << reset << yellow << c_perf.getGPUUsage() << "%" << reset << red << ") " << reset
            << red << "(" << reset << green << "RAM: " << reset << cyan << c_perf.getRAMUsage() << "%" << reset << red << ") " << reset
            << red << "(" << reset << green << "Disk: " << reset << magenta << c_perf.getDiskUsage() << "%" << reset << red << ")" << reset;
        lp.push(ss.str());
    }

    // compact User----------------------------------------------------------
    {
        std::ostringstream ss;
        ss << red << "[User]" << reset << blue << " -> " << reset
            << green << "@" << c_user.getUsername() << reset
            << blue << " -> " << reset << red << "(" << reset << green << "Domain: " << reset << yellow << c_user.getDomain() << reset << red << ")" << reset
            << blue << " -> " << reset << red << "(" << reset << green << "Type: " << reset << cyan << c_user.isAdmin() << reset << red << ")" << reset;
        lp.push(ss.str());
    }


  
  // Network minimal (commented)
   {
       std::ostringstream ss;
       ss << red << "[network]" << reset << blue << " -> " << reset
          << green << "(" << reset << green << "Name: " << reset << green << c_net.get_network_name() << reset << green << ") " << reset
          << green << "(" << reset << green << "Type: " << reset << yellow << c_net.get_network_type() << reset << green << ") " << reset
          << green << "(" << reset << green << "ip: " << reset << magenta << c_net.get_network_ip() << reset << green << ")" << reset;
       lp.push(ss.str());
   }
   

    

    // Disk usage (compact)----------------------------------------------------------
    {
        auto disks = disk.getAllDiskUsage();
        std::ostringstream ss;
        ss << red << "[Disk]" << reset << blue << " -> " << reset;
        for (const auto& d : disks) {
            ss << red << "(" << reset << green << d.first[0] << ":" << reset
                << " " << yellow << std::fixed << std::setprecision(1) << d.second << "%" << reset << red << ") " << reset;
        }
        lp.push(ss.str());

        // Disk capacities----------------------------------------------------------
        auto caps = disk.getDiskCapacity();
        std::ostringstream sc;
        sc << red << "[Disk Cap]" << reset << blue << " -> " << reset;
        for (const auto& c : caps) {
            sc << red << "(" << reset << green << c.first[0] << reset << blue << "-" << reset
                << yellow << c.second << "GB" << reset << red << ")" << reset;
        }
        lp.push(sc.str());
    }

    //-------------------------------------end of compact section---------------------------------

    // Full detailed section (Memory Info)
    {
        lp.push(""); // blank line
        {
            {
                std::ostringstream ss;
                ss << blue << "----------------" << reset
                    << red << "Memory Info" << reset
                    << blue << "---------------" << reset;
                lp.push(ss.str());
            }



            // RAM minimal
            {
                std::ostringstream ss;

                // Prefix
                // ss << red << "[RAM]" << reset << blue << " -> " << reset;
                // Total Section
                ss << blue << "(" << reset << green << "Total: " << reset
                    << yellow << ram.getTotal() << reset << green << " GB" << reset << blue << ") " << reset;
                // Free Section
                ss << blue << "(" << reset << green << "Free: " << reset
                    << cyan << ram.getFree() << reset << green << " GB" << reset << blue << ") " << reset;
                // Used Section
                ss << blue << "(" << reset << green << "Used: " << reset
                    << red << ram.getUsedPercentage() << reset << red << "%" << reset << blue << ")" << reset;
                lp.push(ss.str());
            }

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

            {
                std::ostringstream ss;
                // Structural Marker and Label
                ss << blue << "~ " << reset << magenta << "Memory " << i << reset
                    << blue << ": " << reset;

                // Used Percentage in parentheses
                ss << blue << "(" << reset << green << "Used: " << reset
                    << red << ram.getUsedPercentage() << reset << red << "%" << reset
                    << blue << ") " << reset;

                // Capacity, Type, and Speed
                ss << green << capOut.str() << reset << " "
                    << cyan << modules[i].type << reset << " "
                    << yellow << modules[i].speed << reset;

                lp.push(ss.str());
            }

        }
    }







    // Storage Info (detailed) - STREAMING VERSION
    {
        lp.push("");
        {
            std::ostringstream ss;
            ss << red << "------------------------- " << reset
                << blue << "STORAGE SUMMARY" << reset
                << red << " --------------------------" << reset;
            lp.push(ss.str());
        }


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
            // Keeping your original structure: [ (Used) fmt GiB / fmt GiB % - fs Ext ]
            ss << yellow << d.storage_type << reset << " "
                << cyan << d.drive_letter << reset
                << green << " [" << reset << green << " (" << reset << green << "Used" << reset << green << ") " << reset
                << green << fmt_storage(d.used_space) << reset
                << green << " GiB " << reset << green << "/" << reset << green << fmt_storage(d.total_space) << reset
                << green << " GiB  " << reset << red << d.used_percentage << reset
                << green << " - " << reset << magenta << d.file_system << reset << " "
                << (d.is_external ? blue : white) << (d.is_external ? "Ext" : "Int") << reset
                << green << " ]" << reset;
            lp.push(ss.str());

            });

        // Now print performance sections using captured data
        if (!all_disks_captured.empty()) {
            lp.push("");

            {
                std::ostringstream ss;
                ss << red << "-------------------- " << reset
                    << blue << "DISK PERFORMANCE & DETAILS" << reset
                    << red << " --------------------" << reset;
                lp.push(ss.str());
            }


            for (const auto& d : all_disks_captured) {

                std::ostringstream ss;
                ss << cyan << d.drive_letter << reset
                    << green << " [" << reset << " "
                    << green << "Read:" << reset << " "
                    << yellow << fmt_speed(d.read_speed) << reset
                    << green << " MB/s " << reset << green << "|" << reset << " "
                    << green << "Write:" << reset << " "
                    << yellow << fmt_speed(d.write_speed) << reset
                    << green << " MB/s " << reset << green << "|" << reset << " "
                    << magenta << d.serial_number << reset
                    << (d.is_external ? blue : white) << (d.is_external ? " Ext" : " Int") << reset
                    << green << " ]" << reset;
                lp.push(ss.str());
            }

            lp.push("");
            {
                std::ostringstream ss;
                ss << red << "---------------- " << reset
                    << blue << "DISK PERFORMANCE & DETAILS (Predicted)" << reset
                    << red << " ------------" << reset;
                lp.push(ss.str());
            }


            for (const auto& d : all_disks_captured) {
                std::ostringstream ss;
                ss << cyan << d.drive_letter << reset
                    << green << " [" << reset << " "
                    << green << "Read: " << reset
                    << yellow << fmt_speed(d.predicted_read_speed) << reset
                    << green << " MB/s " << reset << green << "|" << reset << " "
                    << green << "Write: " << reset
                    << yellow << fmt_speed(d.predicted_write_speed) << reset
                    << green << " MB/s " << reset << green << "|" << reset << " "
                    << magenta << d.serial_number << reset
                    << (d.is_external ? blue : white) << (d.is_external ? " Ext" : " Int") << reset
                    << green << " ]" << reset;
                lp.push(ss.str());
            }
        }
        else {
            lp.push("No drives detected.");
        }
    }

    // Network (Compact + Extra)
    {
        {
            std::ostringstream ss;
            ss << blue << "#- " << reset << green << "Network Info " << reset
                << blue << "----------------------------------------------------#" << reset;
            lp.push(ss.str());
        }

        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "Network Name              " << reset
                << blue << ": " << reset << yellow << net.get_network_name() << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "Network Type              " << reset
                << blue << ": " << reset << yellow << c_net.get_network_type() << reset;
            lp.push(ss.str());
        }

        /* temporary hide public ip due to privacy concerns
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "IP                        " << reset
               << blue << ": " << reset << magenta << net.get_public_ip() << reset;
            lp.push(ss.str());
        }
        */
        {
            // dummy public ip for testing and privacy
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "IP                        " << reset
                << blue << ": " << reset << magenta << "12.23.34.5.345" << reset;
            lp.push(ss.str());
        }

        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "Locale                    " << reset
                << blue << ": " << reset << cyan << net.get_locale() << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "Mac address               " << reset
                << blue << ": " << reset << cyan << net.get_mac_address() << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "avg upload speed          " << reset
                << blue << ": " << reset << yellow << net.get_network_upload_speed() << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "avg download speed        " << reset
                << blue << ": " << reset << yellow << net.get_network_download_speed() << reset;
            lp.push(ss.str());
        }
    }




    // OS Info
    {
        lp.push("");
        {
            std::ostringstream ss;
            ss << blue << "#- " << reset << green << "Operating System " << reset
                << blue << "-----------------------------------------------#" << reset;
            lp.push(ss.str());
        }

        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "Name                      " << reset
                << blue << ": " << reset << yellow << os.GetOSName() << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "Build                     " << reset
                << blue << ": " << reset << yellow << os.GetOSVersion() << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "Architecture              " << reset
                << blue << ": " << reset << cyan << os.GetOSArchitecture() << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "Kernel                    " << reset
                << blue << ": " << reset << cyan << os.get_os_kernel_info() << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "Uptime                    " << reset
                << blue << ": " << reset << magenta << os.get_os_uptime() << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "Install Date              " << reset
                << blue << ": " << reset << magenta << os.get_os_install_date() << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "Serial                    " << reset
                << blue << ": " << reset << yellow << os.get_os_serial_number() << reset;
            lp.push(ss.str());
        }
    }


    // CPU Info
    {
        lp.push("");
        {
            std::ostringstream ss;
            ss << blue << "#- " << reset << green << "CPU Info " << reset
                << blue << "-------------------------------------------------------#" << reset;
            lp.push(ss.str());
        }

        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "Brand                    " << reset
                << blue << ": " << reset << yellow << cpu.get_cpu_info() << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "Utilization              " << reset
                << blue << ": " << reset << red << cpu.get_cpu_utilization() << reset << green << "%" << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "Speed                    " << reset
                << blue << ": " << reset << cyan << cpu.get_cpu_speed() << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "Base Speed               " << reset
                << blue << ": " << reset << cyan << cpu.get_cpu_base_speed() << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "Cores                    " << reset
                << blue << ": " << reset << yellow << cpu.get_cpu_cores() << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "Logical Processors       " << reset
                << blue << ": " << reset << yellow << cpu.get_cpu_logical_processors() << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "Sockets                  " << reset
                << blue << ": " << reset << yellow << cpu.get_cpu_sockets() << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "Virtualization           " << reset
                << blue << ": " << reset << cyan << cpu.get_cpu_virtualization() << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "L1 Cache                 " << reset
                << blue << ": " << reset << magenta << cpu.get_cpu_l1_cache() << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "L2 Cache                 " << reset
                << blue << ": " << reset << magenta << cpu.get_cpu_l2_cache() << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "L3 Cache                 " << reset
                << blue << ": " << reset << magenta << cpu.get_cpu_l3_cache() << reset;
            lp.push(ss.str());
        }
    }

   
// GPU Info (detailed)-------------------------------------------------------------
    {
        lp.push("");
        auto all_gpu_info = obj_gpu.get_all_gpu_info();
        if (all_gpu_info.empty()) {
            std::ostringstream ss;
            ss << blue << "#- " << reset << green << "GPU Info " << reset
                << blue << "--------------------------------------------------------#" << reset;
            lp.push(ss.str());

            lp.push(red + "No GPU detected." + reset);
        }
        else {
            {
                std::ostringstream ss;
                ss << blue << "#- " << reset << green << "GPU Info " << reset
                    << blue << "-------------------------------------------------------#" << reset;
                lp.push(ss.str());
            }

            for (size_t i = 0; i < all_gpu_info.size(); ++i) {
                auto& g = all_gpu_info[i];

                // GPU index line
                {
                    std::ostringstream label;
                    if (i == 0) {
                        label << blue << "GPU " << (i + 1) << reset;
                    }
                    else {
                        label << blue << "#-" << reset << green << "GPU " << (i + 1) << reset
                            << blue << "------------------------------------------------------------#" << reset;
                    }

                    std::string lbl = label.str();
                    // Note: Padding logic remains, but be aware color codes add 'invisible' length
                    if (lbl.length() < 27) lbl += std::string(27 - lbl.length(), ' ');
                    lp.push(lbl);
                }

                {
                    std::ostringstream ss;
                    ss << blue << "|-> " << reset << green << "Name                   " << reset
                        << blue << ": " << reset << yellow << g.gpu_name << reset;
                    lp.push(ss.str());
                }
                {
                    std::ostringstream ss;
                    ss << blue << "|-> " << reset << green << "Memory                 " << reset
                        << blue << ": " << reset << cyan << g.gpu_memory << reset;
                    lp.push(ss.str());
                }
                {
                    std::ostringstream ss;
                    ss << blue << "|-> " << reset << green << "Usage                  " << reset
                        << blue << ": " << reset << red << g.gpu_usage << reset << green << "%" << reset;
                    lp.push(ss.str());
                }
                {
                    std::ostringstream ss;
                    ss << blue << "|-> " << reset << green << "Vendor                 " << reset
                        << blue << ": " << reset << yellow << g.gpu_vendor << reset;
                    lp.push(ss.str());
                }
                {
                    std::ostringstream ss;
                    ss << blue << "|-> " << reset << green << "Driver Version         " << reset
                        << blue << ": " << reset << cyan << g.gpu_driver_version << reset;
                    lp.push(ss.str());
                }
                {
                    std::ostringstream ss;
                    ss << blue << "|-> " << reset << green << "Temperature            " << reset
                        << blue << ": " << reset << magenta << g.gpu_temperature << reset << green << " C" << reset;
                    lp.push(ss.str());
                }
                {
                    std::ostringstream ss;
                    ss << blue << "#-> " << reset << green << "Core Count             " << reset
                        << blue << ": " << reset << yellow << g.gpu_core_count << reset;
                    lp.push(ss.str());
                }
            }

            auto primary = detailed_gpu_info.primary_gpu_info();
            {
                std::ostringstream ss;
                ss << blue << "#- " << reset << green << "Primary GPU Details" << reset
                    << blue << "---------------------------------------------#" << reset;
                lp.push(ss.str());
            }
            {
                std::ostringstream ss;
                ss << blue << "|-> " << reset << green << "Name                   " << reset
                    << blue << ": " << reset << yellow << primary.name << reset;
                lp.push(ss.str());
            }
            {
                std::ostringstream ss;
                ss << blue << "|-> " << reset << green << "VRAM                   " << reset
                    << blue << ": " << reset << cyan << primary.vram_gb << reset << green << " GiB" << reset;
                lp.push(ss.str());
            }
            {
                std::ostringstream ss;
                ss << blue << "#-> " << reset << green << "Frequency              " << reset
                    << blue << ": " << reset << magenta << primary.frequency_ghz << reset << green << " GHz" << reset;
                lp.push(ss.str());
            }
        }
    }

    // Display Info
    {
        lp.push("");
        auto monitors = display.get_all_displays();
        if (monitors.empty()) {
            std::ostringstream ss;
            ss << blue << "#- " << reset << red << "Display" << reset
                << blue << " ----------------------------------------------------------#" << reset;
            lp.push(ss.str());
            lp.push(red + "No monitors detected." + reset);
        }
        else {
            for (size_t i = 0; i < monitors.size(); ++i) {
                auto& m = monitors[i];
                {
                    std::ostringstream ss;
                    ss << blue << "#- " << reset << green << "Monitor " << (i + 1) << " " << reset
                        << blue << "--------------------------------------------------------#" << reset;
                    lp.push(ss.str());
                }
                {
                    std::ostringstream ss;
                    ss << blue << "~ " << reset << green << "Brand                    " << reset
                        << blue << ": " << reset << yellow << m.brand_name << reset;
                    lp.push(ss.str());
                }
                {
                    std::ostringstream ss;
                    ss << blue << "~ " << reset << green << "Resolution               " << reset
                        << blue << ": " << reset << cyan << m.resolution << reset;
                    lp.push(ss.str());
                }
                {
                    std::ostringstream ss;
                    ss << blue << "~ " << reset << green << "Refresh Rate             " << reset
                        << blue << ": " << reset << magenta << m.refresh_rate << reset << green << " Hz" << reset;
                    lp.push(ss.str());
                }
            }
        }
    }


    // BIOS & Motherboard Info
    {
        lp.push("");
        {
            std::ostringstream ss;
            ss << blue << "#- " << reset << green << "BIOS & Motherboard Info " << reset
                << blue << "----------------------------------------#" << reset;
            lp.push(ss.str());
        }

        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "Bios Vendor              " << reset
                << blue << ": " << reset << yellow << sys.get_bios_vendor() << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "Bios Version             " << reset
                << blue << ": " << reset << cyan << sys.get_bios_version() << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "Bios Date                " << reset
                << blue << ": " << reset << magenta << sys.get_bios_date() << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "Motherboard Model        " << reset
                << blue << ": " << reset << yellow << sys.get_motherboard_model() << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "Motherboard Manufacturer " << reset
                << blue << ": " << reset << yellow << sys.get_motherboard_manufacturer() << reset;
            lp.push(ss.str());
        }
    }

    // User Info
    {
        lp.push("");
        {
            std::ostringstream ss;
            ss << blue << "#- " << reset << green << "User Info " << reset
                << blue << "------------------------------------------------------#" << reset;
            lp.push(ss.str());
        }

        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "Username                 " << reset
                << blue << ": " << reset << yellow << user.get_username() << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "Computer Name            " << reset
                << blue << ": " << reset << yellow << user.get_computer_name() << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "Domain                   " << reset
                << blue << ": " << reset << cyan << user.get_domain_name() << reset;
            lp.push(ss.str());
        }
    }

    // Performance Info
    {
        lp.push("");
        {
            std::ostringstream ss;
            ss << blue << "#- " << reset << green << "Performance Info " << reset
                << blue << "-----------------------------------------------#" << reset;
            lp.push(ss.str());
        }

        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "System Uptime            " << reset
                << blue << ": " << reset << magenta << perf.get_system_uptime() << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "CPU Usage                " << reset
                << blue << ": " << reset << red << perf.get_cpu_usage_percent() << reset << green << "%" << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "RAM Usage                " << reset
                << blue << ": " << reset << red << perf.get_ram_usage_percent() << reset << green << "%" << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "Disk Usage               " << reset
                << blue << ": " << reset << red << perf.get_disk_usage_percent() << reset << green << "%" << reset;
            lp.push(ss.str());
        }
        {
            std::ostringstream ss;
            ss << blue << "~ " << reset << green << "GPU Usage                " << reset
                << blue << ": " << reset << red << perf.get_gpu_usage_percent() << reset << green << "%" << reset;
            lp.push(ss.str());
        }
    }

    // Audio & Power
    {
        lp.push("");
        ExtraInfo audio;

        // Output devices
        vector<AudioDevice> outputDevices = audio.get_output_devices();
        {
            std::ostringstream ss;
            ss << blue << "#- " << reset << green << "Audio Output " << reset
                << blue << "---------------------------------------------------#" << reset;
            lp.push(ss.str());
        }

        int audio_output_device_count = 0;
        for (const auto& device : outputDevices) {
            audio_output_device_count++;
            ostringstream oss;
            oss << blue << "~ " << reset << yellow << audio_output_device_count << reset << " " << cyan << device.name << reset;
            if (device.isActive) {
                oss << " " << green << "(active)" << reset;
            }
            lp.push(oss.str());
        }

        // Input devices
        vector<AudioDevice> inputDevices = audio.get_input_devices();
        {
            std::ostringstream ss;
            ss << blue << "#- " << reset << green << "Audio Input " << reset
                << blue << "-----------------------------------------------------#" << reset;
            lp.push(ss.str());
        }

        int audio_input_device_count = 0;
        for (const auto& device : inputDevices) {
            audio_input_device_count++;
            ostringstream oss;
            oss << blue << "~ " << reset << yellow << audio_input_device_count << reset << " " << cyan << device.name << reset;
            if (device.isActive) {
                oss << " " << green << "(active)" << reset;
            }
            lp.push(oss.str());
        }

        lp.push("");

        // Power Status
        PowerStatus power = audio.get_power_status();
        {
            std::ostringstream ss;
            ss << blue << "#- " << reset << green << "Power  " << reset
                << blue << "---------------------------------------------------------#" << reset;
            lp.push(ss.str());
        }

        ostringstream ossPower;
        if (!power.hasBattery) {
            ossPower << blue << "[" << reset << yellow << "Wired connection" << reset << blue << "]" << reset;
        }
        else {
            ossPower << blue << "~ " << reset << green << "Battery powered " << reset
                << blue << "(" << reset << red << power.batteryPercent << reset << green << "%)" << reset;

            if (power.isCharging) {
                ossPower << " " << yellow << "(Charging)" << reset;
            }
            else {
                ossPower << " " << white << "(Not Charging)" << reset;
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

