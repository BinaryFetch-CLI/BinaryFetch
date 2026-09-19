#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Image.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <sstream>
#include <vector>
#include <iostream>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

// ============================================================
// TerminalImage — public API (neutral, no protocol names)
// ============================================================

TerminalImage::TerminalImage()
    : rowSpan(0), colSpan(0),
      paddingUp(0), paddingLeft(0), paddingRight(0),
      cellHeightPx(0), cellWidthPx(0), loaded(false) {}

void TerminalImage::setPadding(int up, int left, int right) {
    paddingUp = up; paddingLeft = left; paddingRight = right;
}
int  TerminalImage::getPaddingUp() const    { return paddingUp; }
int  TerminalImage::getPaddingLeft() const  { return paddingLeft; }
int  TerminalImage::getPaddingRight() const { return paddingRight; }
int  TerminalImage::getRowSpan() const      { return rowSpan; }
int  TerminalImage::getColSpan() const      { return colSpan; }
bool TerminalImage::isLoaded() const        { return loaded; }

void TerminalImage::setCellHeightPx(int px) { cellHeightPx = px; }
int  TerminalImage::getCellHeightPx() const { return cellHeightPx; }
void TerminalImage::setCellWidthPx(int px)  { cellWidthPx = px; }
int  TerminalImage::getCellWidthPx() const  { return cellWidthPx; }

// ============================================================
// Windows Sixel encoder — everything below is private to this file
// ============================================================

namespace {

constexpr int PALETTE_SIZE = 256;

// Alpha threshold below which a pixel is treated as fully transparent.
// Pixels at or above this are treated as fully opaque — no partial-alpha
// blending against an unknown terminal background. This threshold is used
// both for opaque-bounds detection and for the encoder's skip decision,
// so the crop and the emitted pixels always agree on what "visible" means.
constexpr uint8_t ALPHA_THRESHOLD = 128;

struct RGB { uint8_t r, g, b; };

// ---- In-band terminal query (XTWINOPS "CSI 16 t") ----
//
// Asks the terminal directly, over the real VT stream, for its cell size
// in pixels. Windows Terminal added support for this query (and its
// window-pixel-size sibling, CSI 14 t) starting in the 1.22 Preview
// release, and it answers with "CSI 6 ; height ; width t".
//
// This is tried before any Win32 API because Win32's console font/rect
// queries (GetCurrentConsoleFontEx, GetClientRect on GetConsoleWindow())
// only ever see the hidden, invisible conhost pseudo-console that ConPTY
// spins up behind Windows Terminal to satisfy legacy console APIs -- not
// the real, visible Windows Terminal window. Their numbers can therefore
// be completely disconnected from the actual rendered font/DPI, no matter
// how carefully those numbers are sanity-checked. Querying the terminal
// in-band sidesteps that translation layer entirely.
//
// Returns false (caller falls through to the Win32 probes below) if the
// terminal doesn't answer within ~200ms -- e.g. plain conhost, or a
// Windows Terminal build older than 1.22, neither of which understand
// this query.
bool queryCellSizeFromTerminal(int& outW, int& outH) {
    HANDLE hIn  = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE || hOut == INVALID_HANDLE_VALUE) return false;
    if (hIn == nullptr || hOut == nullptr) return false;

    DWORD oldMode = 0;
    if (!GetConsoleMode(hIn, &oldMode)) return false;

    // Raw VT input mode so the reply's escape bytes reach us directly
    // instead of being swallowed by the console's line editor.
    SetConsoleMode(hIn, ENABLE_VIRTUAL_TERMINAL_INPUT);
    FlushConsoleInputBuffer(hIn);

    const char* query = "\033[16t";
    DWORD written = 0;
    BOOL wrote = WriteFile(hOut, query, (DWORD)strlen(query), &written, nullptr);

    std::string response;
    if (wrote) {
        char buf[64];
        // Poll in short bursts for up to ~200ms total -- the reply can
        // arrive split across more than one ReadFile call.
        for (int tries = 0; tries < 10; ++tries) {
            if (WaitForSingleObject(hIn, 20) != WAIT_OBJECT_0) {
                if (!response.empty()) break;  // got partial data, then silence
                continue;                      // nothing yet, keep waiting
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

// ---- Terminal cell size ----
//
// Multi-strategy probe. Each strategy is sanity-checked before its result
// is accepted, and the first plausible answer wins.
//
// Strategy 0 asks the terminal directly (see queryCellSizeFromTerminal
// above) -- the only strategy that can see through ConPTY on Windows
// Terminal 1.22+. Strategies 1-4 are the pre-existing Win32 fallbacks for
// terminals that don't answer that query.
COORD getTerminalCellSize() {
    // Strategy 0 — ask the terminal itself via XTWINOPS.
    int qw = 0, qh = 0;
    if (queryCellSizeFromTerminal(qw, qh) &&
        qw >= 4 && qw <= 64 && qh >= 8 && qh <= 128) {
        return { (SHORT)qw, (SHORT)qh };
    }

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    // Strategy 1 — Win32 font metrics. Correct on plain conhost. Reject
    // implausible values (any dimension <4/8px or absurdly large) so a WT
    // placeholder like {1,1} or {0,0} is discarded rather than trusted.
    CONSOLE_FONT_INFOEX fi{};
    fi.cbSize = sizeof(fi);
    if (GetCurrentConsoleFontEx(hOut, FALSE, &fi)) {
        int w = fi.dwFontSize.X;
        int h = fi.dwFontSize.Y;
        if (w >= 4 && w <= 64 && h >= 8 && h <= 128) {
            return { (SHORT)w, (SHORT)h };
        }
    }

    // Strategy 2 — derive cell size from the console window's pixel
    // client rect divided by the character grid it currently displays.
    // Works on classic conhost; often returns real numbers under WT as
    // well because the ConPTY proxy window still reports a real client
    // rect. Same plausibility gates as above.
    HWND hwnd = GetConsoleWindow();
    if (hwnd) {
        RECT rc{};
        if (GetClientRect(hwnd, &rc)) {
            int pxW = rc.right - rc.left;
            int pxH = rc.bottom - rc.top;
            CONSOLE_SCREEN_BUFFER_INFO csbi{};
            if (pxW > 0 && pxH > 0 &&
                GetConsoleScreenBufferInfo(hOut, &csbi)) {
                int cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
                int rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
                if (cols > 0 && rows > 0) {
                    int cw = pxW / cols;
                    int ch = pxH / rows;
                    if (cw >= 4 && cw <= 64 && ch >= 8 && ch <= 128) {
                        return { (SHORT)cw, (SHORT)ch };
                    }
                }
            }
        }
    }

    // Strategy 3 — Try GetConsoleScreenBufferInfoEx, which under WT
    // sometimes reflects the actual raster cell size. Best-effort; if it
    // doesn't produce a plausible pair we simply fall through.
    {
        CONSOLE_SCREEN_BUFFER_INFOEX csbix{};
        csbix.cbSize = sizeof(csbix);
        if (GetConsoleScreenBufferInfoEx(hOut, &csbix)) {
            HWND h2 = GetConsoleWindow();
            if (h2) {
                RECT rc{};
                if (GetClientRect(h2, &rc)) {
                    int pxW = rc.right - rc.left;
                    int pxH = rc.bottom - rc.top;
                    int cols = csbix.srWindow.Right - csbix.srWindow.Left + 1;
                    int rows = csbix.srWindow.Bottom - csbix.srWindow.Top + 1;
                    if (cols > 0 && rows > 0 && pxW > 0 && pxH > 0) {
                        int cw = pxW / cols;
                        int ch = pxH / rows;
                        if (cw >= 4 && cw <= 64 && ch >= 8 && ch <= 128) {
                            return { (SHORT)cw, (SHORT)ch };
                        }
                    }
                }
            }
        }
    }

    // Strategy 4 — safe default. 8x16 is the classic Windows console cell
    // size at 100% DPI with the default raster font, and is close enough
    // that the reserved row count lands in the right ballpark even when
    // none of the probes above produced a trustworthy answer.
    return { 8, 16 };
}

// ---- 256-color palette: 6x6x6 cube + 40 grayscale steps ----
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

// ---- 32x32x32 RGB -> palette-index lookup table ----
std::vector<uint8_t> buildColorLookup(const std::array<RGB, PALETTE_SIZE>& palette) {
    constexpr int SIZE = 32;
    std::vector<uint8_t> lookup(SIZE * SIZE * SIZE);
    for (int r = 0; r < SIZE; ++r) {
        for (int g = 0; g < SIZE; ++g) {
            for (int b = 0; b < SIZE; ++b) {
                RGB color{ (uint8_t)(r * 255 / 31), (uint8_t)(g * 255 / 31), (uint8_t)(b * 255 / 31) };
                int bestIndex = 0, bestDistance = INT32_MAX;
                for (int p = 0; p < PALETTE_SIZE; ++p) {
                    int d = colorDistance(color, palette[p]);
                    if (d < bestDistance) { bestDistance = d; bestIndex = p; }
                }
                lookup[(r * SIZE * SIZE) + (g * SIZE) + b] = (uint8_t)bestIndex;
            }
        }
    }
    return lookup;
}

// Palette + lookup never change between images — compute once per
// process, not once per load(). This is the single biggest speed win
// available here (the lookup build is ~8M comparisons).
const std::array<RGB, PALETTE_SIZE>& getSharedPalette() {
    static const std::array<RGB, PALETTE_SIZE> palette = createPalette();
    return palette;
}
const std::vector<uint8_t>& getSharedLookup() {
    static const std::vector<uint8_t> lookup = buildColorLookup(getSharedPalette());
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

// `opaque` is a per-pixel 0/1 mask (alpha >= threshold). Pixels with
// opaque == 0 are skipped entirely: no bit set, no color marked used, so
// the terminal leaves its own background showing through.
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

// ---- Simple nearest-neighbor resize (RGBA) ----
std::vector<unsigned char> scalePixels(const unsigned char* src, int srcW, int srcH,
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

// ---- Opaque bounding box ----
//
// Scans an RGBA buffer for the smallest rectangle containing every pixel
// whose alpha is >= ALPHA_THRESHOLD. Returns false if no such pixel
// exists (fully transparent image), in which case the caller keeps the
// original canvas dimensions — a degenerate but non-crashing fallback.
//
// This is what makes padding_left/right measure from the visible edge of
// the image rather than from the edge of the PNG canvas. A transparent
// margin is invisible to the eye but still occupies raster columns as far
// as the terminal's cursor accounting is concerned, so without this crop
// an image with transparent borders would appear to have a gap between it
// and the info column even when padding is set to 0.
bool findOpaqueBounds(const unsigned char* rgba, int w, int h,
                       int& outX0, int& outY0, int& outX1, int& outY1) {
    int x0 = w, y0 = h, x1 = -1, y1 = -1;
    for (int y = 0; y < h; ++y) {
        const unsigned char* row = rgba + (size_t)y * w * 4;
        for (int x = 0; x < w; ++x) {
            if (row[(size_t)x * 4 + 3] >= ALPHA_THRESHOLD) {
                if (x < x0) x0 = x;
                if (x > x1) x1 = x;
                if (y < y0) y0 = y;
                if (y > y1) y1 = y;
            }
        }
    }
    if (x1 < x0 || y1 < y0) return false;
    outX0 = x0; outY0 = y0; outX1 = x1; outY1 = y1;
    return true;
}

// ---- Full encode: raw RGBA pixels -> complete Sixel escape sequence ----
bool encodePixelsForTerminal(const unsigned char* pixels, int width, int height,
                              std::string& outEncoded) {
    if (!pixels || width <= 0 || height <= 0) return false;

    const auto& palette = getSharedPalette();
    const auto& lookup  = getSharedLookup();

    std::vector<uint8_t> indexed((size_t)width * height, 0);
    std::vector<uint8_t> opaque ((size_t)width * height, 0);
    for (int y = 0; y < height; ++y) {
        const unsigned char* row = pixels + (size_t)y * width * 4;
        for (int x = 0; x < width; ++x) {
            const unsigned char* p = row + (size_t)x * 4;
            if (p[3] < ALPHA_THRESHOLD) continue;   // leave transparent
            indexed[(size_t)y * width + x] = getPaletteIndex(p[0], p[1], p[2], lookup);
            opaque [(size_t)y * width + x] = 1;
        }
    }

    std::ostringstream out;
    out << "\033P0;1;0q";                            // start sixel, transparent background
    out << '"' << "1;1;" << width << ';' << height;  // raster attributes: exact size
    writePalette(out, palette);
    for (int y = 0; y < height; y += 6)
        encodeBand(out, indexed, opaque, width, height, y);
    out << "\033\\";                                 // end sixel

    outEncoded = out.str();
    return true;
}

} // anonymous namespace

// ============================================================
// TerminalImage — private methods' backing implementation
// ============================================================

bool TerminalImage::load(const std::string& path, int sizePercent) {
    loaded = false;
    encodedData.clear();

    int origW = 0, origH = 0, channels = 0;

    // Force 4 channels so we always have alpha to work with. For RGB-only
    // sources stb fills alpha with 255, so the transparency handling below
    // is a no-op and non-transparent images behave exactly as before.
    unsigned char* pixels = stbi_load(path.c_str(), &origW, &origH, &channels, 4);
    if (!pixels) {
        std::cerr << "Warning: could not decode image: " << path
                   << " (" << stbi_failure_reason() << ")\n";
        return false;
    }

    // ---- Step 1: crop to the opaque bounding box ----
    //
    // Do this BEFORE the size scaling so that a scaled image has its
    // transparent margins removed proportionally, and so the final
    // colSpan/rowSpan describe the visible content, not the canvas.
    // If the image has no opaque pixels at all, fall through with the
    // full canvas — the encoder will simply emit nothing visible and the
    // reserved block will be empty, which is the correct outcome.
    int cropX = 0, cropY = 0, cropW = origW, cropH = origH;
    {
        int bx0, by0, bx1, by1;
        if (findOpaqueBounds(pixels, origW, origH, bx0, by0, bx1, by1)) {
            cropX = bx0;
            cropY = by0;
            cropW = bx1 - bx0 + 1;
            cropH = by1 - by0 + 1;
        }
    }

    // Copy the cropped sub-rectangle into a fresh tightly-packed buffer.
    // We do this unconditionally when the crop is smaller than the canvas,
    // so both the scaling path and the no-scaling path below see a buffer
    // whose rows are exactly cropW pixels wide.
    std::vector<unsigned char> cropped;
    const unsigned char* srcForEncode = pixels;
    int srcW = origW, srcH = origH;
    if (cropW != origW || cropH != origH) {
        cropped.resize((size_t)cropW * cropH * 4);
        for (int y = 0; y < cropH; ++y) {
            const unsigned char* s = pixels + ((size_t)(cropY + y) * origW + cropX) * 4;
            unsigned char* d = cropped.data() + (size_t)y * cropW * 4;
            std::memcpy(d, s, (size_t)cropW * 4);
        }
        srcForEncode = cropped.data();
        srcW = cropW;
        srcH = cropH;
    }

    // ---- Step 2: optional scaling ----
    int targetW = srcW, targetH = srcH;
    if (sizePercent > 0 && sizePercent != 100) {
        targetW = std::max(1, static_cast<int>(srcW * (sizePercent / 100.0)));
        targetH = std::max(1, static_cast<int>(srcH * (sizePercent / 100.0)));
    }

    bool ok;
    if (targetW != srcW || targetH != srcH) {
        std::vector<unsigned char> scaled = scalePixels(srcForEncode, srcW, srcH, targetW, targetH);
        ok = encodePixelsForTerminal(scaled.data(), targetW, targetH, encodedData);
    } else {
        ok = encodePixelsForTerminal(srcForEncode, srcW, srcH, encodedData);
    }
    stbi_image_free(pixels);
    if (!ok) return false;

    // ---- Step 3: compute the spans ----
    //
    // colSpan/rowSpan are computed from the CROPPED, scaled pixel size, so
    // they match what the terminal will actually raster — this is what
    // LivePrinter uses to decide where the info column starts. An explicit
    // cellWidthPx/cellHeightPx override (if set via JSON) always wins over
    // the queried/probed cell size, since it's the one value guaranteed to
    // be correct for this exact terminal/font/DPI combination.
    COORD cell = getTerminalCellSize();

    int effectiveCellW = (cellWidthPx > 0) ? cellWidthPx : cell.X;
    if (effectiveCellW <= 0) effectiveCellW = 8;    // paranoia
    colSpan = (targetW + effectiveCellW - 1) / effectiveCellW;

    int effectiveCellH = (cellHeightPx > 0) ? cellHeightPx : cell.Y;
    if (effectiveCellH <= 0) effectiveCellH = 16;   // paranoia
    rowSpan = (targetH + effectiveCellH - 1) / effectiveCellH;

    loaded = true;
    return true;
}

// Draws the already-encoded image at whatever the current cursor position
// is. Nothing more. No cursor querying, no absolute positioning, no
// remembering where it went afterwards — that used to live here, and it
// was the cause of the alignment bug: writing raw Sixel bytes (which are
// just runs of printable ASCII, 0x3F-0x7E) confuses the legacy console's
// own idea of "where is the cursor now" — especially under ConPTY/Windows
// Terminal — because it accounts for those bytes as if they were ordinary
// text, wrapping and scrolling its internal buffer accordingly. Querying
// that state back out afterwards (as the old render() did, indirectly, by
// caching an anchor and letting LivePrinter re-derive absolute rows from
// it) gives you a number that no longer matches where the image visually
// landed. LivePrinter now works around this the way fastfetch/neofetch do:
// it never asks "where is the cursor", it only ever moves the cursor by
// amounts it already knows (this image's own row/col span).
void TerminalImage::draw() const {
    if (!loaded) return;
    std::cout << encodedData;
    std::cout.flush();
}