#include "AsciiArt.h"
#include <iostream>
#include <fstream>
#include <regex>
#include <locale>
#include <codecvt>
#include <cwchar>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>  // For SHGetFolderPath
#include <direct.h>  // For _mkdir
#else
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#endif

// ---------------- Helper functions for AsciiArt ----------------

// Remove ANSI color/format sequences (like "\x1b[31m") from a string
std::string stripAnsiSequences(const std::string& s) {
    static const std::regex ansi_re("\x1B\\[[0-9;]*[A-Za-z]");
    return std::regex_replace(s, ansi_re, "");
}

// Convert UTF-8 string to wide string (wstring)
std::wstring utf8_to_wstring(const std::string& s) {
    try {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        return conv.from_bytes(s);
    }
    catch (...) {
        // fallback: naive conversion
        std::wstring w;
        w.reserve(s.size());
        for (unsigned char c : s) w.push_back(static_cast<wchar_t>(c));
        return w;
    }
}

// Return visible width of a wide character (for printing aligned ASCII art)
int char_display_width(wchar_t wc) {
#if !defined(_WIN32)
    int w = wcwidth(wc);
    return (w < 0) ? 0 : w;
#else
    // Basic width heuristics for CJK and fullwidth characters
    if (wc == 0) return 0;
    if (wc < 0x1100) return 1;
    if ((wc >= 0x1100 && wc <= 0x115F) ||
        (wc >= 0x2E80 && wc <= 0xA4CF) ||
        (wc >= 0xAC00 && wc <= 0xD7A3) ||
        (wc >= 0xF900 && wc <= 0xFAFF) ||
        (wc >= 0xFE10 && wc <= 0xFE19) ||
        (wc >= 0xFE30 && wc <= 0xFE6F) ||
        (wc >= 0xFF00 && wc <= 0xFF60) ||
        (wc >= 0x20000 && wc <= 0x2FFFD) ||
        (wc >= 0x30000 && wc <= 0x3FFFD))
        return 2;
    return 1;
#endif
}

// Return visible width of UTF-8 string (ignoring ANSI sequences)
size_t visible_width(const std::string& s) {
    const std::string cleaned = stripAnsiSequences(s);
    const std::wstring w = utf8_to_wstring(cleaned);
    size_t width = 0;
    for (size_t i = 0; i < w.size(); ++i) width += static_cast<size_t>(char_display_width(w[i]));
    return width;
}

// ---------------- Sanitize leading invisible characters ----------------
void sanitizeLeadingInvisible(std::string& s) {
    // Remove UTF-8 BOM (EF BB BF)
    if (s.size() >= 3 &&
        (unsigned char)s[0] == 0xEF &&
        (unsigned char)s[1] == 0xBB &&
        (unsigned char)s[2] == 0xBF) {
        s.erase(0, 3);
    }

    // Remove leading zero-width spaces (U+200B -> E2 80 8B)
    while (s.size() >= 3 &&
        (unsigned char)s[0] == 0xE2 &&
        (unsigned char)s[1] == 0x80 &&
        (unsigned char)s[2] == 0x8B) {
        s.erase(0, 3);
    }
}

// ---------------- AsciiArt class implementation ----------------

AsciiArt::AsciiArt() : maxWidth(0), height(0), enabled(true), spacing(2) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8); // enable UTF-8 output on Windows console
#endif
}

// Get the path to user's ASCII art file in AppData
std::string AsciiArt::getUserArtPath() const {
#ifdef _WIN32
    char appDataPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appDataPath))) {
        std::string fullPath = std::string(appDataPath) + "\\BinaryFetch\\BinaryArt.txt";
        return fullPath;
    }
    // Fallback if SHGetFolderPath fails
    return "BinaryArt.txt";
#else
    // Linux/Mac: use ~/.config/BinaryFetch/BinaryArt.txt
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pw = getpwuid(getuid());
        home = pw->pw_dir;
    }
    return std::string(home) + "/.config/BinaryFetch/BinaryArt.txt";
#endif
}

// Ensure directory exists (create if needed)
bool AsciiArt::ensureDirectoryExists(const std::string& path) const {
    // Extract directory from full file path
    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash == std::string::npos) return true; // No directory in path

    std::string directory = path.substr(0, lastSlash);

#ifdef _WIN32
    // Try to create directory (will fail silently if exists)
    int result = _mkdir(directory.c_str());
    // 0 = success, -1 = error (but might already exist)
    if (result == 0 || errno == EEXIST) return true;
    return false;
#else
    // Linux/Mac
    int result = mkdir(directory.c_str(), 0755);
    if (result == 0 || errno == EEXIST) return true;
    return false;
#endif
}

// Copy default art to user location
bool AsciiArt::copyDefaultArt(const std::string& destPath) const {
    // Try multiple locations for default art file
    std::vector<std::string> searchPaths = {
        "Default_Ascii_Art.txt",           // Current working directory
        "DefaultAsciiArt.txt",             // Alternative naming (no underscores)
        "./Default_Ascii_Art.txt",         // Explicit current dir
        "./DefaultAsciiArt.txt",           // Explicit current dir (no underscores)
        "../Default_Ascii_Art.txt",        // Parent directory
        "../DefaultAsciiArt.txt",          // Parent directory (no underscores)
        "../../Default_Ascii_Art.txt",     // Grandparent directory
        "../../DefaultAsciiArt.txt",       // Grandparent directory (no underscores) - FOR x64/Debug builds
        "../../../Default_Ascii_Art.txt",  // Great-grandparent
        "../../../DefaultAsciiArt.txt"     // Great-grandparent (no underscores)
    };

#ifdef _WIN32
    // Get executable directory on Windows
    char exePath[MAX_PATH];
    if (GetModuleFileNameA(NULL, exePath, MAX_PATH)) {
        std::string exeDir = exePath;
        size_t lastSlash = exeDir.find_last_of("\\/");
        if (lastSlash != std::string::npos) {
            exeDir = exeDir.substr(0, lastSlash);
            searchPaths.push_back(exeDir + "\\Default_Ascii_Art.txt");
            searchPaths.push_back(exeDir + "\\DefaultAsciiArt.txt");
        }
    }
#endif

    // Try each path
    std::ifstream src;
    std::string foundPath;

    std::cout << "Searching for default ASCII art file..." << std::endl;
    for (const auto& path : searchPaths) {
        std::cout << "  Trying: " << path << "... ";
        src.open(path, std::ios::binary);
        if (src.is_open()) {
            foundPath = path;
            std::cout << "FOUND!" << std::endl;
            break;
        }
        std::cout << "not found" << std::endl;
    }

    if (!src.is_open()) {
        std::cerr << "\nError: Could not find Default_Ascii_Art.txt in any location." << std::endl;
        std::cerr << "Please ensure Default_Ascii_Art.txt is in the same folder as the executable." << std::endl;
        return false;
    }

    std::cout << "Using default art from: " << foundPath << std::endl;

    // Ensure destination directory exists
    if (!ensureDirectoryExists(destPath)) {
        std::cerr << "Warning: Could not create directory for " << destPath << std::endl;
        return false;
    }

    // Open destination file
    std::ofstream dest(destPath, std::ios::binary);
    if (!dest.is_open()) {
        std::cerr << "Warning: Could not create " << destPath << std::endl;
        return false;
    }

    // Copy content
    dest << src.rdbuf();

    src.close();
    dest.close();

    std::cout << "Created ASCII art file at: " << destPath << std::endl;
    return true;
}

// Internal helper: Load art from specific path
bool AsciiArt::loadArtFromPath(const std::string& filepath) {
    artLines.clear();
    artWidths.clear();

    std::ifstream file(filepath);
    if (!file.is_open()) {
        enabled = false;
        maxWidth = 0;
        height = 0;
        return false;
    }

    std::string line;
    maxWidth = 0;
    bool isFirstLine = true;

    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();

        // Sanitize only the first line for BOM / zero-width characters
        if (isFirstLine) {
            sanitizeLeadingInvisible(line);
            isFirstLine = false;
        }

        artLines.push_back(line);
        size_t vlen = visible_width(line);
        artWidths.push_back((int)vlen);
        if (static_cast<int>(vlen) > maxWidth) maxWidth = static_cast<int>(vlen);
    }

    file.close();

    height = static_cast<int>(artLines.size());
    enabled = !artLines.empty();
    return enabled;
}

// Main load function - automatically manages user's art file
bool AsciiArt::loadFromFile() {
    std::string userArtPath = getUserArtPath();

    // Check if file exists
    std::ifstream checkFile(userArtPath);
    bool fileExists = checkFile.good();
    checkFile.close();

    if (!fileExists) {
        // File doesn't exist - copy from default
        std::cout << "ASCII art not found at: " << userArtPath << std::endl;
        std::cout << "Copying from Default_Ascii_Art.txt..." << std::endl;

        if (!copyDefaultArt(userArtPath)) {
            std::cerr << "Failed to copy default art. Using fallback." << std::endl;
            // Try to load from project folder as last resort
            return loadArtFromPath("Default_Ascii_Art.txt");
        }
    }

    // Now load from user's location
    bool success = loadArtFromPath(userArtPath);

    if (success) {
        std::cout << "ASCII art loaded from: " << userArtPath << std::endl;
    }
    else {
        std::cerr << "Failed to load ASCII art from: " << userArtPath << std::endl;
    }

    return success;
}

// Advanced: Load from custom path (overrides default behavior)
bool AsciiArt::loadFromFile(const std::string& customPath) {
    return loadArtFromPath(customPath);
}

// Check if ASCII art is enabled
bool AsciiArt::isEnabled() const {
    return enabled;
}

// Enable/disable ASCII art display
void AsciiArt::setEnabled(bool enable) {
    enabled = enable;
}

// Clear loaded ASCII art
void AsciiArt::clear() {
    artLines.clear();
    artWidths.clear();
    maxWidth = 0;
    height = 0;
}

// ---------------- LivePrinter implementation ----------------

LivePrinter::LivePrinter(const AsciiArt& artRef) : art(artRef), index(0) {}

// Print a line with ASCII art padding
void LivePrinter::push(const std::string& infoLine) {
    printArtAndPad();
    if (!infoLine.empty()) std::cout << infoLine;
    std::cout << '\n';
    std::cout.flush();
    ++index;
}

// Push a blank line
void LivePrinter::pushBlank() {
    printArtAndPad();
    std::cout << '\n';
    std::cout.flush();
    ++index;
}

// Finish printing remaining ASCII art lines
void LivePrinter::finish() {
    while (index < art.getHeight()) {
        printArtAndPad();
        std::cout << '\n';
        ++index;
    }
}

// Print ASCII art line and pad to max width
void LivePrinter::printArtAndPad() {
    int artH = art.getHeight();
    int maxW = art.getMaxWidth();
    int spacing = art.getSpacing();

    if (index < artH) {
        const std::string& a = art.getLine(index);
        std::cout << a;
        int curW = art.getLineWidth(index);
        if (curW < maxW) std::cout << std::string(maxW - curW, ' ');
    }
    else {
        if (maxW > 0) std::cout << std::string(maxW, ' ');
    }

    if (spacing > 0) std::cout << std::string(spacing, ' ');
}

// Push multi-line formatted string to LivePrinter
void pushFormattedLines(LivePrinter& lp, const std::string& s) {
    std::istringstream iss(s);
    std::string line;
    while (std::getline(iss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        lp.push(line);
    }
}

/*
------------------------------------------------
DOCUMENTATION
------------------------------------------------
CLASS: AsciiArt
OBJECT: N/A (used in main.cpp as 'art')
DESCRIPTION: Handles ASCII art loading, alignment, and padding with automatic user file management.
FUNCTIONS:
    bool loadFromFile() -> bool
        Automatically load ASCII art from user's AppData folder.
        If file doesn't exist, copies from Default_Ascii_Art.txt and creates it.
        Returns true if successful.

    bool loadFromFile(const std::string& customPath) -> bool
        Load ASCII art from custom file path (advanced usage).
        Returns true if successful.

    bool isEnabled() const -> bool
        Check if ASCII art display is enabled.

    void setEnabled(bool enable) -> void
        Enable/disable ASCII art display.

    void clear() -> void
        Clear loaded ASCII art lines.

    int getMaxWidth() const -> int
        Returns max visible width of loaded ASCII art.

    int getHeight() const -> int
        Returns number of ASCII art lines.

    const std::string& getLine(int index) const -> const std::string&
        Returns ASCII art line at given index.

    int getLineWidth(int index) const -> int
        Returns visible width of line at given index.

    int getSpacing() const -> int
        Returns spacing between art and info lines.

PRIVATE HELPERS:
    std::string getUserArtPath() const -> std::string
        Get the full path to user's ASCII art file in AppData.

    bool ensureDirectoryExists(const std::string& path) const -> bool
        Create directory if it doesn't exist.

    bool copyDefaultArt(const std::string& destPath) const -> bool
        Copy Default_Ascii_Art.txt to user location.

    bool loadArtFromPath(const std::string& filepath) -> bool
        Load art from specific file path.
------------------------------------------------
CLASS: LivePrinter
OBJECT: N/A (used in main.cpp as 'lp')
DESCRIPTION: Streams system info lines alongside ASCII art.
FUNCTIONS:
    LivePrinter(const AsciiArt& artRef) -> constructor
        Initialize LivePrinter with a reference to AsciiArt.
    void push(const std::string& infoLine) -> void
        Print a line with ASCII art padding.
    void pushBlank() -> void
        Print a blank line with ASCII art padding.
    void finish() -> void
        Print remaining ASCII art lines if any.
------------------------------------------------
HELPER FUNCTIONS:
    std::string stripAnsiSequences(const std::string& s) -> std::string
        Remove ANSI escape sequences from string.
    std::wstring utf8_to_wstring(const std::string& s) -> std::wstring
        Convert UTF-8 string to wide string.
    int char_display_width(wchar_t wc) -> int
        Return display width of wide character.
    size_t visible_width(const std::string& s) -> size_t
        Return visible width of UTF-8 string (ignoring ANSI codes).
    void sanitizeLeadingInvisible(std::string& s) -> void
        Remove BOM and zero-width spaces from string start.
    void pushFormattedLines(LivePrinter& lp, const std::string& s) -> void
        Push multi-line formatted string to LivePrinter.
------------------------------------------------
*/