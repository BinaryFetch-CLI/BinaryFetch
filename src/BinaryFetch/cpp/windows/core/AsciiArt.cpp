// AsciiArt.cpp
// Windows-only.

/*
 * ============================================================
 *                  ASCII ART MANAGEMENT
 * ============================================================
 *
 * OVERVIEW
 * ------------------------------------------------------------
 * The ASCII-art system separates three responsibilities:
 *
 *   1. Source management
 *      Determines where the ASCII art comes from.
 *
 *   2. Preparation
 *      Loads the art and calculates the metadata required
 *      for correct terminal alignment.
 *
 *   3. Rendering
 *      Displays the prepared ASCII art beside system
 *      information.
 *
 * AsciiArt is responsible for the first two stages.
 * LivePrinter is responsible for the final rendering stage.
 *
 *
 * ------------------------------------------------------------
 * ASCII ART SOURCE
 * ------------------------------------------------------------
 *
 * BinaryArt.txt is the persistent, user-editable ASCII-art
 * source.
 *
 * The default ASCII art is embedded directly inside this
 * source file as kDefaultAsciiArt.
 *
 * The embedded default is NOT the normal runtime source.
 * Its purpose is to initialize BinaryArt.txt when the file
 * does not exist, and to provide a fallback if the file cannot
 * be created.
 *
 *
 * ------------------------------------------------------------
 * LOADING WORKFLOW
 * ------------------------------------------------------------
 *
 *        main.cpp
 *              |
 *              v
 *        AsciiArt::loadFromFile()
 *              |
 *              v
 *        Resolve the BinaryArt.txt path
 *              |
 *              v
 *        Does BinaryArt.txt exist?
 *              |
*               v
 *    +-----------------------+
 *    |                       |
 *   YES                      NO
 *    |                       |
 *    |                       v
 *    |              Create BinaryArt.txt at ("C:\Users\Public\BinaryFetch\BinaryArt.txt")
 *    |                       |
 *    |                       v
 *    |              Implement the ascii art at ("C:\Users\Public\BinaryFetch\BinaryArt.txt")
 *    |                       |
 *    +-----------+-----------+
 *                |
 *                v
 *        loadArtFromPath() - means, it will load the Ascii Art from ("C:\Users\Public\BinaryFetch\BinaryArt.txt")
 *                |
 *                v
 *        Process the file contents
 *                |
 *                +--> Process color codes
 *                |
 *                +--> Calculate visible width
 *                |
 *                +--> Store each art line
 *                |
 *                +--> Store each line width
 *                |
 *                +--> Calculate maximum width
 *                |
 *                +--> Calculate total height
 *                |
 *                v
 *          ASCII ART READY
 *                |
 *                v
 *           LivePrinter: prints the art & manage forma
 *                |
 *                v
 *          Terminal Output
 *
 *
 * ------------------------------------------------------------
 * EXISTING FILE
 * ------------------------------------------------------------
 *
 * If BinaryArt.txt already exists, it is treated as the
 * authoritative source.
 *
 * The contents are loaded directly from the file and converted
 * into the internal AsciiArt representation.
 *
 * No default art is involved in this path.
 *
 *
 * ------------------------------------------------------------
 * MISSING FILE
 * ------------------------------------------------------------
 *
 * If BinaryArt.txt does not exist, the embedded default art is
 * used to initialize it.
 *
 *      kDefaultAsciiArt
 *              |
 *              v
 *      BinaryArt.txt
 *              |
 *              v
 *      loadArtFromPath()
 *
 * After the file has been created, it follows the same loading
 * path as an existing user-created file.
 *
 * This keeps the initialization path and normal runtime path
 * consistent.
 *
 *
 * ------------------------------------------------------------
 * DISK WRITE FAILURE
 * ------------------------------------------------------------
 *
 * If BinaryArt.txt cannot be created (for example, because of
 * insufficient permissions), the system does not require the
 * file in order to display the default art.
 *
 * Instead:
 *
 *      kDefaultAsciiArt
 *              |
 *              v
 *      loadArtFromEmbedded()
 *              |
 *              v
 *      Runtime ASCII art
 *
 * This is a fallback path only. Under normal conditions,
 * BinaryArt.txt remains the persistent source.
 *
 *
 * ------------------------------------------------------------
 * RUNTIME RESPONSIBILITIES
 * ------------------------------------------------------------
 *
 * After loading, AsciiArt maintains the prepared representation:
 *
 *   artLines
 *       -> processed ASCII-art lines
 *
 *   artWidths
 *       -> visible width of each line
 *
 *   maxWidth
 *       -> width of the widest line
 *
 *   height
 *       -> total number of lines
 *
 * These values allow the rendering layer to perform alignment
 * without needing to understand file loading or parsing.
 *
 *
 * ------------------------------------------------------------
 * RENDERING
 * ------------------------------------------------------------
 *
 * LivePrinter receives the prepared AsciiArt object.
 *
 * It does NOT:
 *
 *   - locate BinaryArt.txt
 *   - create BinaryArt.txt
 *   - access the embedded default
 *   - parse the ASCII-art file
 *   - calculate the original dimensions
 *
 * Its only responsibility is to render the already-prepared
 * ASCII art beside system information.
 *
 *
 * ------------------------------------------------------------
 * DESIGN PRINCIPLE
 * ------------------------------------------------------------
 *
 *      Embedded Default
 *             |
 *             |  initialization / fallback
 *             v
 *       BinaryArt.txt
 *             |
 *             |  persistent user source
 *             v
 *          AsciiArt
 *             |
 *             |  parsed runtime representation
 *             v
 *        LivePrinter
 *             |
 *             |  rendering
 *             v
 *       Terminal Output
 *
 * In short:
 *
 *   AsciiArt     = locate, initialize, load and prepare the art.
 *
 *   LivePrinter  = render the prepared art beside system info.
 *
 * This separation keeps file management, data preparation and
 * terminal rendering independent from each other.
 *
 * ============================================================
 */
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#include "AsciiArt.h"
#include <iostream>
#include <fstream>
#include <regex>
#include <locale>
#include <codecvt>
#include <sstream>
#include <map>
#include <windows.h>
#include <shlobj.h>
#include <direct.h>

// ---------------- Color Map (Cyan & White Theme) ----------------
static const std::map<int, std::string> colorMap = {
    {1, "\033[31m"}, {2, "\033[32m"}, {3, "\033[33m"},
    {4, "\033[34m"}, {5, "\033[35m"}, {6, "\033[36m"}, // Cyan
    {7, "\033[37m"}, {8, "\033[91m"}, {9, "\033[92m"},
    {10, "\033[93m"}, {11, "\033[94m"}, {12, "\033[95m"},
    {13, "\033[96m"}, {14, "\033[97m"}, {15, "\033[0m"}
};



/*
This exact ascii art will be pasted on the "C:\Users\Public\BinaryFetch\BinaryArt.txt"
BinaryFetch will load it from "C:\Users\Public\BinaryFetch\BinaryArt.txt"
*/
static const std::string kDefaultAsciiArt =
R"ASCIIART($1##################### $15 <<<<<<<<<<<<<<<<<<<<<<
$1##################### $15 <<<<<<<<<<<<<<<<<<<<<<
$1##################### $15 <<<<<<<<<<<<<<<<<<<<<<
$1##################### $15 <<<<<<<<<<<<<<<<<<<<<<
$1##################### $15 <<<<<<<<<<<<<<<<<<<<<<
$1##################### $15 <<<<<<<<<<<<<<<<<<<<<<
$1##################### $15 <<<<<<<<<<<<<<<<<<<<<<
$1##################### $15 <<<<<<<<<<<<<<<<<<<<<<
$1##################### $15 <<<<<<<<<<<<<<<<<<<<<<
$1##################### $15 <<<<<<<<<<<<<<<<<<<<<<

>>>>>>>>>>>>>>>>>>>>> $1 ######################
>>>>>>>>>>>>>>>>>>>>> $1 ######################
>>>>>>>>>>>>>>>>>>>>> $1 ######################
>>>>>>>>>>>>>>>>>>>>> $1 ######################
>>>>>>>>>>>>>>>>>>>>> $1 ######################
>>>>>>>>>>>>>>>>>>>>> $1 ######################
>>>>>>>>>>>>>>>>>>>>> $1 ######################
>>>>>>>>>>>>>>>>>>>>> $1 ######################
>>>>>>>>>>>>>>>>>>>>> $1 ######################
>>>>>>>>>>>>>>>>>>>>> $1 ######################
)ASCIIART";

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
    if (s.size() >= 3 && (unsigned char)s[0] == 0xEF && (unsigned char)s[1] == 0xBB && (unsigned char)s[2] == 0xBF) s.erase(0, 3);
}

// ---------------- AsciiArt Class ----------------

AsciiArt::AsciiArt() : maxWidth(0), height(0), enabled(true), spacing(2) {
    SetConsoleOutputCP(CP_UTF8);
}

std::string AsciiArt::getUserArtPath() const {
    return "C:\\Users\\Public\\BinaryFetch\\BinaryArt.txt";
}

bool AsciiArt::ensureDirectoryExists(const std::string& path) const {
    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash == std::string::npos) return true;
    std::string directory = path.substr(0, lastSlash);
    return (_mkdir(directory.c_str()) == 0 || errno == EEXIST);
}

// ========== THE SELF-HEALING ENGINE (Embedded Art) ==========
// No more resource lookup / external DefaultAsciiArt.txt search.
// The default art is baked into the binary as kDefaultAsciiArt and
// simply gets written out to destPath the first time it's needed.
bool AsciiArt::copyDefaultArt(const std::string& destPath) const {
    if (!ensureDirectoryExists(destPath)) return false;

    std::ofstream dest(destPath, std::ios::binary);
    if (!dest.is_open()) return false;

    dest << kDefaultAsciiArt;
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

bool AsciiArt::loadArtFromEmbedded() {
    artLines.clear();
    artWidths.clear();

    std::istringstream stream(kDefaultAsciiArt);
    std::string line;
    maxWidth = 0;
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

bool AsciiArt::loadFromFile() {
    std::string userArtPath = getUserArtPath();
    std::ifstream checkFile(userArtPath);
    bool fileExists = checkFile.good();
    checkFile.close();

    // Self-Heal if the file is missing 🧬
    if (!fileExists) {
        if (!copyDefaultArt(userArtPath)) {
            // Couldn't write the file to disk (e.g. permissions) — fall
            // back to parsing the embedded art straight from memory
            // instead of the old "read DefaultAsciiArt.txt" fallback.
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

void LivePrinter::pushBlank() {
    printArtAndPad();
    std::cout << '\n';
    index++;
}

void LivePrinter::finish() {
    while (index < art.getHeight()) {
        printArtAndPad();
        std::cout << '\n';
        index++;
    }
}

void pushFormattedLines(LivePrinter& lp, const std::string& s) {
    std::istringstream iss(s);
    std::string line;
    while (std::getline(iss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        lp.push(line);
    }
}
/*
Color Code Feature:
Use $n in the art to set colors (n = 1-15):
$1 = red              $8 = bright_red
$2 = green            $9 = bright_green
$3 = yellow           $10 = bright_yellow
$4 = blue             $11 = bright_blue
$5 = magenta          $12 = bright_magenta
$6 = cyan             $13 = bright_cyan
$7 = white            $14 = bright_white
$15 = reset

User art file location (Windows only):
C:\Users\Public\BinaryFetch\BinaryArt.txt

The default art is now embedded directly in this source file
(kDefaultAsciiArt) rather than shipped as a separate .txt asset
or a Win32 RCDATA resource.
*/