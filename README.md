

# 🧬 BinaryFetch

> **A modular system information fetcher for Windows — engineered for reliability, customization, and community-driven profiles.**

BinaryFetch is not “another neofetch clone.”
It is a **systems engineering project** focused on **clean architecture**, **self-healing configuration**, and **extreme customization with minimal user effort**.

---

## ✨ Key Philosophy

> **Simplicity is the priority.**

* Two user-editable files
* Self-healing engine
* Modular, scalable design
* Clean separation between *data*, *logic*, and *presentation*

---

## 🚀 What Makes BinaryFetch Different?

### 🧠 Engine-First Design

BinaryFetch guarantees that **the program never crashes due to missing user files**.

If a user:

* deletes `BinaryArt.txt`
* deletes `BinaryFetch.json`
* corrupts configuration
* runs BinaryFetch for the first time

👉 **The engine automatically recreates everything from defaults.**

No prompts.
No errors.
No user intervention.

---

## 🗂️ User Customization (Only 2 Files)

| File               | Purpose                         |
| ------------------ | ------------------------------- |
| `BinaryArt.txt`    | User ASCII art (fully editable, copy-paste-done !) |
| `BinaryFetch.json` | Module configuration & layout   |

That’s it.

No profiles.
No databases.
No complex formats.

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

---

## 🧠 Core Information Modules

### 1️⃣ Operating System (`OSInfo.h`)

* OS name, version, build
* Architecture (32/64-bit)
* Kernel info
* Install date
* Serial number
* Uptime

---

### 2️⃣ CPU / Processor (`CPUInfo.h`)

* Brand & model
* Utilization
* Current & base frequency
* Cores / threads
* Virtualization support
* Cache (L1/L2/L3)
* Process / thread / handle counts

---

### 3️⃣ Memory / RAM (`RAMInfo.h`)

* Total RAM
* Free RAM
* Used percentage
* Detailed formatted output

---

### 4️⃣ GPU (`GPUInfo.h`, `DetailedGPUInfo.h`)

* GPU name & vendor
* VRAM total
* Driver version
* GPU usage
* Core count
* Temperature
* Multi-GPU detection
* Primary GPU selection

---

### 5️⃣ Storage (`StorageInfo.h`)

* Disk list
* Total & free space
* Disk type (SSD/HDD)
* Serial numbers
* Read/write speed
* Predicted performance

---

### 6️⃣ Network (`NetworkInfo.h`)

* Local IP
* Public IP
* MAC address
* SSID / adapter name
* Network speed
* Locale

---

### 7️⃣ Performance (`PerformanceInfo.h`)

* CPU usage
* RAM usage
* Disk usage
* GPU usage
* System uptime

---

### 8️⃣ User Info (`UserInfo.h`)

* Username
* Computer name
* Domain
* User groups
* Admin status

---

### 9️⃣ BIOS / Motherboard (`SystemInfo.h`)

* BIOS vendor, version, date
* Motherboard model & manufacturer
* Environment variables

---

### 🔟 Display (`DisplayInfo.h`)

* Multi-monitor detection
* Resolution
* Refresh rate
* Monitor model & brand

---

### 🔊 Extras (`ExtraInfo.h`)

* Audio input/output devices
* Active device detection
* Power status (desktop/laptop)
* Battery percentage & charging state

---

## 🧬 Compact Mode

BinaryFetch includes a **Compact Mode** for clean, readable output.

Example:

```
[OS]      -> Windows 11 10.0 (Build 22631) (64-bit)
[CPU]     -> Ryzen 5 5600G (6C/12T) @ 3.89 GHz
[GPU]     -> RTX 4070 SUPER (9%) (11.99 GB)
[Memory]  -> 47.79 GB (Free: 28.44 GB) (40%)
[Display] -> 2 Monitors @ 60Hz
[User]    -> @coffee~ (Admin)
```

Each compact section is powered by **dedicated lightweight classes** (`CompactCPU`, `CompactGPU`, etc.).

---

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

---
