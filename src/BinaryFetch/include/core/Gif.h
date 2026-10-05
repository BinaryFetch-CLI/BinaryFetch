#pragma once
#include "PixelEncoders.h"
#include <string>
#include <vector>

// TerminalGif — Windows Sixel GIF player.
//
// Mirrors TerminalImage's contract (load/draw/getRowSpan/getColSpan/
// padding/cell-size overrides) but holds EVERY frame, pre-encoded, so
// playback never re-quantizes live. Unlike TerminalImage, frames are
// NOT individually cropped to their opaque bounds — all frames share
// one canvas size, because per-frame cropping would shift each frame's
// anchor point and make the animation visibly jitter/drift in place.
class TerminalGif {
public:
    TerminalGif();

    // Decodes every frame of the GIF at `path`, optionally scaled to
    // `sizePercent` of original size (100 = no scaling). Returns false
    // (isLoaded() == false) on any decode failure or if the file has
    // zero frames.
    bool load(const std::string& path, int sizePercent);

    // Draws frame `index` at the CURRENT cursor position. Same contract
    // as TerminalImage::draw() — caller handles all cursor positioning.
    void drawFrame(size_t index) const;

    bool isLoaded() const;
    size_t getFrameCount() const;
    int getFrameDelayMs(size_t index) const; // raw per-frame delay, ms

    int getRowSpan() const;
    int getColSpan() const;

    void setPadding(int up, int left, int right);
    int getPaddingUp() const;
    int getPaddingLeft() const;
    int getPaddingRight() const;

    void setCellHeightPx(int px);
    int getCellHeightPx() const;
    void setCellWidthPx(int px);
    void setEncodeOptions(const EncodeOptions& o) { encodeOpts = o; }
    int getCellWidthPx() const;

private:
    std::vector<std::string> frameEncoded;
    std::vector<int> frameDelaysMs;
    int rowSpan, colSpan;
    int paddingUp, paddingLeft, paddingRight;
    int cellHeightPx, cellWidthPx;
    bool loaded;
    EncodeOptions encodeOpts;
};