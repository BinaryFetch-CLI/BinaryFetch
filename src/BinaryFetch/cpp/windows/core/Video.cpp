// Video.cpp
// Windows-only. Decodes via an ffmpeg pipe, Sixel-encodes frame by frame.

#include "Video.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <regex>
#include <sstream>
#include <vector>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace {

constexpr int PALETTE_SIZE = 256;
struct RGB { uint8_t r, g, b; };

// ---- cell size: ask the terminal (CSI 16 t), fall back to Win32 ----
bool queryCellSizeFromTerminalVideo(int& outW, int& outH) {
    HANDLE hIn  = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE || hOut == INVALID_HANDLE_VALUE) return false;
    if (hIn == nullptr || hOut == nullptr) return false;

    DWORD oldMode = 0;
    if (!GetConsoleMode(hIn, &oldMode)) return false;
    SetConsoleMode(hIn, ENABLE_VIRTUAL_TERMINAL_INPUT);
    FlushConsoleInputBuffer(hIn);

    const char* query = "\033[16t";
    DWORD written = 0;
    BOOL wrote = WriteFile(hOut, query, (DWORD)strlen(query), &written, nullptr);

    std::string response;
    if (wrote) {
        char buf[64];
        for (int tries = 0; tries < 10; ++tries) {
            if (WaitForSingleObject(hIn, 20) != WAIT_OBJECT_0) {
                if (!response.empty()) break;
                continue;
            }
            DWORD read = 0;
            if (!ReadFile(hIn, buf, sizeof(buf) - 1, &read, nullptr) || read == 0) break;
            buf[read] = '\0';
            response += buf;
            if (response.find('t') != std::string::npos) break;
        }
    }
    SetConsoleMode(hIn, oldMode);

    int h = 0, w = 0;
    size_t pos = response.find("[6;");
    if (pos != std::string::npos &&
        sscanf(response.c_str() + pos, "[6;%d;%dt", &h, &w) == 2 && h > 0 && w > 0) {
        outW = w; outH = h;
        return true;
    }
    return false;
}

COORD getTerminalCellSizeVideo() {
    int qw = 0, qh = 0;
    if (queryCellSizeFromTerminalVideo(qw, qh) &&
        qw >= 4 && qw <= 64 && qh >= 8 && qh <= 128) {
        return { (SHORT)qw, (SHORT)qh };
    }
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_FONT_INFOEX fi{};
    fi.cbSize = sizeof(fi);
    if (GetCurrentConsoleFontEx(hOut, FALSE, &fi)) {
        int w = fi.dwFontSize.X, h = fi.dwFontSize.Y;
        if (w >= 4 && w <= 64 && h >= 8 && h <= 128) return { (SHORT)w, (SHORT)h };
    }
    return { 8, 16 };
}

// ---- Sixel encoder (same palette scheme as Image.cpp / Gif.cpp) ----
std::array<RGB, PALETTE_SIZE> createPalette() {
    std::array<RGB, PALETTE_SIZE> palette{};
    int index = 0;
    for (int r = 0; r < 6; ++r)
        for (int g = 0; g < 6; ++g)
            for (int b = 0; b < 6; ++b)
                palette[index++] = { (uint8_t)(r * 51), (uint8_t)(g * 51), (uint8_t)(b * 51) };
    for (int i = 216; i < 256; ++i) {
        int gray = static_cast<int>(((i - 216) * 255.0) / 39.0);
        palette[i] = { (uint8_t)gray, (uint8_t)gray, (uint8_t)gray };
    }
    return palette;
}

inline int colorDistance(const RGB& a, const RGB& b) {
    int dr = (int)a.r - b.r, dg = (int)a.g - b.g, db = (int)a.b - b.b;
    return dr * dr + dg * dg + db * db;
}

std::vector<uint8_t> buildColorLookup(const std::array<RGB, PALETTE_SIZE>& palette) {
    constexpr int SIZE = 32;
    std::vector<uint8_t> lookup(SIZE * SIZE * SIZE);
    for (int r = 0; r < SIZE; ++r)
        for (int g = 0; g < SIZE; ++g)
            for (int b = 0; b < SIZE; ++b) {
                RGB color{ (uint8_t)(r * 255 / 31), (uint8_t)(g * 255 / 31), (uint8_t)(b * 255 / 31) };
                int bestIndex = 0, bestDistance = INT32_MAX;
                for (int p = 0; p < PALETTE_SIZE; ++p) {
                    int d = colorDistance(color, palette[p]);
                    if (d < bestDistance) { bestDistance = d; bestIndex = p; }
                }
                lookup[(r * SIZE * SIZE) + (g * SIZE) + b] = (uint8_t)bestIndex;
            }
    return lookup;
}

const std::array<RGB, PALETTE_SIZE>& getSharedPalette() {
    static const std::array<RGB, PALETTE_SIZE> palette = createPalette();
    return palette;
}
const std::vector<uint8_t>& getSharedLookup() {
    static const std::vector<uint8_t> lookup = buildColorLookup(getSharedPalette());
    return lookup;
}

inline uint8_t getPaletteIndex(uint8_t r, uint8_t g, uint8_t b, const std::vector<uint8_t>& lookup) {
    return lookup[((r >> 3) * 32 * 32) + ((g >> 3) * 32) + (b >> 3)];
}

void writePalette(std::ostringstream& out, const std::array<RGB, PALETTE_SIZE>& palette) {
    for (int i = 0; i < PALETTE_SIZE; ++i) {
        const RGB& c = palette[i];
        out << '#' << i << ";2;" << (c.r * 100 / 255) << ';' << (c.g * 100 / 255) << ';' << (c.b * 100 / 255);
    }
}

inline void writeRun(std::ostringstream& out, char value, int count) {
    if (count <= 0) return;
    if (count >= 4) out << '!' << count << value;
    else for (int i = 0; i < count; ++i) out << value;
}

// Video frames are fully opaque, so there is no alpha mask here.
void encodeBand(std::ostringstream& out, const std::vector<uint8_t>& indexed,
                int width, int height, int startY) {
    const int bandHeight = std::min(6, height - startY);
    std::vector<uint8_t> masks((size_t)PALETTE_SIZE * width, 0);
    std::array<bool, PALETTE_SIZE> used{};

    for (int x = 0; x < width; ++x) {
        for (int dy = 0; dy < bandHeight; ++dy) {
            uint8_t color = indexed[(size_t)(startY + dy) * width + x];
            masks[(size_t)color * width + x] |= (uint8_t)(1 << dy);
            used[color] = true;
        }
    }

    for (int color = 0; color < PALETTE_SIZE; ++color) {
        if (!used[color]) continue;
        out << '#' << color;
        const uint8_t* row = masks.data() + (size_t)color * width;

        int first = 0;
        while (first < width && row[first] == 0) ++first;
        int last = width - 1;
        while (last >= first && row[last] == 0) --last;
        if (first > last) continue;

        if (first > 0) writeRun(out, '?', first);
        int x = first;
        while (x <= last) {
            char sixel = (char)(63 + row[x]);
            int runLength = 1;
            while (x + runLength <= last && row[x + runLength] == row[x]) ++runLength;
            writeRun(out, sixel, runLength);
            x += runLength;
        }
        out << '$';
    }
    out << '-';
}

bool encodeFrame(const unsigned char* pixels, int width, int height, std::string& outEncoded) {
    if (!pixels || width <= 0 || height <= 0) return false;
    const auto& palette = getSharedPalette();
    const auto& lookup  = getSharedLookup();

    std::vector<uint8_t> indexed((size_t)width * height);
    for (size_t i = 0, n = (size_t)width * height; i < n; ++i) {
        const unsigned char* p = pixels + i * 4;
        indexed[i] = getPaletteIndex(p[0], p[1], p[2], lookup);
    }

    std::ostringstream out;
    out << "\033P0;1;0q";
    out << '"' << "1;1;" << width << ';' << height;
    writePalette(out, palette);
    for (int y = 0; y < height; y += 6)
        encodeBand(out, indexed, width, height, y);
    out << "\033\\";

    outEncoded = out.str();
    return true;
}

// ---- ffmpeg helpers ----
HANDLE openNul(SECURITY_ATTRIBUTES& sa) {
    return CreateFileA("NUL", GENERIC_READ | GENERIC_WRITE,
                       FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_EXISTING, 0, nullptr);
}

// Runs "ffmpeg -i file", reads its banner from stderr, parses "WxH".
bool probeVideoSize(const std::string& ffmpeg, const std::string& path, int& outW, int& outH) {
    SECURITY_ATTRIBUTES sa{ sizeof(sa), nullptr, TRUE };
    HANDLE rd = nullptr, wr = nullptr;
    if (!CreatePipe(&rd, &wr, &sa, 0)) return false;
    SetHandleInformation(rd, HANDLE_FLAG_INHERIT, 0);
    HANDLE nul = openNul(sa);

    STARTUPINFOA si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = nul;
    si.hStdOutput = wr;
    si.hStdError = wr;

    std::string cmd = "\"" + ffmpeg + "\" -hide_banner -nostdin -i \"" + path + "\"";
    std::vector<char> buf(cmd.begin(), cmd.end());
    buf.push_back('\0');

    PROCESS_INFORMATION pi{};
    BOOL ok = CreateProcessA(nullptr, buf.data(), nullptr, nullptr, TRUE,
                             CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
    CloseHandle(wr);
    if (nul != INVALID_HANDLE_VALUE) CloseHandle(nul);
    if (!ok) { CloseHandle(rd); return false; }
    CloseHandle(pi.hThread);

    std::string text;
    char chunk[4096];
    DWORD got = 0;
    while (ReadFile(rd, chunk, sizeof(chunk), &got, nullptr) && got > 0) text.append(chunk, got);
    WaitForSingleObject(pi.hProcess, 5000);
    CloseHandle(pi.hProcess);
    CloseHandle(rd);

    size_t v = text.find("Video:");
    if (v == std::string::npos) return false;
    size_t eol = text.find('\n', v);
    std::string line = text.substr(v, eol == std::string::npos ? std::string::npos : eol - v);

    static const std::regex re("\\s(\\d{2,5})x(\\d{2,5})[\\s,\\[]");
    std::smatch m;
    if (!std::regex_search(line, m, re)) return false;
    outW = std::stoi(m[1].str());
    outH = std::stoi(m[2].str());
    return outW > 0 && outH > 0;
}

} // anonymous namespace

// ============================================================
// TerminalVideo
// ============================================================

TerminalVideo::TerminalVideo()
    : rowSpan(0), colSpan(0), paddingUp(0), paddingLeft(0), paddingRight(0),
      cellHeightPx(0), cellWidthPx(0), fps(15), outW(0), outH(0), loaded(false),
      ffmpegPath("ffmpeg"), hProcess(nullptr), hPipe(nullptr) {}

TerminalVideo::~TerminalVideo() { close(); }

void TerminalVideo::setPadding(int up, int left, int right) {
    paddingUp = up; paddingLeft = left; paddingRight = right;
}
int  TerminalVideo::getPaddingUp() const    { return paddingUp; }
int  TerminalVideo::getPaddingLeft() const  { return paddingLeft; }
int  TerminalVideo::getPaddingRight() const { return paddingRight; }
int  TerminalVideo::getRowSpan() const      { return rowSpan; }
int  TerminalVideo::getColSpan() const      { return colSpan; }
bool TerminalVideo::isLoaded() const        { return loaded; }
void TerminalVideo::setCellHeightPx(int px) { cellHeightPx = px; }
void TerminalVideo::setCellWidthPx(int px)  { cellWidthPx = px; }
void TerminalVideo::setFps(int f)           { fps = std::clamp(f, 1, 60); }
int  TerminalVideo::getFps() const          { return fps; }
void TerminalVideo::setFfmpegPath(const std::string& p) { ffmpegPath = p.empty() ? "ffmpeg" : p; }

void TerminalVideo::close() {
    if (hProcess) {
        TerminateProcess((HANDLE)hProcess, 0);
        CloseHandle((HANDLE)hProcess);
        hProcess = nullptr;
    }
    if (hPipe) {
        CloseHandle((HANDLE)hPipe);
        hPipe = nullptr;
    }
}

bool TerminalVideo::startStream() {
    close();

    SECURITY_ATTRIBUTES sa{ sizeof(sa), nullptr, TRUE };
    HANDLE rd = nullptr, wr = nullptr;
    size_t frameBytes = rgba.size();
    DWORD pipeSize = (DWORD)std::min<size_t>(frameBytes * 4, (size_t)1 << 26);
    if (!CreatePipe(&rd, &wr, &sa, pipeSize)) return false;
    SetHandleInformation(rd, HANDLE_FLAG_INHERIT, 0);
    HANDLE nul = openNul(sa);

    STARTUPINFOA si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = nul;
    si.hStdOutput = wr;
    si.hStdError = nul;

    std::string cmd = "\"" + ffmpegPath + "\" -nostdin -loglevel quiet -i \"" + videoPath +
                      "\" -an -sn -vf \"fps=" + std::to_string(fps) +
                      ",scale=" + std::to_string(outW) + ":" + std::to_string(outH) +
                      "\" -f rawvideo -pix_fmt rgba -";
    std::vector<char> buf(cmd.begin(), cmd.end());
    buf.push_back('\0');

    PROCESS_INFORMATION pi{};
    BOOL ok = CreateProcessA(nullptr, buf.data(), nullptr, nullptr, TRUE,
                             CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
    CloseHandle(wr);
    if (nul != INVALID_HANDLE_VALUE) CloseHandle(nul);
    if (!ok) { CloseHandle(rd); return false; }

    CloseHandle(pi.hThread);
    hProcess = pi.hProcess;
    hPipe = rd;
    return true;
}

bool TerminalVideo::load(const std::string& path, int sizePercent) {
    loaded = false;
    close();
    videoPath = path;

    int srcW = 0, srcH = 0;
    if (!probeVideoSize(ffmpegPath, path, srcW, srcH)) {
        std::cerr << "Warning: could not read video (is ffmpeg installed, or is "
                     "Video.ffmpeg_path set?): " << path << "\n";
        return false;
    }

    outW = srcW; outH = srcH;
    if (sizePercent > 0 && sizePercent != 100) {
        outW = std::max(1, (int)(srcW * (sizePercent / 100.0)));
        outH = std::max(1, (int)(srcH * (sizePercent / 100.0)));
    }

    rgba.assign((size_t)outW * outH * 4, 0);
    if (!startStream()) {
        std::cerr << "Warning: could not start ffmpeg.\n";
        return false;
    }

    COORD cell = getTerminalCellSizeVideo();
    int cw = (cellWidthPx > 0) ? cellWidthPx : cell.X;
    if (cw <= 0) cw = 8;
    int ch = (cellHeightPx > 0) ? cellHeightPx : cell.Y;
    if (ch <= 0) ch = 16;
    colSpan = (outW + cw - 1) / cw;
    rowSpan = (outH + ch - 1) / ch;

    loaded = true;
    return true;
}

bool TerminalVideo::nextFrame(std::string& out) {
    if (!hPipe) return false;
    size_t total = 0;
    while (total < rgba.size()) {
        DWORD got = 0;
        DWORD want = (DWORD)std::min<size_t>(rgba.size() - total, (size_t)1 << 20);
        if (!ReadFile((HANDLE)hPipe, rgba.data() + total, want, &got, nullptr) || got == 0)
            return false;   // end of video (or ffmpeg died)
        total += got;
    }
    return encodeFrame(rgba.data(), outW, outH, out);
}

bool TerminalVideo::restart() {
    return startStream();
}