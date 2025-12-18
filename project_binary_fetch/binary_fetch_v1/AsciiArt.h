#ifndef ASCIIART_H
#define ASCIIART_H

#include <string>
#include <vector>

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
   - Auto-creating the file from default template if missing
   - Keeping track of line widths
   - Reporting how tall and wide the art is
   - Providing safe access to art lines for real-time display

  This class does NOT print anything itself — LivePrinter
  handles actual on-screen printing.
*/
class AsciiArt {
public:
    AsciiArt();

    // Load ASCII art automatically from AppData location.
    // If file doesn't exist, copies from Default_Ascii_Art.txt
    // Returns true on success.
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

private:
    std::vector<std::string> artLines;     // the actual ASCII art lines
    std::vector<int> artWidths;            // precomputed widths for faster alignment
    int maxWidth;                          // longest visible width (used for padding)
    int height;                            // number of lines
    bool enabled;                          // toggle for showing/hiding the ASCII art
    int spacing;                           // spaces between art and info columns

    // Internal helper: Get the full path to user's ASCII art file
    std::string getUserArtPath() const;

    // Internal helper: Ensure directory exists
    bool ensureDirectoryExists(const std::string& path) const;

    // Internal helper: Copy default art to user location
    bool copyDefaultArt(const std::string& destPath) const;

    // Internal helper: Load art from a specific file path
    bool loadArtFromPath(const std::string& filepath);
};



/*
 ---------------------------------------------------------
                     LivePrinter Class
 ---------------------------------------------------------

  This component performs the "magic" of incremental,
  side-by-side printing.

  Every time the program pushes a line of system info,
  LivePrinter prints:
     [ASCII ART LINE] + padding + [INFO LINE]

  This allows the system info to appear gradually while the
  ASCII art stays on the left, properly aligned.
*/
class LivePrinter {
public:
    LivePrinter(const AsciiArt& artRef);

    // Push a single line of system info.
    // Each call prints the next art line (or blank padding).
    void push(const std::string& infoLine);

    // Same as push("") — convenient for spacing
    void pushBlank();

    // After finishing all info lines, print any remaining
    // ASCII art lines that weren't paired with info.
    void finish();

private:
    const AsciiArt& art;   // reference to the loaded ASCII art
    int index;             // which art line we are currently on

    // Core helper: prints the art line + spacing
    void printArtAndPad();
};



/*
 ---------------------------------------------------------
       Helper: Push multi-line formatted string
 ---------------------------------------------------------

  Some parts of the project generate multi-line text blocks
  (such as full CPU info or multiline network summaries).

  This helper splits a long string by newline characters
  and pushes each line individually to LivePrinter.
*/
void pushFormattedLines(LivePrinter& lp, const std::string& s);

#endif // ASCIIART_H