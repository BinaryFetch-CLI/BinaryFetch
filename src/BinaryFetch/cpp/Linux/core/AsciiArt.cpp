// AsciiArt.cpp
// Linux implementation

#include "core/AsciiArt.h"
#include "core/linux_ascii_database.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <locale>
#include <codecvt>
#include <map>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>
#include <pwd.h>
#include <errno.h>
#include <algorithm>

// ---------------- Helper path utilities ----------------

static std::string getHomeDir() {
    const char* home = std::getenv("HOME");
    if (home && *home != '\0') {
        return std::string(home);
    }
    struct passwd* pw = getpwuid(getuid());
    if (pw && pw->pw_dir) {
        return std::string(pw->pw_dir);
    }
    return ".";
}

static std::string getLinuxConfigDir() {
    const char* xdg = std::getenv("XDG_CONFIG_HOME");
    if (xdg && *xdg != '\0') {
        return std::string(xdg) + "/binaryfetch";
    }
    return getHomeDir() + "/.config/binaryfetch";
}

// ---------------- Distro Detection (/etc/os-release) ----------------

static std::string readOSReleaseValue(const std::string& key) {
    std::ifstream file("/etc/os-release");
    if (!file.is_open()) return "";

    std::string line;
    const std::string prefix = key + "=";
    while (std::getline(file, line)) {
        if (line.rfind(prefix, 0) != 0) continue;

        std::string value = line.substr(prefix.size());
        if (!value.empty() && (value.front() == '"' || value.front() == '\'')) {
            value.erase(0, 1);
        }
        if (!value.empty() && (value.back() == '"' || value.back() == '\'')) {
            value.pop_back();
        }
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        return value;
    }
    return "";
}

static std::vector<std::string> splitIdLikeChain(const std::string& raw) {
    std::vector<std::string> chain;
    std::stringstream ss(raw);
    std::string token;
    while (ss >> token) chain.push_back(token);
    return chain;
}

// Resolves the correct art for THIS system:
//   1. Direct ID= match (key or alias) in the database.
//   2. Walk ID_LIKE= chain, same key-or-alias lookup, in order.
//   3. Database's mandatory "linux" fallback entry.
static std::string resolveDistroArt() {
    const std::map<std::string, DistroArt>& db = getDistroArtDatabase();

    auto lookup = [&](const std::string& id) -> const std::string* {
        if (id.empty()) return nullptr;
        auto it = db.find(id);
        if (it != db.end()) return &it->second.art;
        for (const auto& [dbKey, entry] : db) {
            for (const auto& alias : entry.aliases) {
                if (alias == id) return &entry.art;
            }
        }
        return nullptr;
    };

    std::string id = readOSReleaseValue("ID");
    if (const std::string* art = lookup(id)) return *art;

    for (const auto& likeId : splitIdLikeChain(readOSReleaseValue("ID_LIKE"))) {
        if (const std::string* art = lookup(likeId)) return *art;
    }

    auto fallback = db.find("linux");
    return (fallback != db.end()) ? fallback->second.art : std::string();
}

// ---------------- Default Color Map ----------------
static const std::map<int, std::string> kDefaultColorMap = {
    {1, "\033[31m"}, {2, "\033[32m"}, {3, "\033[33m"},
    {4, "\033[34m"}, {5, "\033[35m"}, {6, "\033[36m"}, 
    {7, "\033[37m"}, {8, "\033[91m"}, {9, "\033[92m"},
    {10, "\033[93m"}, {11, "\033[94m"}, {12, "\033[95m"},
    {13, "\033[96m"}, {14, "\033[97m"}, {15, "\033[0m"}
};



// ---------------- Utility Functions ----------------

std::string stripAnsiSequences(const std::string& s) {
    static const std::regex ansi_re("\x1B\\[[0-9;]*[A-Za-z]");
    return std::regex_replace(s, ansi_re, "");
}

std::string processColorCodes(const std::string& line, const std::map<int, std::string>& colors) {
    std::string result = line;
    std::regex colorCodeRegex("\\$(\\d+)");
    std::smatch match;
    std::string processed;
    std::string remaining = result;

    while (std::regex_search(remaining, match, colorCodeRegex)) {
        processed += match.prefix();
        int colorNum = std::stoi(match[1].str());
        auto it = colors.find(colorNum);
        if (it != colors.end()) processed += it->second;
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
    if (wc == 0) return 0;
    if (wc < 0x1100) return 1;
    if ((wc >= 0x1100 && wc <= 0x115F) || (wc >= 0x2E80 && wc <= 0xA4CF) ||
        (wc >= 0xAC00 && wc <= 0xD7A3) || (wc >= 0xFF00 && wc <= 0xFF60)) return 2;
    return 1;
}

size_t visible_width(const std::string& s) {
    const std::string cleaned = stripAnsiSequences(s);
    const std::wstring w = utf8_to_wstring(cleaned);
    size_t width = 0;
    for (wchar_t wc : w) width += static_cast<size_t>(char_display_width(wc));
    return width;
}

void sanitizeLeadingInvisible(std::string& s) {
    if (s.size() >= 3 && (unsigned char)s[0] == 0xEF && (unsigned char)s[1] == 0xBB && (unsigned char)s[2] == 0xBF) {
        s.erase(0, 3);
    }
}

// ---------------- AsciiArt Class Implementation ----------------

AsciiArt::AsciiArt()
    : maxWidth(0), height(0), enabled(true), spacing(2),
      paddingUp(0), paddingLeft(0), paddingRight(0) {}

std::string AsciiArt::getUserArtPath() const {
    return getLinuxConfigDir() + "/BinaryArt.txt";
}

bool AsciiArt::ensureDirectoryExists(const std::string& path) const {
    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash == std::string::npos) return true;
    std::string directory = path.substr(0, lastSlash);

    size_t pos = 0;
    while ((pos = directory.find('/', pos + 1)) != std::string::npos) {
        std::string sub = directory.substr(0, pos);
        if (!sub.empty()) {
            mkdir(sub.c_str(), 0755);
        }
    }
    return (mkdir(directory.c_str(), 0755) == 0 || errno == EEXIST);
}

bool AsciiArt::copyDefaultArt(const std::string& destPath) const {
    if (!ensureDirectoryExists(destPath)) return false;

    std::ofstream dest(destPath, std::ios::binary);
    if (!dest.is_open()) return false;

    std::string art = resolveDistroArt();
    if (art.empty()) {
        std::cerr << "Warning: resolveDistroArt() returned empty — "
                     "database missing its mandatory 'linux' fallback entry.\n";
        return false;
    }
    dest << art;
    dest.close();
    return true;
}

bool AsciiArt::loadArtFromPath(const std::string& filepath) {
    artLines.clear();
    artWidths.clear();
    std::ifstream file(filepath);
    if (!file.is_open()) {
        enabled = false;
        return false;
    }

    const std::map<int, std::string>& activeColorMap = colorMap.empty() ? kDefaultColorMap : colorMap;

    std::string line;
    maxWidth = 0;
    bool isFirstLine = true;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (isFirstLine) { sanitizeLeadingInvisible(line); isFirstLine = false; }
        std::string processedLine = processColorCodes(line, activeColorMap);
        artLines.push_back(processedLine);
        size_t vlen = visible_width(processedLine);
        artWidths.push_back((int)vlen);
        if ((int)vlen > maxWidth) maxWidth = (int)vlen;
    }
    height = static_cast<int>(artLines.size());
    enabled = !artLines.empty();
    return enabled;
}

bool AsciiArt::loadArtFromEmbedded() {
    artLines.clear();
    artWidths.clear();

    const std::map<int, std::string>& activeColorMap = colorMap.empty() ? kDefaultColorMap : colorMap;

    std::string art = resolveDistroArt();
    if (art.empty()) {
        std::cerr << "Warning: resolveDistroArt() returned empty — "
                     "database missing its mandatory 'linux' fallback entry.\n";
        enabled = false;
        return false;
    }
    std::istringstream stream(art);
    std::string line;
    maxWidth = 0;
    bool isFirstLine = true;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (isFirstLine) { sanitizeLeadingInvisible(line); isFirstLine = false; }
        std::string processedLine = processColorCodes(line, activeColorMap);
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

    // Self-Heal if missing: create ~/.config/binaryfetch/BinaryArt.txt
    if (!fileExists) {
        if (!copyDefaultArt(userArtPath)) {
            return loadArtFromEmbedded();
        }
    }
    return loadArtFromPath(userArtPath);
}

bool AsciiArt::loadFromFile(const std::string& customPath) {
    return loadArtFromPath(customPath);
}

bool AsciiArt::isEnabled() const {
    return enabled;
}

void AsciiArt::setEnabled(bool enable) {
    enabled = enable;
}

void AsciiArt::clear() {
    artLines.clear();
    artWidths.clear();
    maxWidth = 0;
    height = 0;
}

void AsciiArt::setPadding(int up, int left, int right) {
    paddingUp = up;
    paddingLeft = left;
    paddingRight = right;
}

void AsciiArt::setColorMap(const std::map<int, std::string>& map) {
    colorMap = map;
}
