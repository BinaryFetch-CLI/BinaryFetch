
#include "AsciiArt.h"
#include "DistroDetector.h"
#include "resource.h"
#include <iostream>
#include <fstream>
#include <regex>
#include <locale>
#include <codecvt>
#include <cwchar>
#include <sstream>
#include <map>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <direct.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#endif

// ---------------- Color Map (Cyan & White Theme) ----------------
static const std::map<int, std::string> colorMap = {
    {1, "\033[31m"}, {2, "\033[32m"}, {3, "\033[33m"},
    {4, "\033[34m"}, {5, "\033[35m"}, {6, "\033[36m"}, // Cyan
    {7, "\033[37m"}, {8, "\033[91m"}, {9, "\033[92m"},
    {10, "\033[93m"}, {11, "\033[94m"}, {12, "\033[95m"},
    {13, "\033[96m"}, {14, "\033[97m"}, {15, "\033[0m"}
};

// ---------------- Utility Functions ----------------

std::string stripAnsiSequences(const std::string& s) {
    static const std::regex ansi_re("\x1B\\[[0-9;]*[A-Za-z]");
    return std::regex_replace(s, ansi_re, "");
}

std::string processColorCodes(const std::string& line) {
    std::string result = line;
    std::regex colorCodeRegex("\\$(\\d+)");
    std::smatch match;
    std::string processed;
    std::string remaining = result;

    while (std::regex_search(remaining, match, colorCodeRegex)) {
        processed += match.prefix();
        int colorNum = std::stoi(match[1].str());
        auto it = colorMap.find(colorNum);
        if (it != colorMap.end()) processed += it->second;
        remaining = match.suffix();
    }
    processed += remaining + "\033[0m";
    return processed;
}

std::wstring utf8_to_wstring(const std::string& s) {
    try {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        return conv.from_bytes(s);
    }
    catch (...) {
        std::wstring w;
        for (unsigned char c : s) w.push_back(static_cast<wchar_t>(c));
        return w;
    }
}

int char_display_width(wchar_t wc) {
#if !defined(_WIN32)
    int w = wcwidth(wc);
    return (w < 0) ? 0 : w;
#else
    if (wc == 0) return 0;
    if (wc < 0x1100) return 1;
    if ((wc >= 0x1100 && wc <= 0x115F) || (wc >= 0x2E80 && wc <= 0xA4CF) ||
        (wc >= 0xAC00 && wc <= 0xD7A3) || (wc >= 0xFF00 && wc <= 0xFF60)) return 2;
    return 1;
#endif
}

size_t visible_width(const std::string& s) {
    const std::string cleaned = stripAnsiSequences(s);
    const std::wstring w = utf8_to_wstring(cleaned);
    size_t width = 0;
    for (wchar_t wc : w) width += static_cast<size_t>(char_display_width(wc));
    return width;
}

void sanitizeLeadingInvisible(std::string& s) {
    if (s.size() >= 3 && (unsigned char)s[0] == 0xEF && (unsigned char)s[1] == 0xBB && (unsigned char)s[2] == 0xBF) s.erase(0, 3);
}

// ---------------- AsciiArt Class ----------------

AsciiArt::AsciiArt() : maxWidth(0), height(0), enabled(true), spacing(2) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
}

std::string AsciiArt::getUserArtPath() const {
#ifdef _WIN32
    return "C:\\Users\\Public\\BinaryFetch\\BinaryArt.txt";
#else
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pw = getpwuid(getuid());
        home = pw ? pw->pw_dir : "/tmp";
    }
    return std::string(home) + "/.config/binaryfetch/BinaryArt.txt";
#endif
}

bool AsciiArt::ensureDirectoryExists(const std::string& path) const {
    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash == std::string::npos) return true;
    std::string directory = path.substr(0, lastSlash);
#ifdef _WIN32
    return (_mkdir(directory.c_str()) == 0 || errno == EEXIST);
#else
    return (mkdir(directory.c_str(), 0755) == 0 || errno == EEXIST);
#endif
}

// ========== THE SELF-HEALING ENGINE ==========
bool AsciiArt::copyDefaultArt(const std::string& destPath) const {
    if (!ensureDirectoryExists(destPath)) return false;

#ifdef _WIN32
    HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(102), RT_RCDATA);
    if (!hRes) return false;

    HGLOBAL hData = LoadResource(NULL, hRes);
    DWORD size = SizeofResource(NULL, hRes);
    const char* data = static_cast<const char*>(LockResource(hData));

    std::ofstream dest(destPath, std::ios::binary);
    if (dest.is_open()) {
        dest.write(data, size);
        dest.close();
        return true;
    }
    return false;
#else
    std::vector<std::string> searchPaths = {
        "DefaultAsciiArt.txt",
        "./DefaultAsciiArt.txt",
        "../DefaultAsciiArt.txt",
        "/usr/share/binaryfetch/BinaryArt.txt",
        "/usr/local/share/binaryfetch/BinaryArt.txt",
    };

    char exePath[4096];
    ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
    if (len != -1) {
        exePath[len] = '\0';
        std::string exeDir = exePath;
        size_t lastSlash = exeDir.find_last_of('/');
        if (lastSlash != std::string::npos) {
            exeDir = exeDir.substr(0, lastSlash);
            searchPaths.push_back(exeDir + "/DefaultAsciiArt.txt");
            searchPaths.push_back(exeDir + "/BinaryArt.txt");
        }
    }

    std::ifstream src;
    for (const auto& path : searchPaths) {
        src.open(path, std::ios::binary);
        if (src.is_open()) break;
    }

    if (!src.is_open()) return false;

    std::ofstream dest(destPath, std::ios::binary);
    if (!dest.is_open()) {
        src.close();
        return false;
    }

    dest << src.rdbuf();
    src.close();
    dest.close();
    return true;
#endif
}

bool AsciiArt::loadArtFromPath(const std::string& filepath) {
    artLines.clear();
    artWidths.clear();
    std::ifstream file(filepath);
    if (!file.is_open()) {
        enabled = false;
        return false;
    }

    std::string line;
    maxWidth = 0;
    bool isFirstLine = true;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (isFirstLine) { sanitizeLeadingInvisible(line); isFirstLine = false; }
        std::string processedLine = processColorCodes(line);
        artLines.push_back(processedLine);
        size_t vlen = visible_width(processedLine);
        artWidths.push_back((int)vlen);
        if ((int)vlen > maxWidth) maxWidth = (int)vlen;
    }
    height = static_cast<int>(artLines.size());
    enabled = !artLines.empty();
    return enabled;
}

bool AsciiArt::loadFromFile() {
    std::string userArtPath = getUserArtPath();
    std::ifstream checkFile(userArtPath);
    bool fileExists = checkFile.good();
    checkFile.close();

    if (!fileExists) {
#ifndef _WIN32
        Distro distro = DistroDetector::detect();
        std::string distroArt = DistroDetector::getAsciiArt(distro);
        if (!distroArt.empty()) {
            return loadArtFromString(distroArt);
        }
#endif
        if (!copyDefaultArt(userArtPath)) {
            return loadArtFromPath("DefaultAsciiArt.txt");
        }
    }
    return loadArtFromPath(userArtPath);
}

bool AsciiArt::loadArtFromString(const std::string& artContent) {
    artLines.clear();
    artWidths.clear();
    maxWidth = 0;

    std::istringstream stream(artContent);
    std::string line;
    bool isFirstLine = true;
    
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (isFirstLine) { sanitizeLeadingInvisible(line); isFirstLine = false; }
        std::string processedLine = processColorCodes(line);
        artLines.push_back(processedLine);
        size_t vlen = visible_width(processedLine);
        artWidths.push_back((int)vlen);
        if ((int)vlen > maxWidth) maxWidth = (int)vlen;
    }
    
    height = static_cast<int>(artLines.size());
    enabled = !artLines.empty();
    return enabled;
}

// ---------------- LivePrinter ----------------

LivePrinter::LivePrinter(const AsciiArt& artRef) : art(artRef), index(0) {}

void LivePrinter::push(const std::string& infoLine) {
    printArtAndPad();
    if (!infoLine.empty()) std::cout << infoLine;
    std::cout << '\n';
    index++;
}

void LivePrinter::printArtAndPad() {
    int artH = art.getHeight();
    int maxW = art.getMaxWidth();
    int spacing = art.getSpacing();

    if (index < artH) {
        std::cout << art.getLine(index);
        int curW = art.getLineWidth(index);
        if (curW < maxW) std::cout << std::string(maxW - curW, ' ');
    }
    else if (maxW > 0) {
        std::cout << std::string(maxW, ' ');
    }
    if (spacing > 0) std::cout << std::string(spacing, ' ');
}

void LivePrinter::finish() {
    while (index < art.getHeight()) {
        printArtAndPad();
        std::cout << '\n';
        index++;
    }
}

