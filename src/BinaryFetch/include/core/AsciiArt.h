#ifndef ASCIIART_H
#define ASCIIART_H

#include <string>
#include <vector>
#include <map>

/*
 ---------------------------------------------------------
    AsciiArt Utilities — Helper Functions (Declarations)
 ---------------------------------------------------------

 These helpers deal with visual correctness when printing
 ASCII art beside system info lines. They handle tricky,
 behind-the-scenes problems like invisible ANSI sequences,
 UTF-8 character width, and trimming unexpected whitespace.
*/

// Removes ANSI color escape codes so width calculations stay correct
std::string stripAnsiSequences(const std::string& s);

// Converts UTF-8 text to wide string (needed for width calculation)
std::wstring utf8_to_wstring(const std::string& s);

// Returns how "wide" a Unicode character appears when printed.
// Some characters take 2 columns (Asian chars, emojis).
int char_display_width(wchar_t wc);

// Measures how many *visible* characters a UTF-8 string occupies.
size_t visible_width(const std::string& s);

// Some ASCII art lines may start with invisible ANSI codes.
// This trims them so alignment doesn't break.
void sanitizeLeadingInvisible(std::string& s);



/*
 ---------------------------------------------------------
                      AsciiArt Class
 ---------------------------------------------------------

  Responsible for:
   - Loading ASCII art from user's AppData folder
   - Auto-creating the file from the embedded default art if missing
   - Keeping track of line widths
   - Reporting how tall and wide the art is
   - Providing safe access to art lines for real-time display
   - Holding JSON-configurable padding (up/left/right) applied by
     the renderer

  This class does NOT print anything itself — LivePrinter
  (defined in main.cpp) handles actual on-screen printing.

  Windows-only.
*/
class AsciiArt {
public:
    AsciiArt();

    // Load ASCII art automatically from AppData location.
    // If file doesn't exist, writes it out from the embedded
    // default art baked into AsciiArt.cpp. Returns true on success.
    bool loadFromFile();

    // Advanced: Load from custom path (overrides default behavior)
    bool loadFromFile(const std::string& customPath);

    // Whether ASCII art printing is turned on
    bool isEnabled() const;
    void setEnabled(bool enable);

    // Clears current art lines & resets metadata
    void clear();

    // ------------ Runtime getters ------------
    int getHeight() const { return height; }               // how many art lines exist
    int getMaxWidth() const { return maxWidth; }           // longest visible line
    int getSpacing() const { return spacing; }             // padding between art & info
    const std::string& getLine(int i) const { return artLines[i]; }
    int getLineWidth(int i) const { return (i >= 0 && i < (int)artWidths.size()) ? artWidths[i] : 0; }

    // ------------ Padding (JSON-configurable) ------------
    // up:    blank lines printed before the art starts
    // left:  spaces printed before each art line
    // right: spaces printed after each art line, before the
    //        art->info spacing gap
    void setPadding(int up, int left, int right);
    int getPaddingUp() const { return paddingUp; }
    int getPaddingLeft() const { return paddingLeft; }
    int getPaddingRight() const { return paddingRight; }

    // ------------ $N color map (JSON-configurable) ------------
    // Injected from ConfigManager before loadFromFile() is called.
    // If never set (left empty), processColorCodes() falls back to
    // the same hardcoded 15-color default table AsciiArt always
    // shipped with — fully optional, fully backward-compatible.
    void setColorMap(const std::map<int, std::string>& map);

private:
    std::vector<std::string> artLines;     // the actual ASCII art lines
    std::vector<int> artWidths;            // precomputed widths for faster alignment
    int maxWidth;                          // longest visible width (used for padding)
    int height;                            // number of lines
    bool enabled;                          // toggle for showing/hiding the ASCII art
    int spacing;                           // spaces between art and info columns
    int paddingUp;                         // blank lines before the art starts
    int paddingLeft;                       // spaces before each art line
    int paddingRight;                      // spaces after each art line
    std::map<int, std::string> colorMap;   // $N -> ANSI escape, injected via setColorMap()

    // Internal helper: Get the full path to user's ASCII art file
    std::string getUserArtPath() const;

    // Internal helper: Ensure directory exists
    bool ensureDirectoryExists(const std::string& path) const;

    // Internal helper: Write the embedded default art to user location
    bool copyDefaultArt(const std::string& destPath) const;

    // Internal helper: Load art from a specific file path
    bool loadArtFromPath(const std::string& filepath);

    // Internal helper: Load art directly from the embedded default
    // string (fallback used when writing to disk fails)
    bool loadArtFromEmbedded();
};

#endif // ASCIIART_H