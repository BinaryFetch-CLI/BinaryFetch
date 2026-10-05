// PixelEncoders.cpp: Sixel, HalfBlock, Kitty, ITerm2 pixel encoders.
#include "PixelEncoders.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <sstream>
#include <vector>

namespace {

constexpr int PALETTE_SIZE = 256;
struct RGB { uint8_t r, g, b; };
using Palette = std::array<RGB, PALETTE_SIZE>;

// ---- Dithered palette: 6x7x6 = 252 colours. Index = (r*7+g)*6+b ----
Palette makeDitheredPalette() {
    Palette p{};
    int i = 0;
    for (int r = 0; r < 6; ++r)
        for (int g = 0; g < 7; ++g)
            for (int b = 0; b < 6; ++b)
                p[i++] = { (uint8_t)(r * 255 / 5), (uint8_t)(g * 255 / 6), (uint8_t)(b * 255 / 5) };
    return p;
}

// ---- Legacy palette: 6x6x6 cube (216) + 40-step gray ramp ----
Palette makeLegacyPalette() {
    Palette p{};
    int i = 0;
    for (int r = 0; r < 6; ++r)
        for (int g = 0; g < 6; ++g)
            for (int b = 0; b < 6; ++b)
                p[i++] = { (uint8_t)(r * 51), (uint8_t)(g * 51), (uint8_t)(b * 51) };
    for (int k = 216; k < 256; ++k) {
        int gray = static_cast<int>(((k - 216) * 255.0) / 39.0);
        p[k] = { (uint8_t)gray, (uint8_t)gray, (uint8_t)gray };
    }
    return p;
}

inline int colorDistance(const RGB& a, const RGB& b) {
    int dr = (int)a.r - b.r, dg = (int)a.g - b.g, db = (int)a.b - b.b;
    return dr * dr + dg * dg + db * db;
}

std::vector<uint8_t> buildLookup(const Palette& palette) {
    constexpr int SIZE = 32;
    std::vector<uint8_t> lookup(SIZE * SIZE * SIZE);
    for (int r = 0; r < SIZE; ++r)
        for (int g = 0; g < SIZE; ++g)
            for (int b = 0; b < SIZE; ++b) {
                RGB c{ (uint8_t)(r * 255 / 31), (uint8_t)(g * 255 / 31), (uint8_t)(b * 255 / 31) };
                int best = 0, bestD = INT32_MAX;
                for (int p = 0; p < PALETTE_SIZE; ++p) {
                    int d = colorDistance(c, palette[p]);
                    if (d < bestD) { bestD = d; best = p; }
                }
                lookup[(r * SIZE * SIZE) + (g * SIZE) + b] = (uint8_t)best;
            }
    return lookup;
}

// Built lazily, so a process that never uses a mode never pays for it.
const Palette& ditheredPalette() { static const Palette p = makeDitheredPalette(); return p; }
const Palette& legacyPalette()   { static const Palette p = makeLegacyPalette();   return p; }
const std::vector<uint8_t>& legacyLookup() {
    static const std::vector<uint8_t> l = buildLookup(legacyPalette());
    return l;
}

void writePalette(std::ostringstream& out, const Palette& palette) {
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

void encodeBand(std::ostringstream& out, const std::vector<uint8_t>& indexed,
                const std::vector<uint8_t>& opaque, int width, int height, int startY) {
    const int bandHeight = std::min(6, height - startY);
    std::vector<uint8_t> masks((size_t)PALETTE_SIZE * width, 0);
    std::array<bool, PALETTE_SIZE> used{};

    for (int x = 0; x < width; ++x)
        for (int dy = 0; dy < bandHeight; ++dy) {
            size_t idx = (size_t)(startY + dy) * width + x;
            if (!opaque[idx]) continue;
            uint8_t color = indexed[idx];
            masks[(size_t)color * width + x] |= (uint8_t)(1 << dy);
            used[color] = true;
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
            int run = 1;
            while (x + run <= last && row[x + run] == row[x]) ++run;
            writeRun(out, sixel, run);
            x += run;
        }
        out << '$';
    }
    out << '-';
}

bool encodeSixel(const unsigned char* pixels, int width, int height,
                 const EncodeOptions& opt, std::string& outEncoded) {
    const bool dithered = (opt.mode == SixelMode::Dithered);
    const Palette& palette = dithered ? ditheredPalette() : legacyPalette();
    const std::vector<uint8_t>* lookup = dithered ? nullptr : &legacyLookup();
    const float strength = std::clamp(opt.dither, 0.0f, 1.0f);

    static const uint8_t bayer[64] = {
         0, 32,  8, 40,  2, 34, 10, 42,
        48, 16, 56, 24, 50, 18, 58, 26,
        12, 44,  4, 36, 14, 46,  6, 38,
        60, 28, 52, 20, 62, 30, 54, 22,
         3, 35, 11, 43,  1, 33,  9, 41,
        51, 19, 59, 27, 49, 17, 57, 25,
        15, 47,  7, 39, 13, 45,  5, 37,
        63, 31, 55, 23, 61, 29, 53, 21
    };

    std::vector<uint8_t> indexed((size_t)width * height, 0);
    std::vector<uint8_t> opaque ((size_t)width * height, 0);

    for (int y = 0; y < height; ++y) {
        const unsigned char* row = pixels + (size_t)y * width * 4;
        for (int x = 0; x < width; ++x) {
            const unsigned char* p = row + (size_t)x * 4;
            if (p[3] < opt.alphaThreshold) continue;

            uint8_t idx;
            if (dithered) {
                float t = 0.5f + (((bayer[((y & 7) << 3) | (x & 7)] + 0.5f) / 64.0f) - 0.5f) * strength;
                int r = std::min(5, (int)(p[0] * (5.0f / 255.0f) + t));
                int g = std::min(6, (int)(p[1] * (6.0f / 255.0f) + t));
                int b = std::min(5, (int)(p[2] * (5.0f / 255.0f) + t));
                idx = (uint8_t)((r * 7 + g) * 6 + b);
            } else {
                idx = (*lookup)[((p[0] >> 3) * 32 * 32) + ((p[1] >> 3) * 32) + (p[2] >> 3)];
            }
            indexed[(size_t)y * width + x] = idx;
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

bool encodeHalfBlock(const unsigned char* pixels, int width, int height,
                     const EncodeOptions& opt, std::string& out) {
    std::ostringstream s;
    for (int y = 0; y < height; y += 2) {
        for (int x = 0; x < width; ++x) {
            const unsigned char* top = pixels + ((size_t)y * width + x) * 4;
            const unsigned char* bot = (y + 1 < height) ? pixels + ((size_t)(y + 1) * width + x) * 4 : nullptr;

            bool topOpaque = (top[3] >= opt.alphaThreshold);
            bool botOpaque = (bot && bot[3] >= opt.alphaThreshold);

            if (!topOpaque && !botOpaque) {
                s << "\033[0m ";
            } else if (topOpaque && !botOpaque) {
                s << "\033[0;38;2;" << (int)top[0] << ';' << (int)top[1] << ';' << (int)top[2] << "m▀";
            } else if (!topOpaque && botOpaque) {
                s << "\033[0;38;2;" << (int)bot[0] << ';' << (int)bot[1] << ';' << (int)bot[2] << "m▄";
            } else {
                s << "\033[0;38;2;" << (int)top[0] << ';' << (int)top[1] << ';' << (int)top[2]
                  << ";48;2;" << (int)bot[0] << ';' << (int)bot[1] << ';' << (int)bot[2] << "m▀";
            }
        }
        s << "\033[0m";
        if (y + 2 < height) s << "\033[B\033[" << width << 'D';
    }
    out = s.str();
    return true;
}

static std::string base64(const unsigned char* d, size_t n) {
    static const char* T = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string o;
    o.reserve((n + 2) / 3 * 4);
    for (size_t i = 0; i < n; i += 3) {
        unsigned v = d[i] << 16;
        if (i + 1 < n) v |= d[i + 1] << 8;
        if (i + 2 < n) v |= d[i + 2];
        o += T[(v >> 18) & 63];
        o += T[(v >> 12) & 63];
        o += (i + 1 < n) ? T[(v >> 6) & 63] : '=';
        o += (i + 2 < n) ? T[v & 63] : '=';
    }
    return o;
}

// Kitty: raw RGBA (f=32), chunked base64. q=2 silences replies so they
// don't land in stdin. C=1 keeps the cursor still. i=1 + the leading delete
// means each new frame replaces the previous one instead of stacking.
static bool encodeKitty(const unsigned char* px, int w, int h,
                        const EncodeOptions&, std::string& out) {
    std::string b64 = base64(px, (size_t)w * h * 4);
    std::ostringstream s;
    s << "\033_Ga=d,d=i,i=1,q=2\033\\";
    const size_t CHUNK = 4096;
    for (size_t pos = 0; pos < b64.size(); pos += CHUNK) {
        bool first = (pos == 0);
        bool last  = (pos + CHUNK >= b64.size());
        s << "\033_G";
        if (first) s << "a=T,f=32,s=" << w << ",v=" << h << ",i=1,q=2,C=1,";
        s << "m=" << (last ? 0 : 1) << ';' << b64.substr(pos, CHUNK) << "\033\\";
    }
    out = s.str();
    return true;
}

static void pngAppend(void* ctx, void* data, int size) {
    auto* v = static_cast<std::vector<unsigned char>*>(ctx);
    auto* p = static_cast<unsigned char*>(data);
    v->insert(v->end(), p, p + size);
}

// iTerm2: one escape sequence carrying a base64 PNG file.
static bool encodeITerm2(const unsigned char* px, int w, int h,
                         const EncodeOptions&, std::string& out) {
    std::vector<unsigned char> png;
    if (!stbi_write_png_to_func(pngAppend, &png, w, h, 4, px, w * 4)) return false;
    std::ostringstream s;
    s << "\033]1337;File=inline=1;width=" << w << "px;height=" << h
      << "px;preserveAspectRatio=0:" << base64(png.data(), png.size()) << '\a';
    out = s.str();
    return true;
}

} // namespace

bool encodePixels(const unsigned char* pixels, int width, int height,
                  const EncodeOptions& opt, std::string& outEncoded) {
    if (!pixels || width <= 0 || height <= 0) return false;

    switch (opt.protocol) {
        case GraphicsProtocol::HalfBlock: return encodeHalfBlock(pixels, width, height, opt, outEncoded);
        case GraphicsProtocol::Kitty:     return encodeKitty(pixels, width, height, opt, outEncoded);
        case GraphicsProtocol::ITerm2:    return encodeITerm2(pixels, width, height, opt, outEncoded);
        case GraphicsProtocol::Sixel:
        default:                          return encodeSixel(pixels, width, height, opt, outEncoded);
    }
}
