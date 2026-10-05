#pragma once
#include <string>

// Portable: no platform headers. Keep it that way so Linux can reuse it.

enum class GraphicsProtocol { Sixel, HalfBlock, Kitty, ITerm2 };

enum class SixelMode {
    Legacy,    // 6x6x6 cube + gray ramp, nearest-colour lookup, no dithering
    Dithered   // 6x7x6 cube + 8x8 Bayer ordered dithering
};

struct EncodeOptions {
    GraphicsProtocol protocol       = GraphicsProtocol::Sixel;
    SixelMode        mode           = SixelMode::Dithered;
    float            dither         = 1.0f;  // Dithered only: 1.0 full, 0.0 off
    unsigned char    alphaThreshold = 128;   // alpha below this = transparent
};

// RGBA in, complete encoded graphics escape sequence out.
bool encodePixels(const unsigned char* rgba, int width, int height,
                  const EncodeOptions& opt, std::string& out);
