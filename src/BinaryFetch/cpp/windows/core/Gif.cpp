// Gif.cpp
// Windows-only. Decodes GIF frames via stb_image and encodes via PixelEncoders.

#include "Gif.h"
#include "PixelEncoders.h"
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
      cellHeightPx(0), cellWidthPx(0), loaded(false) {
    encodeOpts.mode = SixelMode::Legacy;
}

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
            encodePixels(scaled.data(), targetW, targetH, encodeOpts, encoded);
        } else {
            encodePixels(framePtr, width, height, encodeOpts, encoded);
        }

        frameEncoded.push_back(std::move(encoded));
        frameDelaysMs.push_back(delays[f]);
    }

    stbi_image_free(pixels);
    free(delays);

    COORD cell = getTerminalCellSizeGif();
    int effectiveCellW = (cellWidthPx > 0) ? cellWidthPx : cell.X;
    if (effectiveCellW <= 0) effectiveCellW = 8;
    colSpan = (encodeOpts.protocol == GraphicsProtocol::HalfBlock)
                ? targetW
                : (targetW + effectiveCellW - 1) / effectiveCellW;

    int effectiveCellH = (cellHeightPx > 0) ? cellHeightPx : cell.Y;
    if (effectiveCellH <= 0) effectiveCellH = 16;
    rowSpan = (encodeOpts.protocol == GraphicsProtocol::HalfBlock)
                ? (targetH + 1) / 2
                : (targetH + effectiveCellH - 1) / effectiveCellH;

    loaded = !frameEncoded.empty();
    return loaded;
}

void TerminalGif::drawFrame(size_t index) const {
    if (!loaded || index >= frameEncoded.size()) return;
    std::cout << frameEncoded[index];
    std::cout.flush();
}