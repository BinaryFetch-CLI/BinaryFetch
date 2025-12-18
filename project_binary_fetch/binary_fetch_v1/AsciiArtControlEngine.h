#pragma once
#include <vector>
#include <string>

/**
 * AsciiArtControlEngine loads and holds ASCII art lines for rendering.
 * It ensures the user ASCII art file exists (creating it from a default if missing),
 * and strips BOM and ANSI codes while calculating display widths.
 */
class AsciiArtControlEngine {
public:
    /** Loads ASCII art from the user file (creating it if needed). */
    bool loadAsciiArt();

    /** Returns the loaded lines (after BOM/ANSI removal). */
    const std::vector<std::string>& getLines() const { return lines; }

    /** Returns the maximum display width (in columns) among all lines. */
    int getMaxWidth() const { return maxWidth; }

    /** Returns the number of lines of ASCII art. */
    int getHeight() const { return static_cast<int>(lines.size()); }

    /** Returns the display width (in columns) of a given line index. */
    int getLineWidth(int index) const {
        if (index < 0 || index >= (int)lineWidths.size()) return 0;
        return lineWidths[index];
    }

    /** Returns the left indent (spacing) removed from all lines. */
    int getSpacing() const { return spacing; }

private:
    std::vector<std::string> lines;   // Loaded ASCII art lines
    std::vector<int> lineWidths;      // Display width of each line
    int maxWidth = 0;                 // Maximum line width
    int spacing = 0;                 // Common leading-space indent removed from lines
};
