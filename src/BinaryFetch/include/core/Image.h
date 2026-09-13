#pragma once
#include <string>

// TerminalImage — platform-neutral image-to-terminal-graphics bridge.
//
// Responsibility is intentionally narrow: decode an image file, encode it
// into whatever terminal-graphics protocol the platform backend uses, and
// report how many terminal rows/columns the result will occupy once drawn.
//
// TerminalImage does NOT know or care where on screen it ends up — it never
// queries or sets the cursor position. Placement (padding, sitting beside
// info text, etc.) is entirely LivePrinter's job, in main.cpp — the same
// way LivePrinter, not AsciiArt, already owns placement for ASCII-art mode.
//
// This is a deliberate change from the first working version: that version
// had TerminalImage query the console for "where is the cursor right now"
// both before AND relied on that value staying valid after printing raw
// image bytes. It doesn't stay valid (see LivePrinter's comments in
// main.cpp for why) — so that responsibility, and the anchorRow/anchorCol
// state that went with it, has been removed from this class entirely.
//
// Configuration note: everything this class needs in order to report an
// accurate row span and to leave zero extra horizontal space when padding
// is zero is derived internally. No new JSON keys are required for the
// default behaviour to be correct -- the two cell-size overrides below are
// opt-in and only needed if auto-detection doesn't match your terminal.
class TerminalImage {
public:
    TerminalImage();

    // Decodes `path`, optionally scales to `sizePercent` of its original
    // size (100 = no scaling), and encodes it for the terminal. Returns
    // false (and leaves isLoaded() == false) on any decode failure.
    //
    // If the source image has an alpha channel, the image is first cropped
    // to the bounding box of its opaque pixels, and pixels below the
    // transparency threshold inside that box are omitted from the emitted
    // graphics stream so the terminal's own background shows through them.
    //
    // The crop matters for placement, not just for looks: a transparent
    // margin on the canvas is still part of the raster the terminal sees,
    // so an uncropped image would reserve columns for pixels that render
    // nothing — putting an invisible gap between the visible edge of the
    // image and the info column even when padding_left/right are 0. After
    // cropping, padding_left/right measure from the visible content.
    bool load(const std::string& path, int sizePercent);

    // Writes the already-encoded image at the CURRENT cursor position and
    // nowhere else. Does not move the cursor before drawing, and makes no
    // assumption about where the cursor ends up afterwards — the caller
    // (LivePrinter) accounts for that using only relative motion.
    void draw() const;

    bool isLoaded() const;

    // Size of the drawn image, in whole terminal rows/columns (rounded up),
    // computed from the real terminal cell size at load() time, AFTER the
    // opaque-bounds crop above. These are therefore the dimensions of the
    // visible content, not of the original canvas.
    int getRowSpan() const;
    int getColSpan() const;

    // JSON-configurable padding, applied by LivePrinter — same convention
    // as AsciiArt::setPadding. LivePrinter already reads these from the
    // existing Image.padding_up/left/right keys; nothing new here.
    //
    // Because colSpan now refers to the visible content (post-crop), a
    // padding of 0 means the info column starts at the first terminal cell
    // at or after the rightmost opaque pixel, with no hidden slack.
    void setPadding(int up, int left, int right);
    int getPaddingUp() const;
    int getPaddingLeft() const;
    int getPaddingRight() const;

    // Optional override for the terminal cell HEIGHT (pixels per row)
    // used to convert the decoded image's pixel height into a row span.
    //
    //   0 (default) = auto-detect: an in-band XTWINOPS query first, then a
    //                 multi-strategy Win32 probe if that goes unanswered
    //   >0          = use exactly this many pixels per row, no detection
    //
    // Under Windows Terminal / ConPTY, GetCurrentConsoleFontEx sometimes
    // returns placeholder font metrics that have nothing to do with the
    // pixels being rendered (this is what produced the "image reserves
    // many times its real height" bug), and even the newer in-band query
    // only works on Windows Terminal 1.22+ and depends on the terminal
    // answering promptly. This override is the reliable fallback: a value
    // the user measures once for their terminal/font/DPI setup and pins
    // in JSON, the same way tools like fastfetch require an explicit
    // width/height for sixel logos on Windows rather than trying to
    // auto-derive it.
    void setCellHeightPx(int px);
    int getCellHeightPx() const;

    // Optional override for the terminal cell WIDTH (pixels per column),
    // same convention and same rationale as setCellHeightPx above, but
    // feeding colSpan instead of rowSpan.
    void setCellWidthPx(int px);
    int getCellWidthPx() const;

private:
    std::string encodedData;
    int rowSpan;
    int colSpan;
    int paddingUp;
    int paddingLeft;
    int paddingRight;
    int cellHeightPx;   // 0 = auto
    int cellWidthPx;    // 0 = auto
    bool loaded;
};