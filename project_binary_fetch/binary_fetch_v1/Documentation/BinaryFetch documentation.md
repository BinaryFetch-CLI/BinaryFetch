## 1️⃣ Operating System Info (OSInfo.h)

```cpp
✅ string GetOSVersion(); --> returns full Windows version  
✅ string GetOSArchitecture(); --> returns OS bit type (32/64-bit)  
✅ string GetOSName(); --> returns Windows edition name  
✅ string get_os_uptime(); --> shows OS uptime since reboot  
✅ string get_os_install_date(); --> returns installation date  
✅ string get_os_serial_number(); --> returns OS serial number  
✅ string get_os_kernel_info(); --> returns OS Kernel info  
```

## 2️⃣ CPU / Processor Info (CPUInfo.h)

```cpp
✅ string get_cpu_info(); --> returns CPU brand and model  
✅ string get_cpu_utilization(); --> returns CPU usage percent  
✅ string get_cpu_speed(); --> returns current CPU speed  
✅ string get_cpu_base_speed(); --> returns base CPU frequency  
✅ string get_cpu_sockets(); --> returns number of sockets  
✅ string get_cpu_cores(); --> returns number of cores  
✅ string get_cpu_logical_processors(); --> returns number of threads  
✅ string get_cpu_virtualization(); --> shows virtualization status  
✅ string get_cpu_l1_cache(); --> returns L1 cache size  
✅ string get_cpu_l2_cache(); --> returns L2 cache size  
✅ string get_cpu_l3_cache(); --> returns L3 cache size  
✅ string get_system_uptime(); --> shows system uptime  
✅ string get_process_count(); --> returns process count  
✅ string get_thread_count(); --> returns thread count  
✅ string get_handle_count(); --> returns handle count  
```

## 3️⃣ RAM / Memory Info (RAMInfo.h)


```cpp
✅ string get_total_memory(); --> returns total RAM in GB  
✅ string get_free_memory(); --> returns free RAM in GB  
✅ string used_ram(); --> returns used memory percentage  
✅ string get_formatted_memory_info(); --> returns detailed RAM info  
```
## 4️⃣ GPU / Graphics Card Info (GPUInfo.h)

```cpp
<---------------- Basic GPU Info Functions (GPUInfo.h) ----------------->
✅string get_gpu_name;              // returns GPU name
✅string get_gpu_vendor;               // returns GPU vendor
✅string get_gpu_memory_total;      // returns total VRAM
✅string get_gpu_driver_version;    // returns driver version
✅float get_gpu_usage;                 // returns GPU usage percent
  float get_gpu_temperature;           // returns GPU temperature
✅int get_gpu_core_count;              // returns GPU core count

// --- Multi-GPU / Extra Features 
<----------------(DetailedGPUInfo.h / DetailedGPUInfo.cpp) ------------->
✅ vector<GPUData> get_all_gpus();     // returns info of all GPUs in the system
✅ GPUData primary_gpu_info();         // returns info of the primary GPU (GPU 0)

//here's the implementation inside main.cpp ------------------------------

        auto primary = detailed_gpu_info.primary_gpu_info();
        cout << "GPU 0 : " << primary.name
            << " @ " << primary.frequency_ghz << " GHz ( "
            << primary.vram_gb << " GIB)" << endl;
//(output) -----> NVIDIA GeForce RTX 4070 SUPER @ 3.10 GHz (11.72 GiB)

        auto all = detailed_gpu_info.get_all_gpus();
        cout << "GPU 0 : " << primary.name
            << " @ " << primary.frequency_ghz << " GHz ( "
            << primary.vram_gb << " GIB)";
       cout << endl << endl;
//(output) -----> NVIDIA GeForce RTX 4070 SUPER @ 3.10 GHz (11.72 GiB)
               // NVIDIA GeForce RTX 4078 SUPER @ 3.70 GHz (16.00 GiB)  
               (could show multiple gpu list differently)     

```
## 5️⃣ Storage Info (StorageInfo.h)

```cpp
✅ string get_disk_list; --> returns all disks  
✅ string get_disk_size; --> returns total disk size  
✅ string get_disk_free_space; --> returns free disk space  
✅ string get_disk_type; --> returns disk type (SSD/HDD)  
✅ string get_disk_serial_number; --> returns disk serial number  
✅ string get_disk_read_speed; --> returns read speed  
✅ string get_disk_write_speed; --> returns write speed  
✅ string predicted_read_speed; --> returns predicted disk read speed
✅ string predicted_write_speed; --> returns predicted write speed 
```
## 6️⃣ Network / IP Info (NetworkInfo.h)

```cpp
✅ string get_ip_address(); --> returns local IP  
✅ string get_mac_address(); --> returns MAC address  
✅ string get_network_name(); --> returns network name  (SSID)
✅ string get_network_speed(); --> returns network speed  
✅ string get_locale(); --> returns system locale  
✅ string get_public_ip(); --> returns public IP  
```
## 7️⃣ System Uptime / Performance (PerformanceInfo.h)

```cpp
✅ string get_system_uptime(); --> returns uptime  
✅ float get_cpu_usage_percent(); --> CPU usage percent  
✅ float get_ram_usage_percent(); --> RAM usage percent  
✅ float get_disk_usage_percent(); --> Disk usage percent  
float get_gpu_usage_percent(); --> GPU usage percent  (paused for now)
```
## 8️⃣ User / System Info (UserInfo.h)

```cpp
✅ sstring get_username(); --> returns username  
✅ sstring get_computer_name(); --> returns PC name  
✅ sstring get_domain_name(); --> returns domain name  
✅ sstring get_user_groups(); --> returns user groups  
```
## 9️⃣ BIOS / Motherboard / Environment (SystemInfo.h)

```cpp
✅ string get_bios_vendor(); --> returns BIOS vendor  
✅ string get_bios_version(); --> returns BIOS version  
✅ string get_bios_date(); --> returns BIOS date  
✅ string get_motherboard_model(); --> returns motherboard model  
✅ string get_motherboard_manufacturer(); --> returns manufacturer  
✅ string get_environment_variables(); --> returns environment vars  
```
## 🔟 DisplayInfo (DisplayInfo.h)

```cpp
✅ string get_screen_resolution(); --> returns screen resolution  
✅ float get_screen_refresh_rate(); --> returns refresh rate  
```
## 🔟 Optional Extras (ExtraInfo.h)

```cpp
/ 🎧 Returns a list of available audio devices.
/ Includes both input (microphones) and output (speakers/headphones).
/ Marks which device is currently active.
/ Example:
/   Headphone (High Definition Audio) (active)
/   Speaker (High Definition Audio)
/   Microphone (High Definition Audio) (active)
/   Microphone (DroidCam)
string get_audio_devices();

/ 🔋 Returns the system's current power status.
/ For desktop PCs → "Power Status: Wired connection"
/ For laptops → "Power Status: Battery powered (85%) (Charging)"
/               or "Power Status: Battery powered (62%) (Not Charging)"
string get_power_status();
```




# 🧬 BinaryFetch Compact Mode — Class Structure

### 🧠 Overview

This document defines the modular class structure for **BinaryFetch Compact Mode**, where each system information category is represented by a separate class.  
All modules are lightweight and independent, designed for clean integration and scalability.
═══════════════ BinaryFetch Compact Mode ═══════════════

[OS] -> Windows 11 10.0 (Build 22631) (64-bit) (uptime: 1h 25m)  
[CPU] -> AMD Ryzen 5 5600G with Radeon Graphics (6C/12T) @ 3.89 GHz  
[Display 1] -> Generic PnP Monitor (2194 x 1234) @60Hz  
[Display 2] -> HP M22f FHD Monitor (2194 x 1234) @60Hz  
[Memory] -> (total: 47.79 GB) (free: 28.44 GB) ( 40.00% )  
[Audio Input] -> Microphone (High Definition Audio Device)(Active)  
[Audio Output] -> Headphones (High Definition Audio Device)(Active)  
[BIOS] -> American Megatrends Inc. 2423 (08/10/2021)  
[Motherboard] -> ASUSTeK COMPUTER INC. TUF GAMING A520M-PLUS WIFI  
[GPU] -> NVIDIA GeForce RTX 4070 SUPER (9%) (11.99 GB) (@2535 MHz)  
[Performance] -> (CPU: 33%) (GPU: 9%) (RAM: 40%) (Disk: 97%)  
[User] -> @coffee~ -> (Domain: InterStudio) -> (Type: Admin)  
[network] -> (Name: Maruf Hasan) (Type: WiFi) (ip: 182.168.96.1)


------------------------------------------------

{
  "modules": {
    "ExtraInfo": {
      "enabled": true,
      "audio_devices": true,
      "power_status": true
    },

    "DisplayInfo": {
      "enabled": true,
      "monitors": true,
      "show_name_&_model":true,
      "show_refresh_rate": true,
      "show_resolution": true
    },

    "SystemInfo": {
      "enabled": true,
      "bios": {
        "vendor": true,
        "version": true,
        "date": true
      },
      "motherboard": {
        "model": true,
        "manufacturer": true
      }
    },

    "UserInfo": {
      "enabled": true,
      "username": true,
      "computer_name": true,
      "domain_name": true,
      "user_groups": true
    },

    "PerformanceInfo": {
      "enabled": true,
      "uptime": true,
      "cpu_usage": true,
      "ram_usage": true,
      "disk_usage": true,
      "gpu_usage": true
    },

    "GPUInfo": {
      "enabled": true,
      "mode": "expanded",
      "fields": {
        "gpu_name": true,
        "gpu_memory": true,
        "gpu_usage": true,
        "gpu_vendor": true,
        "gpu_driver_version": true,
        "gpu_temperature": true,
        "gpu_core_count": true
      }
    },

    "MemoryInfo": {
      "enabled": true,
      "total_memory": true,
      "free_memory": true,
      "used_percentage": true
    },

    "OSInfo": {
      "enabled": true,
      "version": true,
      "architecture": true,
      "name": true,
      "kernel_version": true,
      "uptime": true,
      "install_date": true,
      "serial_number": true
    },

    "CPUInfo": {
      "enabled": true,
      "brand": true,
      "utilization": true,
      "speed": true,
      "base_speed": true,
      "sockets": true,
      "cores": true,
      "logical_processors": true,
      "virtualization": true,
      "cache": {
        "l1": true,
        "l2": true,
        "l3": true
      }
    },

    "StorageInfo": {
      "enabled": true,
      "summary": true,
      "details": true,
      "predicted_performance": true
    },

    "NetworkInfo": {
      "enabled": true,
      "local_ip": true,
      "mac_address": true,
      "locale": true,
      "public_ip": true,
      "ssid": true,
      "network_speed": true
    }
  },

  "CompactInfo": {
    "enabled": true,
    "order": [
      "OSInfo",
      "CPUInfo",
      "MemoryInfo",
      "GPUInfo",
      "DisplayInfo",
      "ExtraInfo",
      "SystemInfo",
      "UserInfo",
      "PerformanceInfo"
    ],
    "fields": {
      "OSInfo": ["name", "version", "architecture", "uptime"],
      "CPUInfo": ["brand", "cores", "threads", "speed", "utilization"],
      "MemoryInfo": ["total", "free", "used_percentage"],
      "GPUInfo": ["gpu_name", "gpu_memory", "gpu_usage"],
      "DisplayInfo": ["resolution", "refresh_rate", "monitor_count"],
      "ExtraInfo": ["active_output", "active_input", "power_status"],
      "SystemInfo": ["bios_vendor", "motherboard_model"],
      "UserInfo": ["username", "computer_name", "domain_name", "user_groups"],
      "PerformanceInfo": ["cpu_usage", "ram_usage", "disk_usage", "gpu_usage", "uptime"]
    },
    "prefix_icons": {
      "OSInfo": "🧠",
      "CPUInfo": "⚙️",
      "MemoryInfo": "💾",
      "GPUInfo": "🎮",
      "DisplayInfo": "🖥",
      "ExtraInfo": "🔊",
      "SystemInfo": "🔧",
      "UserInfo": "👤",
      "PerformanceInfo": "📈"
    }
  },

  "preferences": {
    "color_scheme": "default",
    "highlight_color": "yellow",
    "display_mode": "expanded" 
  }
}
### 🧠 **CompactOS**

**File:** `CompactOS.h / CompactOS.cpp`

```cpp
class CompactOS {
public:
 ✅ std::string getOSName();  
 ✅ std::string getOSBuild();
 ✅ std::string getArchitecture();
 ✅ std::string getUptime();
};
```

---

### ⚙️ **CompactCPU**

**File:** `CompactCPU.h / CompactCPU.cpp`

```cpp
class CompactCPU {
public:
✅  std::string getCPUName();
✅  std::string getCores();
✅  std::string getThreads();
✅  double getClockSpeedGHz();
✅  double getUsagePercent();
};
```

---

### 💿 **CompactMemory**

**File:** `CompactRAM.h / CompactRAM.cpp`

```cpp
class CompactMemory {
public:
 ✅   double get_total_memory();
 ✅   double get_free_memory();
 ✅   double get_used_memory_percent();
};
```

---

### 🎮 **CompactGPU**

**File:** `CompactGPU.h / CompactGPU.cpp`

```cpp
class CompactGPU {
public:
 ✅   std::string getGPUName();
 ✅   double getVRAMGB();
 ✅   double getGPUUsagePercent();
 ✅   string gpu_frequncy();
};
```

---

### 🖥 **CompactDisplay**

**File:** `CompactDisplay.h / CompactDisplay.cpp`

```cpp
class CompactDisplay {
public:
    struct ScreenInfo {
✅  std::string brand;   // monitor model / brand (e.g. "HP M22f")
✅  std::string resolution;   // "1920 x 1080"
✅  int refresh_rate;         // e.g. 60
};

    std::vector<DisplayInfo> getDisplays();
};
```

---

### 🔊 **CompactAudio**

**File:** `CompactAudio.h / CompactAudio.cpp`

```cpp
class CompactAudio {
public:
 ✅   std::string getOutputDevice();
 ✅   std::string getInputDevice();
 ✅   bool isOutputActive();
 ✅   bool isInputActive();
};
```

---

### 🔋 **CompactPower**

**File:** `CompactPower.h / CompactPower.cpp`

```cpp
class CompactPower {
public:
    bool isPluggedIn();
    int getBatteryPercent();
    std::string getPowerStatus();
};
```

---

### 🔧 **CompactSystem**

**File:** `CompactSystem.h / CompactSystem.cpp`

```cpp
class CompactSystem {
public:
 ✅   std::string getBIOSInfo();
 ✅   std::string getMotherboardInfo();
};
```

---

### 👤 **CompactUser**

**File:** `CompactUser.h / CompactUser.cpp`

```cpp
class CompactUser {
public:
 ✅ std::string getUsername();
 ✅ std::string getDomain();
 ✅ bool isAdmin();
};
```


### 🎮 **CompactNetwork**

**File:** `CompactNetwork.h / CompactNetwork.cpp`

```cpp
class CompactNetwork {
public:
✅std::string get_network_name();  // Adapter name or WiFi SSID
✅std::string get_network_type();  // "WiFi" or "Ethernet"
✅std::string get_network_ip();    // Local IPv4 address
};
```

---

### 📊 **CompactPerformance**

**File:** `CompactPerformance.h / CompactPerformance.cpp`

```cpp
class CompactPerformance {
public:
 ✅   double getCPUUsage();
 ✅   double getRAMUsage();
 ✅   double getDiskUsage();
 ✅   double getGPUUsage();
 ✅   std::string getUptime();
};
```

---







