
![Alt text](123456.png)
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

`C:\Users\Default\AppData\Local\BinaryFetch\`
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

---
