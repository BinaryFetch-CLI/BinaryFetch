

# AsciiArt.cpp Documentation

## Overview

The `AsciiArt` module provides a comprehensive system for loading, processing, and displaying ASCII art with advanced features including UTF-8 support, ANSI color codes, inline color markup, and cross-platform file management.

---

## Table of Contents

1. [Features](#features)
2. [Color Code System](#color-code-system)
3. [Class Reference](#class-reference)
4. [File Locations](#file-locations)
5. [Usage Examples](#usage-examples)
6. [Platform-Specific Behavior](#platform-specific-behavior)

---

## Features

### Core Functionality
- **UTF-8 Support** - Full Unicode/UTF-8 character support with proper width calculation
- **ANSI Color Processing** - Handles existing ANSI escape sequences
- **Inline Color Codes** - Custom `$n` syntax for easy color markup
- **CJK Character Support** - Proper display width for Chinese, Japanese, Korean characters
- **Cross-Platform** - Windows, Linux, and macOS support
- **Automatic File Management** - Auto-creates directories and copies default art files
- **Live Printing** - Stream system info alongside ASCII art with proper alignment

### Advanced Features
- **BOM Handling** - Automatically strips UTF-8 BOM and zero-width characters
- **Width Calculation** - Accurate visible width calculation ignoring ANSI codes
- **Alignment Preservation** - Maintains proper spacing with colored text
- **Multi-Color Lines** - Multiple color changes within a single line

---

## Color Code System

### Available Colors

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
$1⠀⣄⠀⠀⠏⣤⣤⣀⡀⠀⠀⠀⠀⠀⠾⢯⣀
```
Output: Entire line in red

**Multiple colors per line:**
```
$2⠀⡠⠋⡤⠠$3⢀⠐⠁⠀⠈$1⣙⢯⡃⠀⢈⡻⣦
```
Output: Green → Yellow → Red

**No color (default white):**
```
⠀⠀⣻⠃⠀⣰⡿⠛⠁⠀⠀⠀⢤⣀⡀⠀⠺⣿⡟⠛⠁
```
Output: Standard white text

### Important Notes
- Color codes (`$n`) are **not displayed** in output
- Each line **automatically resets** color at the end
- Colors **do not bleed** to the next line
- Color codes **do not affect alignment** calculations

---

## Class Reference

### `AsciiArt` Class

Main class for managing ASCII art loading and storage.

#### Constructor
```cpp
AsciiArt()
```
Initializes the ASCII art system with UTF-8 console output enabled.

#### Public Methods

##### `bool loadFromFile()`
Loads ASCII art from the default user location. Automatically copies from `Default_Ascii_Art.txt` if not found.

**Returns:** `true` if successful, `false` otherwise

**Example:**
```cpp
AsciiArt art;
if (art.loadFromFile()) {
    // Art loaded successfully
}
```

##### `bool loadFromFile(const std::string& customPath)`
Loads ASCII art from a custom file path.

**Parameters:**
- `customPath` - Full path to ASCII art file

**Returns:** `true` if successful, `false` otherwise

##### `bool isEnabled() const`
Check if ASCII art is currently enabled.

##### `void setEnabled(bool enable)`
Enable or disable ASCII art display.

##### `void clear()`
Clear all loaded ASCII art from memory.

##### `int getHeight() const`
Get the number of lines in the loaded ASCII art.

##### `int getMaxWidth() const`
Get the maximum visible width of any line.

##### `int getSpacing() const`
Get the spacing between ASCII art and info text.

##### `const std::string& getLine(int index) const`
Get a specific line of ASCII art (with color codes processed).

##### `int getLineWidth(int index) const`
Get the visible width of a specific line.

---

### `LivePrinter` Class

Utility class for printing system information alongside ASCII art.

#### Constructor
```cpp
LivePrinter(const AsciiArt& artRef)
```

#### Public Methods

##### `void push(const std::string& infoLine)`
Print a line of information next to the corresponding ASCII art line.

**Example:**
```cpp
LivePrinter lp(art);
lp.push("OS: Windows 11");
lp.push("CPU: Intel i7");
```

##### `void pushBlank()`
Print a blank line (ASCII art only, no info).

##### `void finish()`
Complete printing by displaying remaining ASCII art lines.

---

### Helper Functions

#### `void pushFormattedLines(LivePrinter& lp, const std::string& s)`
Push multi-line formatted strings to LivePrinter.

**Example:**
```cpp
std::string multiLine = "Line 1\nLine 2\nLine 3";
pushFormattedLines(lp, multiLine);
```

#### `std::string stripAnsiSequences(const std::string& s)`
Remove ANSI color/format codes from a string.

#### `std::wstring utf8_to_wstring(const std::string& s)`
Convert UTF-8 string to wide string for proper Unicode handling.

#### `size_t visible_width(const std::string& s)`
Calculate visible width of a string (ignoring ANSI codes).

#### `int char_display_width(wchar_t wc)`
Get display width of a wide character (1 for ASCII, 2 for CJK).

---

## File Locations

### Windows
**Primary Location:**
```
C:\Users\Default\AppData\Local\BinaryFetch\BinaryArt.txt
```

**Default Art Search Paths** (in order):
1. `Default_Ascii_Art.txt` (current directory)
2. `DefaultAsciiArt.txt` (current directory)
3. `./Default_Ascii_Art.txt`
4. `../Default_Ascii_Art.txt`
5. `../../Default_Ascii_Art.txt` (for Debug builds)
6. `../../../Default_Ascii_Art.txt`
7. Executable directory + `Default_Ascii_Art.txt`

### Linux/macOS
**Primary Location:**
```
~/.config/BinaryFetch/BinaryArt.txt
```

---

## Usage Examples

### Basic Usage

```cpp
#include "AsciiArt.h"

int main() {
    // Load ASCII art
    AsciiArt art;
    if (!art.loadFromFile()) {
        std::cerr << "Failed to load ASCII art" << std::endl;
        return 1;
    }
    
    // Create live printer
    LivePrinter lp(art);
    
    // Print system info
    lp.push("OS: Windows 11");
    lp.push("CPU: Intel i7-12700K");
    lp.push("RAM: 32 GB DDR5");
    lp.push("GPU: NVIDIA RTX 4080");
    
    // Finish printing
    lp.finish();
    
    return 0;
}
```

### Custom Path Loading

```cpp
AsciiArt art;
if (art.loadFromFile("C:\\MyArt\\custom.txt")) {
    LivePrinter lp(art);
    lp.push("Using custom ASCII art!");
    lp.finish();
}
```

### Dynamic Enable/Disable

```cpp
AsciiArt art;
art.loadFromFile();

// Disable ASCII art
art.setEnabled(false);

// Enable again
art.setEnabled(true);
```

---

## Platform-Specific Behavior

### Windows
- Uses `CSIDL_LOCAL_APPDATA` for file storage
- UTF-8 console output via `SetConsoleOutputCP(CP_UTF8)`
- Directory creation via `_mkdir()`
- Character width uses heuristic-based calculation

### Linux/macOS
- Uses `~/.config/` for configuration files
- Character width via `wcwidth()` system call
- Directory creation via `mkdir()` with 0755 permissions
- Environment variables: `$HOME` with fallback to `getpwuid()`

---

## Technical Details

### Color Processing Pipeline

1. **Load File** → Read BinaryArt.txt line by line
2. **Sanitize** → Remove BOM and zero-width characters (first line only)
3. **Process Color Codes** → Replace `$n` with ANSI escape sequences
4. **Add Reset** → Append `\033[0m` to end of each line
5. **Calculate Width** → Compute visible width (ANSI codes excluded)
6. **Store** → Save processed line with metadata

### Alignment Algorithm

```
Visible Width = String Length - ANSI Code Length - CJK Extra Width
Padding = Max Width - Current Line Width
Output = Art Line + Padding + Spacing + Info Line
```

### Error Handling

The system gracefully handles:
- Missing files (auto-copies default)
- Invalid UTF-8 sequences (fallback conversion)
- Missing directories (auto-creates)
- Permission errors (silent failure with fallback)
- Invalid color codes (ignored, no replacement)

---

## Troubleshooting

### ASCII Art Not Loading
1. Ensure `Default_Ascii_Art.txt` exists in project directory
2. Check file permissions on target location
3. On Windows, verify user has write access to `C:\Users\Default\AppData\Local`

### Colors Not Displaying
1. Verify terminal supports ANSI colors
2. Check that `$n` codes use valid numbers (1-15)
3. Ensure no spaces between `$` and number

### Alignment Issues
1. Check for tab characters (use spaces instead)
2. Verify UTF-8 encoding of art file
3. Ensure CJK characters are properly counted

### Width Calculation Problems
1. Strip ANSI codes before measuring
2. Use `visible_width()` instead of `string.length()`
3. Test with `stripAnsiSequences()` for debugging

---

## Best Practices

1. **Always use UTF-8 encoding** for ASCII art files
2. **Test colors in target terminal** before deployment
3. **Keep color codes minimal** for better readability
4. **Use bright colors** for better visibility
5. **End complex color sequences** with `$15` (reset)
6. **Avoid tabs** - use spaces for alignment
7. **Test on all target platforms** (Windows/Linux/Mac)

---

## Future Enhancements

Potential improvements:
- RGB color support (24-bit true color)
- Background color codes
- Bold/italic/underline formatting
- Animation support
- Dynamic color themes
- Color scheme presets

---

## Dependencies

- Standard C++ Library (C++11 or later)
- `<regex>` for color code parsing
- `<codecvt>` for UTF-8 conversion
- Platform-specific headers:
  - Windows: `<windows.h>`, `<shlobj.h>`, `<direct.h>`
  - Unix: `<sys/stat.h>`, `<unistd.h>`, `<pwd.h>`

---

## License & Credits

Part of the BinaryFetch system information tool.

**Key Features:**
- Inline color code system
- Cross-platform UTF-8 support
- Automatic file management
- CJK character width calculation

---

## Version History

**Current Version:** 2.0
- Added inline color code support ($n syntax)
- Changed storage location to Default user profile
- Improved ANSI sequence handling
- Added automatic color reset per line

**Previous Version:** 1.0
- Basic ASCII art loading
- UTF-8 support
- Cross-platform compatibility