// Gif.cpp
// Windows-only. Mirrors Image.cpp's private Sixel-encoding internals
// (duplicated here, not shared) so Image.cpp/Image.h remain untouched.

#include "Gif.h"
#include "stb_image.h"   // STB_IMAGE_IMPLEMENTATION already defined in Image.cpp

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

TerminalGif::TerminalGif()
    : rowSpan(0), colSpan(0),
      paddingUp(0), paddingLeft(0), paddingRight(0),
      cellHeightPx(0), cellWidthPx(0), loaded(false) {}

void TerminalGif::setPadding(int up, int left, int right) {
    paddingUp = up; paddingLeft = left; paddingRight = right;
}
int  TerminalGif::getPaddingUp() const    { return paddingUp; }
int  TerminalGif::getPaddingLeft() const  { return paddingLeft; }
int  TerminalGif::getPaddingRight() const { return paddingRight; }
int  TerminalGif::getRowSpan() const      { return rowSpan; }
int  TerminalGif::getColSpan() const      { return colSpan; }
bool TerminalGif::isLoaded() const        { return loaded; }
size_t TerminalGif::getFrameCount() const { return frameEncoded.size(); }

void TerminalGif::setCellHeightPx(int px) { cellHeightPx = px; }
int  TerminalGif::getCellHeightPx() const { return cellHeightPx; }
void TerminalGif::setCellWidthPx(int px)  { cellWidthPx = px; }
int  TerminalGif::getCellWidthPx() const  { return cellWidthPx; }

int TerminalGif::getFrameDelayMs(size_t index) const {
    if (index >= frameDelaysMs.size()) return 100; // sane default
    int d = frameDelaysMs[index];
    return (d > 0) ? d : 100; // some GIFs encode 0ms delays; treat as 100ms
}

namespace {

constexpr int PALETTE_SIZE = 256;
constexpr uint8_t ALPHA_THRESHOLD = 128;

struct RGB { uint8_t r, g, b; };

bool queryCellSizeFromTerminalGif(int& outW, int& outH) {
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
        sscanf(response.c_str() + pos, "[6;%d;%dt", &h, &w) == 2 &&
        h > 0 && w > 0) {
        outW = w;
        outH = h;
        return true;
    }
    return false;
}

COORD getTerminalCellSizeGif() {
    int qw = 0, qh = 0;
    if (queryCellSizeFromTerminalGif(qw, qh) &&
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

const std::array<RGB, PALETTE_SIZE>& getSharedPaletteGif() {
    static const std::array<RGB, PALETTE_SIZE> palette = createPalette();
    return palette;
}
const std::vector<uint8_t>& getSharedLookupGif() {
    static const std::vector<uint8_t> lookup = buildColorLookup(getSharedPaletteGif());
    return lookup;
}

inline uint8_t getPaletteIndex(uint8_t r, uint8_t g, uint8_t b, const std::vector<uint8_t>& lookup) {
    int rr = r >> 3, gg = g >> 3, bb = b >> 3;
    return lookup[(rr * 32 * 32) + (gg * 32) + bb];
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

void encodeBand(std::ostringstream& out,
                 const std::vector<uint8_t>& indexed,
                 const std::vector<uint8_t>& opaque,
                 int width, int height, int startY) {
    const int bandHeight = std::min(6, height - startY);
    std::vector<uint8_t> masks((size_t)PALETTE_SIZE * width, 0);
    std::array<bool, PALETTE_SIZE> used{};

    for (int x = 0; x < width; ++x) {
        for (int dy = 0; dy < bandHeight; ++dy) {
            int y = startY + dy;
            size_t idx = (size_t)y * width + x;
            if (!opaque[idx]) continue;
            uint8_t color = indexed[idx];
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

bool encodeOneFrame(const unsigned char* pixels, int width, int height, std::string& outEncoded) {
    if (!pixels || width <= 0 || height <= 0) return false;

    const auto& palette = getSharedPaletteGif();
    const auto& lookup  = getSharedLookupGif();

    std::vector<uint8_t> indexed((size_t)width * height, 0);
    std::vector<uint8_t> opaque ((size_t)width * height, 0);
    for (int y = 0; y < height; ++y) {
        const unsigned char* row = pixels + (size_t)y * width * 4;
        for (int x = 0; x < width; ++x) {
            const unsigned char* p = row + (size_t)x * 4;
            if (p[3] < ALPHA_THRESHOLD) continue;
            indexed[(size_t)y * width + x] = getPaletteIndex(p[0], p[1], p[2], lookup);
            opaque [(size_t)y * width + x] = 1;
        }
    }

    std::ostringstream out;
    out << "\033P0;1;0q";
    out << '"' << "1;1;" << width << ';' << height;
    writePalette(out, palette);
    for (int y = 0; y < height; y += 6)
        encodeBand(out, indexed, opaque, width, height, y);
    out << "\033\\";

    outEncoded = out.str();
    return true;
}

std::vector<unsigned char> scaleFrame(const unsigned char* src, int srcW, int srcH,
                                       int dstW, int dstH) {
    std::vector<unsigned char> dst((size_t)dstW * dstH * 4);
    for (int y = 0; y < dstH; ++y) {
        int srcY = y * srcH / dstH;
        for (int x = 0; x < dstW; ++x) {
            int srcX = x * srcW / dstW;
            const unsigned char* s = src + ((size_t)srcY * srcW + srcX) * 4;
            unsigned char* d = dst.data() + ((size_t)y * dstW + x) * 4;
            d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = s[3];
        }
    }
    return dst;
}

} // anonymous namespace

bool TerminalGif::load(const std::string& path, int sizePercent) {
    loaded = false;
    frameEncoded.clear();
    frameDelaysMs.clear();

    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Warning: could not open GIF: " << path << "\n";
        return false;
    }
    std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(file)),
                                        std::istreambuf_iterator<char>());
    if (buffer.empty()) return false;

    int width = 0, height = 0, frameCount = 0, channels = 0;
    int* delays = nullptr;

    unsigned char* pixels = stbi_load_gif_from_memory(
        buffer.data(), (int)buffer.size(),
        &delays, &width, &height, &frameCount, &channels, 4);

    if (!pixels || frameCount <= 0) {
        std::cerr << "Warning: could not decode GIF: " << path
                   << " (" << stbi_failure_reason() << ")\n";
        if (pixels) stbi_image_free(pixels);
        if (delays) free(delays);
        return false;
    }

    int targetW = width, targetH = height;
    if (sizePercent > 0 && sizePercent != 100) {
        targetW = std::max(1, static_cast<int>(width * (sizePercent / 100.0)));
        targetH = std::max(1, static_cast<int>(height * (sizePercent / 100.0)));
    }

    const size_t frameBytes = (size_t)width * height * 4;
    frameEncoded.reserve(frameCount);
    frameDelaysMs.reserve(frameCount);

    for (int f = 0; f < frameCount; ++f) {
        const unsigned char* framePtr = pixels + (size_t)f * frameBytes;
        std::string encoded;

        if (targetW != width || targetH != height) {
            std::vector<unsigned char> scaled = scaleFrame(framePtr, width, height, targetW, targetH);
            encodeOneFrame(scaled.data(), targetW, targetH, encoded);
        } else {
            encodeOneFrame(framePtr, width, height, encoded);
        }

        frameEncoded.push_back(std::move(encoded));
        frameDelaysMs.push_back(delays[f]);
    }

    stbi_image_free(pixels);
    free(delays);

    COORD cell = getTerminalCellSizeGif();
    int effectiveCellW = (cellWidthPx > 0) ? cellWidthPx : cell.X;
    if (effectiveCellW <= 0) effectiveCellW = 8;
    colSpan = (targetW + effectiveCellW - 1) / effectiveCellW;

    int effectiveCellH = (cellHeightPx > 0) ? cellHeightPx : cell.Y;
    if (effectiveCellH <= 0) effectiveCellH = 16;
    rowSpan = (targetH + effectiveCellH - 1) / effectiveCellH;

    loaded = !frameEncoded.empty();
    return loaded;
}

void TerminalGif::drawFrame(size_t index) const {
    if (!loaded || index >= frameEncoded.size()) return;
    std::cout << frameEncoded[index];
    std::cout.flush();
}