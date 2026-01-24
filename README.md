
![alt](https://github.com/InterCentury/BinaryFetch/blob/main/Previews/Info_preview.png?raw=true)
![Alt text](https://github.com/InterCentury/BinaryFetch/blob/main/Previews/BinaryFetch_banner_v3.png?raw=true)
# 🍁 BinaryFetch-CLI based fully customizable system information tool 
An advanced Windows system information fetcher written in C++ — featuring self-healing configuration, modular architecture, compact & detailed modes, and extreme customization.

Built for Windows. Built to last. Built to be shared.


## ✨ Key Philosophy

> **Simplicity is the priority.**

* Two user-editable files
* Self-healing engine
* Modular, scalable design
* Clean separation between *data*, *logic*, and *presentation*




## 🚀 What Makes BinaryFetch Different?


BinaryFetch guarantees that **the program never crashes due to missing user files**.

If a user:

* deletes `BinaryArt.txt`
* deletes `BinaryFetch_Config.json`
* runs BinaryFetch for the first time

👉 **The engine automatically recreates everything from defaults.**



![Alt text](https://github.com/InterCentury/BinaryFetch/blob/main/Visual%20Instructions/Ascii_art_tutorial.png?raw=true)

## 🗂️ User Customization (Only 2 Files)
you can modify and customize them safely from,

`C:\Users\Public\BinaryFetch\`
| File               | Purpose                         |
| ------------------ | ------------------------------- |
| `BinaryArt.txt`    | User ASCII art (fully editable, copy-paste-done !) |
| `BinaryFetch_Config.json` | Module configuration & layout   |

That’s it.

No profiles.
No databases.
No complex formats.
## Color Code System

### And also you can customize each character's Color of your `BinaryArt.txt`

Use `$n` in your `BinaryArt.txt` file where `n` is the color number:

| Code | Color | ANSI Code | Code | Color | ANSI Code |
|------|-------|-----------|------|-------|-----------|
| `$1` | Red | `\033[31m` | `$8` | Bright Red | `\033[91m` |
| `$2` | Green | `\033[32m` | `$9` | Bright Green | `\033[92m` |
| `$3` | Yellow | `\033[33m` | `$10` | Bright Yellow | `\033[93m` |
| `$4` | Blue | `\033[34m` | `$11` | Bright Blue | `\033[94m` |
| `$5` | Magenta | `\033[35m` | `$12` | Bright Magenta | `\033[95m` |
| `$6` | Cyan | `\033[36m` | `$13` | Bright Cyan | `\033[96m` |
| `$7` | White | `\033[37m` | `$14` | Bright White | `\033[97m` |
|      |       |           | `$15` | Reset | `\033[0m` |

### Color Code Examples

**Single color per line:**
```
$1⠀⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿
```
Output: Entire line in red

**Multiple colors per line:**
```
$2⠀⣿⣿⣿⣿⣿⣿$3⣿⣿⣿⣿⣿⣿$1⣿⣿⣿⣿⣿⣿
```
Output: Green → Yellow → Red

**No color (default white):**
```
⠀⠀⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿
```
Output: Standard white text
---

## 🧩 Core Features Overview

* Modular system info engine
* Compact mode & expanded mode
* Multi-GPU support
* Multi-monitor support
* Audio input/output detection
* Power & battery detection
* Predictive disk performance 
* Clean ASCII rendering pipeline
* Community-shareable profiles (just JSON + ASCII)

---

## 🧱 Project Architecture

BinaryFetch follows a **module-per-domain** architecture.

Each system category is implemented as an **independent class** with no side effects.

**TOTAL MODULES & SUB-MODULES:**

## Modules (Classes): 34
## Sub-Modules (Functions): 158

**Breakdown:**
- **Core Modules:** 12 (compact mode)
- **Detailed Modules:** 12 (expanded info)
- **Utility Modules:** 10 (support/helper classes)
**Each module has 1-11 sub-modules for specific data retrieval.**

## 🛡️ Self-Healing file management system (Worst-Case Handling)

BinaryFetch **never breaks** due to user mistakes. (expect json syntax)

### Automatic Recovery Includes:

* Missing ASCII art file → recreated from bundled default
* Missing JSON config → recreated from default schema
* Deleted directories → recreated silently
* Corrupted config → safely regenerated



---

## 🧪 Engineering Back-Bone

* ✅ First run always succeeds
* ✅ User customization preserved
* ✅ No duplicated logic
* ✅ No orchestration spaghetti
* ✅ Clean single-call API
* ✅ Future-proof modularity


# BinaryFetch feature lists text preview...you can toggle and customize each module 🍀



![Alt text](https://github.com/InterCentury/BinaryFetch/blob/main/Previews/Timeline%201%20(2).gif?raw=true)



# **BInaryFetch Feature Overview**

## **📊 Core Modules: 12**
1. **Header Module** - System banner/logo display
2. **Compact Time Module** - Real-time clock display
3. **Compact OS Module** - OS summary
4. **Compact CPU Module** - Processor overview
5. **Compact GPU Module** - Graphics card overview
6. **Compact Screen Module** - Display information
7. **Compact Memory Module** - RAM usage summary
8. **Compact Audio Module** - Audio device status
9. **Compact Performance Module** - System performance metrics
10. **Compact User Module** - User session info
11. **Compact Network Module** - Network connection status
12. **Compact Disk Module** - Storage usage summary

## **🔍 Detailed/Expanded Modules: 12**
1. **Detailed Memory Module** - In-depth RAM analysis
2. **Detailed Storage Module** - Comprehensive disk info
3. **Network Info Module** - Complete network details
4. **Dummy Network Info Module** - Fallback network display
5. **OS Info Module** - Operating system details
6. **CPU Info Module** - Processor specifications
7. **GPU Info Module** - Graphics card details
8. **Display Info Module** - Monitor/display configuration
9. **BIOS/Motherboard Info Module** - Firmware and hardware details
10. **User Info Module** - User account information
11. **Performance Info Module** - System performance metrics
12. **Audio & Power Info Module** - Audio devices and power status

## **🎨 Configuration Features:**
- **Color Customization**: 14+ color options per module
- **Toggle Controls**: Individual enable/disable for each module
- **Granular Display**: Control over every element's visibility
- **Emoji Support**: Optional emoji indicators
- **Compact/Detailed Views**: Multiple display modes
- **Theme System**: Consistent color scheme across all modules

### **📈 Statistics:**
- **Total Configurable Options**: 300+ individual settings
- **Color Properties**: 150+ color configuration points
- **Toggle Switches**: 80+ enable/disable controls
- **Sub-section Controls**: 60+ detailed display options
- **Customizable Elements**: 200+ individually configurable components

## **🎯 Key Capabilities:**
- **Snapshot form realtime data**: e.g. Disk information inlcuding read\write\used
- **Hardware Profiling**: Detailed component specifications
- **Performance Tracking**: CPU/GPU/RAM/Disk utilization
- **Network Analysis**: Connection and speed monitoring
- **Storage Analysis**: Disk usage and performance metrics
- **Display Configuration**: Monitor settings and resolutions
- **Audio Monitoring**: Input/output device status
- **User Session Info**: Account and computer details
- **System Uptime**: Runtime tracking
- **Power Management**: Battery status monitoring

## **🛠️ Technical Features:**
- **Modular Design**: Independent module enable/disable
- **Fallback Systems**: Dummy/backup modules like dummy network 
- **Detailed/Compact Toggle**: Two viewing modes for each module type
- **Consistent Formatting**: Uniform display structure
- **Extensible Architecture**: Easy addition of new modules
- **Cross-Platform Ready**: Windows-focused but adaptable

## **🌈 Visual Features:**
- **Colored Brackets/Parentheses**: Syntax highlighting
- **Separator Lines**: Section dividers
- **Label/Value Distinction**: Clear text differentiation
- **Emoji Integration**: Visual indicators
- **Header Banners**: Module identification
- **Progress/Status Indicators**: Visual performance representation

**Total Configurable Components**: 400+ individual display elements across 24 main modules with extensive customization options for appearance, content, and functionality.



---

## **TOTAL STATISTICS:**
- **Total Classes:** 34
- **Total Functions:** 158
- **Total Objects Instantiated:** 21
- **Total Helper Lambda Functions:** 5
- **Total Structures:** 8

## **CLASS-BY-CLASS BREAKDOWN:**

### **1. AsciiArt**
- **Functions:** 1
- Functions:
  - `loadFromFile()` - Returns `bool` (success/failure)

### **2. LivePrinter**
- **Functions:** 2
- Functions:
  - `push(std::string)` - Returns `void`
  - `finish()` - Returns `void`

### **3. OSInfo**
- **Functions:** 7
- Functions:
  - `GetOSName()` - Returns `std::string` (OS name)
  - `GetOSVersion()` - Returns `std::string` (OS version)
  - `GetOSArchitecture()` - Returns `std::string` (architecture)
  - `get_os_kernel_info()` - Returns `std::string` (kernel info)
  - `get_os_uptime()` - Returns `std::string` (uptime)
  - `get_os_install_date()` - Returns `std::string` (install date)
  - `get_os_serial_number()` - Returns `std::string` (serial number)

### **4. CPUInfo**
- **Functions:** 11
- Functions:
  - `get_cpu_info()` - Returns `std::string` (CPU model)
  - `get_cpu_utilization()` - Returns `std::string` (usage percentage)
  - `get_cpu_speed()` - Returns `std::string` (current speed)
  - `get_cpu_base_speed()` - Returns `std::string` (base speed)
  - `get_cpu_cores()` - Returns `std::string` (core count)
  - `get_cpu_logical_processors()` - Returns `std::string` (logical processors)
  - `get_cpu_sockets()` - Returns `std::string` (socket count)
  - `get_cpu_virtualization()` - Returns `std::string` (virtualization status)
  - `get_cpu_l1_cache()` - Returns `std::string` (L1 cache size)
  - `get_cpu_l2_cache()` - Returns `std::string` (L2 cache size)
  - `get_cpu_l3_cache()` - Returns `std::string` (L3 cache size)

### **5. MemoryInfo**
- **Functions:** 4
- Functions:
  - `getTotal()` - Returns `std::string` (total RAM in GB)
  - `getFree()` - Returns `std::string` (free RAM in GB)
  - `getUsedPercentage()` - Returns `std::string` (usage percentage)
  - `getModules()` - Returns `std::vector<memory_module>` (RAM modules)

### **6. GPUInfo**
- **Functions:** 1
- Functions:
  - `get_all_gpu_info()` - Returns `std::vector<GPUData>` (all GPU info)

### **7. DetailedGPUInfo**
- **Functions:** 1
- Functions:
  - `primary_gpu_info()` - Returns `GPUDetailedData` (detailed GPU info)

### **8. StorageInfo**
- **Functions:** 1
- Functions:
  - `process_storage_info(callback)` - Returns `void` (processes via callback)

### **9. NetworkInfo**
- **Functions:** 7
- Functions:
  - `get_network_name()` - Returns `std::string` (network name)
  - `get_local_ip()` - Returns `std::string` (local IP)
  - `get_public_ip()` - Returns `std::string` (public IP)
  - `get_locale()` - Returns `std::string` (locale)
  - `get_mac_address()` - Returns `std::string` (MAC address)
  - `get_network_upload_speed()` - Returns `std::string` (upload speed)
  - `get_network_download_speed()` - Returns `std::string` (download speed)

### **10. UserInfo**
- **Functions:** 3
- Functions:
  - `get_username()` - Returns `std::string` (username)
  - `get_computer_name()` - Returns `std::string` (computer name)
  - `get_domain_name()` - Returns `std::string` (domain name)

### **11. PerformanceInfo**
- **Functions:** 5
- Functions:
  - `get_system_uptime()` - Returns `std::string` (system uptime)
  - `get_cpu_usage_percent()` - Returns `std::string` (CPU usage %)
  - `get_ram_usage_percent()` - Returns `std::string` (RAM usage %)
  - `get_disk_usage_percent()` - Returns `std::string` (disk usage %)
  - `get_gpu_usage_percent()` - Returns `std::string` (GPU usage %)

### **12. DisplayInfo**
- **Functions:** 1
- Functions:
  - `getScreens()` - Returns `std::vector<ScreenInfo>` (screen info)

### **13. ExtraInfo**
- **Functions:** 3
- Functions:
  - `get_output_devices()` - Returns `std::vector<AudioDevice>` (audio outputs)
  - `get_input_devices()` - Returns `std::vector<AudioDevice>` (audio inputs)
  - `get_power_status()` - Returns `PowerStatus` (power/battery info)

### **14. SystemInfo**
- **Functions:** 5
- Functions:
  - `get_bios_vendor()` - Returns `std::string` (BIOS vendor)
  - `get_bios_version()` - Returns `std::string` (BIOS version)
  - `get_bios_date()` - Returns `std::string` (BIOS date)
  - `get_motherboard_model()` - Returns `std::string` (motherboard model)
  - `get_motherboard_manufacturer()` - Returns `std::string` (manufacturer)

### **15. TimeInfo**
- **Functions:** 10
- Functions:
  - `getHour()` - Returns `int` (hour 0-23)
  - `getMinute()` - Returns `int` (minute 0-59)
  - `getSecond()` - Returns `int` (second 0-59)
  - `getDay()` - Returns `int` (day 1-31)
  - `getMonthName()` - Returns `std::string` (month name)
  - `getMonthNumber()` - Returns `int` (month 1-12)
  - `getYearNumber()` - Returns `int` (year)
  - `getWeekNumber()` - Returns `int` (week number)
  - `getDayName()` - Returns `std::string` (day name)
  - `getLeapYear()` - Returns `std::string` (leap year status)

### **16. CompactAudio**
- **Functions:** 4
- Functions:
  - `active_audio_input()` - Returns `std::string` (input device)
  - `active_audio_input_status()` - Returns `std::string` (input status)
  - `active_audio_output()` - Returns `std::string` (output device)
  - `active_audio_output_status()` - Returns `std::string` (output status)

### **17. CompactOS**
- **Functions:** 4
- Functions:
  - `getOSName()` - Returns `std::string` (OS name)
  - `getOSBuild()` - Returns `std::string` (OS build)
  - `getArchitecture()` - Returns `std::string` (architecture)
  - `getUptime()` - Returns `std::string` (uptime)

### **18. CompactCPU**
- **Functions:** 4
- Functions:
  - `getCPUName()` - Returns `std::string` (CPU name)
  - `getCPUCores()` - Returns `std::string` (core count)
  - `getCPUThreads()` - Returns `std::string` (thread count)
  - `getClockSpeed()` - Returns `std::string` (clock speed in GHz)

### **19. CompactMemory**
- **Functions:** 3
- Functions:
  - `get_total_memory()` - Returns `std::string` (total RAM in GB)
  - `get_free_memory()` - Returns `std::string` (free RAM in GB)
  - `get_used_memory_percent()` - Returns `std::string` (RAM usage %)

### **20. CompactScreen**
- **Functions:** 1
- Functions:
  - `getScreens()` - Returns `std::vector<CompactScreenInfo>` (screen info)

### **21. CompactSystem**
- **Functions:** 0 (declared but not used in current code)

### **22. CompactGPU**
- **Functions:** 4
- Functions:
  - `getGPUName()` - Returns `std::string` (GPU name)
  - `getGPUUsagePercent()` - Returns `std::string` (GPU usage %)
  - `getVRAMGB()` - Returns `std::string` (VRAM in GB)
  - `getGPUFrequency()` - Returns `std::string` (GPU frequency)

### **23. CompactPerformance**
- **Functions:** 4
- Functions:
  - `getCPUUsage()` - Returns `std::string` (CPU usage %)
  - `getGPUUsage()` - Returns `std::string` (GPU usage %)
  - `getRAMUsage()` - Returns `std::string` (RAM usage %)
  - `getDiskUsage()` - Returns `std::string` (disk usage %)

### **24. CompactUser**
- **Functions:** 3
- Functions:
  - `getUsername()` - Returns `std::string` (username)
  - `getDomain()` - Returns `std::string` (domain)
  - `isAdmin()` - Returns `std::string` (admin status)

### **25. CompactNetwork**
- **Functions:** 3
- Functions:
  - `get_network_name()` - Returns `std::string` (network name)
  - `get_network_type()` - Returns `std::string` (network type)
  - `get_network_ip()` - Returns `std::string` (IP address)

### **26. DiskInfo**
- **Functions:** 2
- Functions:
  - `getAllDiskUsage()` - Returns `std::map<std::string, double>` (drive:usage%)
  - `getDiskCapacity()` - Returns `std::map<std::string, double>` (drive:capacity GB)

### **27. DetailedScreen** (header included, object not created)
- **Functions:** Not instantiated in main

### **28-34. Other Included Headers** (objects not instantiated)
- These headers are included but their classes aren't instantiated in main:
  - CompactSystem
  - DetailedScreen
  - Various other headers in includes

## **DATA STRUCTURES:**

### **1. memory_module** (MemoryInfo)
- **Fields:** capacity, type, speed

### **2. GPUData** (GPUInfo)
- **Fields:** gpu_name, gpu_memory, gpu_usage, gpu_vendor, gpu_driver_version, gpu_temperature, gpu_core_count

### **3. GPUDetailedData** (DetailedGPUInfo)
- **Fields:** name, vram_gb, frequency_ghz

### **4. storage_data** (StorageInfo)
- **Fields:** drive_letter, total_space, used_space, used_percentage, file_system, is_external, serial_number, read_speed, write_speed, predicted_read_speed, predicted_write_speed, storage_type

### **5. ScreenInfo** (DisplayInfo)
- **Fields:** name, current_width, current_height, native_resolution, refresh_rate, aspect_ratio, scale_percent, upscale, dsr_enabled, dsr_type

### **6. AudioDevice** (ExtraInfo)
- **Fields:** name, isActive

### **7. PowerStatus** (ExtraInfo)
- **Fields:** hasBattery, batteryPercent, isCharging

### **8. CompactScreenInfo** (CompactScreen)
- **Fields:** name, native_width, native_height, scale_percent, upscale, refresh_rate

## **HELPER LAMBDA FUNCTIONS:**

### **1. getColor()**
- Returns: `std::string` (ANSI color code)
- Parameters: section, key, defaultColor

### **2. isEnabled()**
- Returns: `bool` (section enabled status)
- Parameters: section

### **3. isSubEnabled()**
- Returns: `bool` (subsection enabled status)
- Parameters: section, key

### **4. isSectionEnabled()**
- Returns: `bool` (nested section enabled status)
- Parameters: module, section

### **5. isNestedEnabled()**
- Returns: `bool` (deep nested config enabled status)
- Parameters: module, section, key

## **RETURN TYPE SUMMARY:**
- **String Returns:** 121 functions (76.6%)
- **Vector Returns:** 6 functions (3.8%)
- **Map Returns:** 2 functions (1.3%)
- **Structure Returns:** 8 functions (5.1%)
- **Boolean Returns:** 1 function (0.6%)
- **Void Returns:** 6 functions (3.8%)
- **Integer Returns:** 10 functions (6.3%)
- **Callback-based:** 1 function (0.6%)

## **KEY OBSERVATIONS:**
1. **String Dominance:** Most functions (76.6%) return strings for easy display formatting
2. **Modular Design:** Each class has a specific, focused responsibility
3. **Data Structures:** Well-defined structures for complex data types
4. **Configuration Integration:** All output is JSON-configurable via helper functions
5. **Separation of Concerns:** Compact vs Detailed modules for different display needs




# Q: What about the linux version of Binary Fetch ?
# A: It's under develpement.
# Q: will Linux users also get as much featurs as windows version ?
# A: Of course, even more.
# Q: Does Binary Fetch Share user data ?
# A: Straight No.
# Q: Does Binary Fetch run in the background ?
# A: Since it's a CLI tool, it only runs whenever user type the command on terminal.

