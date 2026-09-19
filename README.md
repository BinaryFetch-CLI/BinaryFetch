# BinaryFetch-CLI based fully customizable system information tool

An advanced Windows system information fetcher written in C++ — featuring self-healing configuration, modular architecture, compact & detailed modes, and extreme customization.

![alt](https://github.com/BinaryFetch-CLI/BinaryFetch/blob/984ddde6b5d583c74f146ba0a71fb131a9e91e29/Previews/00_main.png)
![Alt text](https://github.com/BinaryFetch-CLI/BinaryFetch/blob/5c7beffcc4306b2e3d8ef5ddec5a64cd1549dae8/Previews/03_BinaryFetch_banner.png)
![Alt text](https://github.com/InterCentury/BinaryFetch/blob/main/Visual%20Instructions/Ascii_art_tutorial.png?raw=true)

## User Customization (Only 2 Files)

You can modify and customize them safely from `C:\Users\Public\BinaryFetch\`

| File | Purpose |
| --- | --- |
| `BinaryArt.txt` | User ASCII art (fully editable, copy-paste-done) |
| `BinaryFetch_Config.jsonc` | Module configuration & layout (legacy `.json` still supported) |

BinaryFetch 1.6 supports both `.jsonc` and `.json` config files side by side. The `.jsonc` format is preferred going forward because it allows comments. If only a legacy `.json` exists, it is loaded as-is. If neither exists, BinaryFetch self-heals by extracting its embedded default config as `.jsonc`.

Important: BinaryFetch is receiving continuous updates, including architectural updates. Your old BinaryFetch config no longer works on v1.6. To make BinaryFetch 1.6 work properly, please delete this folder:

```
C:\Users\Public\BinaryFetch
```

BinaryFetch will recreate a fresh default config on the next launch. Because BinaryFetch is receiving continuous feature and architecture updates, your current config may no longer work in future versions either.

### And also you can customize each character's Color of your `BinaryArt.txt`

Use `$n` in your `BinaryArt.txt` file where `n` is the color number:

| Code | Color | ANSI Code | Code | Color | ANSI Code |
| --- | --- | --- | --- | --- | --- |
| `$1` | Red | `\033[31m` | `$8` | Bright Red | `\033[91m` |
| `$2` | Green | `\033[32m` | `$9` | Bright Green | `\033[92m` |
| `$3` | Yellow | `\033[33m` | `$10` | Bright Yellow | `\033[93m` |
| `$4` | Blue | `\033[34m` | `$11` | Bright Blue | `\033[94m` |
| `$5` | Magenta | `\033[35m` | `$12` | Bright Magenta | `\033[95m` |
| `$6` | Cyan | `\033[36m` | `$13` | Bright Cyan | `\033[96m` |
| `$7` | White | `\033[37m` | `$14` | Bright White | `\033[97m` |
|  |  |  | `$15` | Reset | `\033[0m` |

### Color Code Examples

```
Single color per line:**     $1⠀⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿  Output: Entire line in red

Multiple colors per line:**  $2⠀⣿⣿⣿⣿⣿⣿$3⣿⣿⣿⣿⣿⣿$1⣿⣿⣿⣿⣿⣿ Output: Green → Yellow → Red

No color (default white or the default text color of your terminal): ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿
Output: Standard white text
```

## New in BinaryFetch 1.6

### 1. Fully Custom Color Selection from Config

Every color used by BinaryFetch now comes from the top-level `colors` object in the config. No color name is hardcoded in the application logic anymore. You can define any color name you want, with no limit.

Three color formats are supported:

| Format | Example | Notes |
| --- | --- | --- |
| Hex | `"#RRGGBB"` | Converted to 24-bit truecolor |
| Plain RGB | `"R,G,B"` | Converted to 24-bit truecolor |
| Raw ANSI escape | `"\u001b[38;2;R;G;Bm"` | Used exactly as written |
| Special reset | `"RESET"` | Always becomes the terminal reset code |

Example from the default config:

```jsonc
"colors": {
  "red":            "#F2726B",
  "green":          "#3ECF8E",
  "yellow":         "#F2C063",
  "blue":           "#60A5FA",
  "magenta":        "#C084FC",
  "cyan":           "#5EEAD4",
  "white":          "#E5E7EB",

  "bright_red":     "#F8A39D",
  "bright_green":   "#7EE2B8",
  "bright_yellow":  "#F6D68E",
  "bright_blue":    "#93C5FD",
  "bright_magenta": "#D8B4FE",
  "bright_cyan":    "#99F6E4",
  "bright_white":   "#F9FAFB",

  "reset":          "RESET",

  "purple":         "#A78BFA",
  "amber":          "#F2C063",
  "orange":         "#F2924B",
  "muted":          "#7D8491",
  "muted_2":        "107, 119, 122",
  "fg_dim":         "#C9CDD3"
}
```

How resolution works:

- Invalid color values are skipped. In debug builds, a warning is printed for each invalid entry.
- If the `colors` section is absent, BinaryFetch still runs. Every lookup degrades to plain white through a single hardcoded safety net.

### 2. JSON and JSONC Config Support

BinaryFetch 1.6 supports two config extensions side by side:

- `.jsonc` — preferred going forward, parsed with comments allowed.
- `.json` — legacy, still parsed exactly as before. Comments are now allowed here too, but a comment-free file behaves identically either way, so nothing already deployed breaks.

Resolution order, checked fresh on every launch:

1. Both `.jsonc` and `.json` exist — `.jsonc` wins.
2. Only `.jsonc` exists — load it.
3. Only `.json` exists — load it as-is. BinaryFetch never silently creates a `.jsonc` next to it. An existing legacy install stays on `.json` until the user removes that file themselves.
4. Neither exists — self-heal from the embedded EXE resource. This is the only branch that ever creates a new file, and it always writes `.jsonc`.

So if you delete your `.json` later with no `.jsonc` present, BinaryFetch will recreate a fresh `.jsonc` default on the next run. Nothing ever overwrites a config file that already exists.

<img width="1081" height="834" alt="BinaryFetch_preview_xi" src="https://github.com/user-attachments/assets/01bf0393-ab1f-4b62-9986-12272e268b27" />

### 3. Sixel Image Support for Windows Terminal

BinaryFetch 1.6 adds image support through the `art` section. This is intended for terminals that support Sixel graphics, such as Windows Terminal.

Example:

```jsonc
"art": {
  "Ascii_Art": {
    "enabled": true,
    "padding_up": 0,
    "padding_left": 0,
    "padding_right": 0
  },
  "Image": {
    "enabled": false,
// Windows paths need DOUBLE backslashes ("\\") because a single
// backslash is a JSON escape character. Forward slashes ("/")
// "G:/screenshot/sky.jpg"     -> valid
// "G:\\screenshot\\sky.jpg"   -> valid
// "G:\screenshot\sky.jpg"     -> invalid
    "image_path": "G:\\screenshot\\hisky.jpg",
    "image_size_percentage": 47,
    "padding_up": 1,
    "padding_left": 0,
    "padding_right": 1
  }
}
```



You can toggle ASCII art and images independently. Padding is configurable on all sides.

### 4. New Modern Default Theme — Bitsmooth Lite

BinaryFetch 1.6 ships with a new default theme called Bitsmooth Lite. It is a softened, pastel version of the base palette, designed to be easier on the eyes during long terminal sessions.

The theme defines:

- Base colors: red, green, yellow, blue, magenta, cyan, white.
- Bright variants: bright_red, bright_green, bright_yellow, bright_blue, bright_magenta, bright_cyan, bright_white.
- Terminal-panel colors: purple, amber, orange, muted, muted_2, fg_dim.
- Reset: special `RESET` value.

All values are truecolor hex codes or plain RGB. You can replace any of them with your own values. Because every module reads colors through the config manager, changing the theme is a config-only operation.

### 5. Adjust Section and Element Order

BinaryFetch 1.6 introduces full ordering control. You can adjust the order of each core module, both compact and detailed. Everything is under your control.

The key is `section_order` at the top level of the config. It is an array of section names.

Example:

```jsonc
"section_order": [
  "header_settings",
  "compact_date_and_time",
  "compact_operating_system",
  "compact_processor",
  "compact_graphics_card",
  "compact_display_monitor",
  "compact_system_memory",
  "compact_audio_devices",
  "compact_resource_usage",
  "compact_user_account",
  "compact_network_connection",
  "compact_disk_storage",
  "detailed_resource_usage",
  //"detailed_system_memory",
  "detailed_disk_storage"
  //"detailed_operating_system",
  //"detailed_processor",
  //"detailed_graphics_card",
  //"detailed_display_monitor",
  //"detailed_bios_and_motherboard",
  //"detailed_user_account",
  //"detailed_network_connection",
  //"detailed_audio_and_power"
]
```

How to use it:

- Reorder entries to change the order modules appear.
- Comment out an entry to disable that whole section.
- Make sure to keep commas correct when commenting entries in or out.
- Non-string entries are silently skipped.
- If the array is empty or missing, BinaryFetch won't show the section.



### 6. UTF-8 Support and Emoji Control

BinaryFetch now supports UTF-8 format. This means you can use emoji everywhere in labels, prefixes, and strings, unless your terminal does not support it.

There is a global `emoji` section:

```jsonc
"emoji": {
  "enabled": false,
  "style": "color"
}
```

- `enabled` — when `false`, emoji-eligible glyphs and their variation selectors are removed entirely.
- `style` — accepts `"auto"`, `"color"`, or `"text"`.
  - `"auto"` — pure no-op. The original string is returned untouched. This is what every config written before this feature existed will hit.
  - `"text"` — appends U+FE0E, the text presentation selector, to eligible glyphs.
  - `"color"` — appends U+FE0F, the emoji presentation selector, to eligible glyphs.

Note: the comment in the default config mentions `"mono"` for a guaranteed flat look, but the 1.6 validation only accepts `"auto"`, `"color"`, and `"text"`. Any other value falls back to `"auto"` and prints a warning in debug builds.

Malformed or truncated UTF-8 sequences fall back to treating the single byte as-is, so a stray byte never corrupts or crashes the rest of the string.

## Config Schema Reference

The config may look big and messy at first glance, but it is fully predictable. Every sub-module, whether a field or a sub-value inside a group, is built from exactly two parts:

1. `label`
2. `value`

Each of those always follows the same shape:

```jsonc
label: { prefix, prefix_color, text, color, suffix, suffix_color }
value: { prefix, prefix_color, color, suffix, suffix_color }
```

Some keys are left out where they have no use case. An omitted key simply means empty or no effect. Nothing breaks.

Section-level toggles:

- `enabled` — toggles a whole section.
- `sections` — toggles named subsections inside a module.
- `labels` — overrides label text.
- `prefixes` — overrides prefix text.
- `colors` — overrides colors inside a section.

Alias fallbacks are built in for common keys:

- `item` can fall back to `|->`, `~`, or `#`.
- `item_alt` can fall back to `#->`.
- `header` can fall back to `#-` or `>>~`.

### BinaryFetch feature lists text preview...you can toggle and customize each module

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
