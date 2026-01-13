
![Alt text](123456.png)
![Alt text](https://github.com/InterCentury/BinaryFetch/blob/main/Previews/BinaryFetch_banner2.png?raw=true)
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

### 🧠 Engine-First Design

BinaryFetch guarantees that **the program never crashes due to missing user files**.

If a user:

* deletes `BinaryArt.txt`
* deletes `BinaryFetch_Config.json`
* corrupts configuration
* runs BinaryFetch for the first time

👉 **The engine automatically recreates everything from defaults.**

No prompts.
No errors.
No user intervention.


![Alt text](https://github.com/InterCentury/BinaryFetch/blob/main/Visual%20Instructions/21312.png?raw=true)

## 🗂️ User Customization (Only 2 Files)
you can modify and customize them safely from,

`C:\Users\Public\Local\BinaryFetch\`
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

### 📦 Total Modules

* **17+ core modules**
* **80+ system info functions**
* **Zero hard dependencies between modules**


## 🛡️ Self-Healing Engine (Worst-Case Handling)

BinaryFetch **never breaks** due to user mistakes.

### Automatic Recovery Includes:

* Missing ASCII art file → recreated from bundled default
* Missing JSON config → recreated from default schema
* Deleted directories → recreated silently
* Corrupted config → safely regenerated

> **Main never manages files. The engine does everything.**

---

## 🧪 Engineering Back-Bone

* ✅ First run always succeeds
* ✅ User customization preserved
* ✅ No duplicated logic
* ✅ No orchestration spaghetti
* ✅ Clean single-call API
* ✅ Future-proof modularity


# BinaryFetch feature lists text preview...you can toggle and customize each module 🍀
```
#####################  <<<<<<<<<<<<<<<<<<<<<<  ~>> BinaryFetch_____________________________________________________
#####################  <<<<<<<<<<<<<<<<<<<<<<  📅 (Time: 02:51:20) (Date: 12 : January : 2026) (Week: 2 - Monday)
#####################  <<<<<<<<<<<<<<<<<<<<<<  🚀 OS: Windows 11  10.0 (Build 22631) (64-bit) (uptime: 2h 39m)
#####################  <<<<<<<<<<<<<<<<<<<<<<  🧠 CPU: AMD Ryzen 5 5600G with Radeon Graphics (6C/12T) @ 3.89 GHz
#####################  <<<<<<<<<<<<<<<<<<<<<<  🔥 GPU: NVIDIA GeForce RTX 4070 SUPER (13%) (11.9941 GB) (@2535 MHz)
#####################  <<<<<<<<<<<<<<<<<<<<<<  📺 Display 1: DELL S2240L (3840 x 2160) (Scale: 175%) (@60Hz)
#####################  <<<<<<<<<<<<<<<<<<<<<<  📺 Display 2: HP M22f FHD (3840 x 2160) (Scale: 175%) (@60Hz)
#####################  <<<<<<<<<<<<<<<<<<<<<<  📟 Memory: (total: 47.2937 GB) (free: 27.2848 GB) (42%)
#####################  <<<<<<<<<<<<<<<<<<<<<<  🎙️ Audio Input: Microphone (High Definition Audio Device) [Active]
#####################  <<<<<<<<<<<<<<<<<<<<<<  🎧 Audio Output: Headphones (High Definition Audio Device) [Active]
                                               🔋 Performance: (CPU: 36%) (GPU: 0%) (RAM: 42%) (Disk: 93%)
>>>>>>>>>>>>>>>>>>>>>  ######################  ☕ User: @coffee~ (Domain: MARUF) (Type: Admin)
>>>>>>>>>>>>>>>>>>>>>  ######################  🌐 Network: (Name: Maruf Hasan) (Type: WiFi) (ip: 172.128.56.1)
>>>>>>>>>>>>>>>>>>>>>  ######################  📂 Disk Usage: (C: 93%) (D: 68%) (G: 93%) (H: 71%) (I: 62%) (Y: 26%)
>>>>>>>>>>>>>>>>>>>>>  ######################  📊 Disk Cap: (C-237GB)(D-465GB)(G-112GB)(H-931GB)(I-110GB)(Y-465GB)
>>>>>>>>>>>>>>>>>>>>>  ######################
>>>>>>>>>>>>>>>>>>>>>  ######################  >>~ Memory Info -------------------------*
>>>>>>>>>>>>>>>>>>>>>  ######################  ~ (Total: 48 GB) (Free: 27 GB) (Used: 43%)
>>>>>>>>>>>>>>>>>>>>>  ######################  ~ Memory 0: (Used: 43%) 16GB DDR4 2133 MHz
>>>>>>>>>>>>>>>>>>>>>  ######################  ~ Memory 1: (Used: 43%) 08GB DDR4 2133 MHz
>>>>>>>>>>>>>>>>>>>>>  ######################  ~ Memory 2: (Used: 43%) 16GB DDR4 2133 MHz
>>>>>>>>>>>>>>>>>>>>>  ######################  ~ Memory 3: (Used: 43%) 08GB DDR4 2133 MHz

                                               ------------------------- STORAGE SUMMARY --------------------------
                                               SSD Disk (C:) [ (Used)  221.62 GiB / 237.32 GiB  (93%) - NTFS  Int ]
                                               HDD Disk (D:) [ (Used)  319.82 GiB / 465.76 GiB  (68%) - NTFS  Int ]
                                               USB Disk (G:) [ (Used)  105.26 GiB / 112.64 GiB  (93%) - NTFS  Ext ]
                                               HDD Disk (H:) [ (Used)  664.04 GiB / 931.50 GiB  (71%) - NTFS  Int ]
                                               SSD Disk (I:) [ (Used)   68.81 GiB / 110.79 GiB  (62%) - NTFS  Int ]
                                               HDD Disk (Y:) [ (Used)  122.77 GiB / 465.75 GiB  (26%) - NTFS  Int ]

                                               -------------------- DISK PERFORMANCE & DETAILS --------------------
                                               Disk (C:) [ Read: 1215.87 MB/s | Write:  555.25 MB/s | SN-1000 Int ]
                                               Disk (D:) [ Read:  120.37 MB/s | Write:   77.68 MB/s | SN-1001 Int ]
                                               Disk (G:) [ Read:  147.38 MB/s | Write:   67.79 MB/s | SN-1002 Ext ]
                                               Disk (H:) [ Read:  119.90 MB/s | Write:  123.54 MB/s | SN-1003 Int ]
                                               Disk (I:) [ Read:  506.55 MB/s | Write:  430.21 MB/s | SN-1004 Int ]
                                               Disk (Y:) [ Read:   92.65 MB/s | Write:   84.98 MB/s | SN-1005 Int ]

                                               ---------------- DISK PERFORMANCE & DETAILS (Predicted) ------------
                                               Disk (C:) [ Read:  500.00 MB/s | Write:  450.00 MB/s | SN-1000 Int ]
                                               Disk (D:) [ Read:  140.00 MB/s | Write:  120.00 MB/s | SN-1001 Int ]
                                               Disk (G:) [ Read:  100.00 MB/s | Write:   80.00 MB/s | SN-1002 Ext ]
                                               Disk (H:) [ Read:  140.00 MB/s | Write:  120.00 MB/s | SN-1003 Int ]
                                               Disk (I:) [ Read:  500.00 MB/s | Write:  450.00 MB/s | SN-1004 Int ]
                                               Disk (Y:) [ Read:  140.00 MB/s | Write:  120.00 MB/s | SN-1005 Int ]

                                               #- Network Info ---------------------------------------------------#
                                               ~ Network Name              : InterCentury
                                               ~ Network Type              : WiFi
                                               ~ Local IP                  : 192.---.56.1/--
                                               ~ Public IP:                : 103.---.209.--
                                               ~ Locale                    : en-US
                                               ~ Mac address               : 0A:00:--:00:--:0F
                                               ~ avg upload speed          : 21.4 Mbps
                                               ~ avg download speed        : 14.8 Mbps

                                               #- Operating System -----------------------------------------------#
                                               ~ Name                      : Microsoft Windows 11 Pro
                                               ~ Build                     : Windows 10.0 Build 22631
                                               ~ Architecture              : 64-bit
                                               ~ Kernel                    : WIN32_NT 10.0.22631.4169 (23H2)
                                               ~ Uptime                    : 2 hours, 39 minutes
                                               ~ Install Date              : 2024-09-12
                                               ~ Serial                    : Unknown

                                               #- CPU Info -------------------------------------------------------#
                                               ~ Brand                     : AMD Ryzen 5 5600G with Radeon Graphics
                                               ~ Utilization               : 31.8996%
                                               ~ Speed                     : 3.90 GHz
                                               ~ Base Speed                : 3.90 GHz
                                               ~ Cores                     : 6
                                               ~ Logical Processors        : 12
                                               ~ Sockets                   : 1
                                               ~ Virtualization            : Disabled
                                               ~ L1 Cache                  : 384 KB
                                               ~ L2 Cache                  : 3 MB
                                               ~ L3 Cache                  : 16 MB

                                               #- GPU Info -------------------------------------------------------#
                                               GPU 1
                                               |-> Name                   : NVIDIA GeForce RTX 4070 SUPER
                                               |-> Memory                 : 11.7 GB
                                               |-> Usage                  : 25%
                                               |-> Vendor                 : NVIDIA
                                               |-> Driver Version         : 32.0.15.8097
                                               |-> Temperature            : 38 C
                                               #-> Core Count             : 7168
                                               #-GPU 2------------------------------------------------------------#
                                               |-> Name                   : AMD Radeon(TM) Graphics
                                               |-> Memory                 : 0.5 GB
                                               |-> Usage                  : 0%
                                               |-> Vendor                 : AMD
                                               |-> Driver Version         : 31.0.21923.11000
                                               |-> Temperature            : -1 C
                                               #-> Core Count             : 7168
                                               #-GPU 3------------------------------------------------------------#
                                               |-> Name                   : Microsoft Basic Render Driver
                                               |-> Memory                 : 0.0 GB
                                               |-> Usage                  : 0%
                                               |-> Vendor                 : Unknown
                                               |-> Driver Version         : 10.0.22621.3672
                                               |-> Temperature            : -1 C
                                               #-> Core Count             : 7168

                                               #- Primary GPU Details---------------------------------------------#
                                               |-> Name                   : NVIDIA GeForce RTX 4070 SUPER
                                               |-> VRAM                   : 11.7178 GiB
                                               #-> Frequency              : 2.535 GHz

                                               #- Display 1 ------------------------------------------------------#
                                               |-> Name                   : DELL S2240L
                                               |-> Applied Resolution     : 3840x2160 @60Hz
                                               |-> Native Resolution      : 1920x1080
                                               |-> Aspect Ratio           : 16:9
                                               |-> Scaling                : 175%
                                               |-> Upscale                : 2x
                                               |-> DSR / VSR              : Enabled (DSR)

                                               #- Display 2 ------------------------------------------------------#
                                               |-> Name                   : HP M22f FHD
                                               |-> Applied Resolution     : 3840x2160 @60Hz
                                               |-> Native Resolution      : 1920x1080
                                               |-> Aspect Ratio           : 16:9
                                               |-> Scaling                : 175%
                                               |-> Upscale                : 2x
                                               |-> DSR / VSR              : Enabled (DSR)


                                               #- BIOS & Motherboard Info ----------------------------------------#
                                               ~ Bios Vendor               : American Megatrends Inc.
                                               ~ Bios Version              : 2423
                                               ~ Bios Date                 : 08/10/2021
                                               ~ Motherboard Model         : TUF GAMING A520M-PLUS WIFI
                                               ~ Motherboard Manufacturer  : ASUSTeK COMPUTER INC.

                                               #- User Info ------------------------------------------------------#
                                               ~ Username                  : MarufHasan~
                                               ~ Computer Name             : InterCentury
                                               ~ Domain                    : WORKGROUP

                                               #- Performance Info -----------------------------------------------#
                                               ~ System Uptime            : 2h 39m 22s
                                               ~ CPU Usage                : 37.9507%
                                               ~ RAM Usage                : 42%
                                               ~ Disk Usage               : 93.3852%
                                               ~ GPU Usage                : 1%

                                               #- Audio Output ---------------------------------------------------#
                                               ~ 1 Headphones (High Definition Audio Device) (active)
                                               ~ 2 CABLE Input (VB-Audio Virtual Cable)
                                               ~ 3 Speakers (High Definition Audio Device)
                                               ~ 4 DELL S2240L (NVIDIA High Definition Audio)
                                               ~ 5 Virtual Speakers (Virtual Speakers for AudioRelay)
                                               ~ 6 Internal AUX Jack (DroidCam Audio)
                                               ~ 7 Digital Audio (S/PDIF) (High Definition Audio Device)
                                               ~ 8 CABLE In 16ch (VB-Audio Virtual Cable)
                                               #- Audio Input ----------------------------------------------------#
                                               ~ 1 Microphone (DroidCam Audio)
                                               ~ 2 Virtual Mic (Virtual Mic for AudioRelay)
                                               ~ 3 CABLE Output (VB-Audio Virtual Cable)
                                               ~ 4 Microphone (High Definition Audio Device)
                                               ~ 5 Microphone (High Definition Audio Device) (active)
                                               ~ 6 MIDI (DroidCam Audio)

                                               #- Power  ---------------------------------------------------------#
                                               [Wired connection]

```
