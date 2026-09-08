
# 🍁 BinaryFetch-CLI based fully customizable system information tool 
An advanced Windows system information fetcher written in C++ — featuring self-healing configuration, modular architecture, compact & detailed modes, and extreme customization.

![alt](https://github.com/BinaryFetch-CLI/BinaryFetch/blob/984ddde6b5d583c74f146ba0a71fb131a9e91e29/Previews/00_main.png)
![Alt text](https://github.com/BinaryFetch-CLI/BinaryFetch/blob/5c7beffcc4306b2e3d8ef5ddec5a64cd1549dae8/Previews/03_BinaryFetch_banner.png)
![Alt text](https://github.com/InterCentury/BinaryFetch/blob/main/Visual%20Instructions/Ascii_art_tutorial.png?raw=true)

##  User Customization (Only 2 Files)
you can modify and customize them safely from, `C:\Users\Public\BinaryFetch\`
| File               | Purpose                         |
| ------------------ | ------------------------------- |
| `BinaryArt.txt`    | User ASCII art (fully editable, copy-paste-done !) |
| `BinaryFetch_Config.json` | Module configuration & layout   |


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
```
Single color per line:**     $1⠀⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿  Output: Entire line in red

Multiple colors per line:**  $2⠀⣿⣿⣿⣿⣿⣿$3⣿⣿⣿⣿⣿⣿$1⣿⣿⣿⣿⣿⣿ Output: Green → Yellow → Red

No color (default white or the default text color of your terminal): ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿
Output: Standard white text
```
# BinaryFetch feature lists text preview...you can toggle and customize each module 

![Alt text](https://github.com/InterCentury/BinaryFetch/blob/main/Previews/Timeline%201%20(2).gif?raw=true)





**Features Overview: Compact Modules:**
```
Date/Time: Hour/ Minute/ Second/ Day/ MonthName/ MonthNum/ Year/ WeekNum/ DayName/ LeapYear
OS:          Name/ Build/ Architecture/ Uptime
CPU:         Name/ Cores/ Threads/ Clock
GPU:         Name/ Usage/ VRAM/ Frequency
Display:     Index/ Name/ Resolution/ Scale/ Upscale/ RefreshRate
Memory:      Total/ Free/ UsedPercent
Audio:       InputName/ InputStatus/ OutputName/ OutputStatus
Performance: CPU/ GPU/ RAM/ Disk
User:        Username/ Domain/ UserType
Network:     Name/ Type/ IP
Disk:        DriveLetter/ UsedPercent/ Capacity
```
**Features Overview: Detailed Modules:**
```
Memory:       Total/ Free/ UsedPercent/ ModuleCap/ ModuleType/ ModuleSpeed
Storage:      DriveLetter/ StorageType/ UsedSpace/ TotalSpace/ UsedPercent/ 
              FileSystem/ ExtStatus/ ReadSpeed/ WriteSpeed/ SerialNumber/ 
              PredictedRead/ PredictedWrite
Network:      Name/ Type/ LocalIP/ PublicIP/ Locale/ MAC/ UploadSpeed/ DownloadSpeed
DummyNetwork: Name/ Type/ LocalIP/ ReadSpeed/ WriteSpeed
OS:           Name/ Build/ Architecture/ Kernel/ Uptime/ InstallDate/ Serial
CPU:          Brand/ Utilization/ CurrentSpeed/ BaseSpeed/ Cores/ LogicalProcs/ 
              Sockets/ Virtualization/ L1Cache/ L2Cache/ L3Cache
GPU:          Name/ Memory/ Usage/ Vendor/ Driver/ Temperature/ CoreCount/ PrimaryName/ 
              PrimaryVRAM/ PrimaryFreq
Display:      Index/ Name/ CurrentRes/ RefreshRate/ NativeRes/ AspectRatio/ Scaling/ 
              Upscale/ DSRStatus/ DSRType
BIOS/MB:      BiosVendor/ BiosVersion/ BiosDate/ MotherboardModel/ MotherboardManufacturer
User:         Username/ ComputerName/ Domain
Performance:  Uptime/ CPUUsage/ RAMUsage/ DiskUsage/ GPUUsage
Audio/Power:  OutputDevices/ InputDevices/ PowerStatus/ BatteryPercent/ ChargingStatus
```
**Technical Overview: Classes (27 total)**
```
[AsciiArt.cpp ][ConfigManager.cpp ][LivePrinter.cpp ][OSInfo.cpp ]
[CPUInfo.cpp ][MemoryInfo.cpp ][GPUInfo.cpp ][DetailedGPUInfo.cpp ]
[StorageInfo.cpp ][NetworkInfo.cpp ][UserInfo.cpp ]
[PerformanceInfo.cpp ][DisplayInfo.cpp ][ExtraInfo.cpp ]
[SystemInfo.cpp ][CompactAudio.cpp ][CompactOS.cpp ]
[CompactCPU.cpp ][CompactMemory.cpp ][CompactSystem.cpp ]
[CompactGPU.cpp ][CompactPerformance.cpp ][CompactUser.cpp ]
[CompactNetwork.cpp ][DiskInfo.cpp ][TimeInfo.cpp ][CompactScreen]


```

**Q: What about the Linux version of Binary Fetch?**  
**A:** It's under development.

**Q: Does Binary Fetch share user data?**  
**A:** No. Binary Fetch does not collect or share any user data.  

**Q: Does Binary Fetch run in the background?**  
**A:** No. Binary Fetch is a CLI tool and only runs when the command is executed in the terminal.
