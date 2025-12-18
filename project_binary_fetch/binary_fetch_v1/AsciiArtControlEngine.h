#pragma once
#include <filesystem>
#include <string>
#include <vector>

enum class AsciiGeneratorStatus {
    Success,               // File exists and is readable
    FileAlreadyExists,     // User file already present (same as Success semantically)
    CreatedFromDefault,    // Created new user file from bundled default
    MissingBundledDefault, // Default_Ascii_Art.txt missing in source/bundle
    IoError,               // Generic I/O error
    PermissionDenied,      // Cannot write to target location
};

class AsciiArtControlEngine {
public:
    AsciiArtControlEngine();
    ~AsciiArtControlEngine();

    // Ensure user ASCII file exists (creates directories and copies bundled default if needed).
    AsciiGeneratorStatus ensureUserAsciiExists(
        const std::filesystem::path& userPath,
        const std::filesystem::path& bundledDefaultPath);

    // Create the user ASCII file from bundled default (atomic write).
    AsciiGeneratorStatus createUserAsciiFromDefault(
        const std::filesystem::path& userPath,
        const std::filesystem::path& bundledDefaultPath);

    // Load ASCII art (reads file, sanitizes first line, computes visible widths)
    bool loadFromFile(const std::string& filename);

    // Query functions
    bool isEnabled() const;
    int getHeight() const;
    int getMaxWidth() const;
    const std::string& getLine(int index) const;
    int getLineWidth(int index) const;
    int getSpacing() const;
    void setSpacing(int s);
    void clear();

private:
    // Internal storage
    std::vector<std::string> artLines;
    std::vector<int> artWidths;
    int maxWidth;
    int height;
    bool enabled;
    int spacing;

    // Helpers
    AsciiGeneratorStatus atomicCopyFile(const std::filesystem::path& src, const std::filesystem::path& dst);
    static std::string stripAnsiSequences(const std::string& s);
    static std::wstring utf8_to_wstring(const std::string& s);
    static int char_display_width(wchar_t wc);
    static size_t visible_width(const std::string& s);
    static void sanitizeLeadingInvisible(std::string& s);
};

// LivePrinter: prints info lines aligned to ASCII art
class LivePrinter {
public:
    explicit LivePrinter(const AsciiArtControlEngine& artRef);

    // Push a single info line (printed next to current ASCII art line)
    void push(const std::string& infoLine);

    // Push a blank line (still respects ASCII art padding)
    void pushBlank();

    // Finish printing the rest of the ASCII art lines (if info finished early)
    void finish();

private:
    const AsciiArtControlEngine& art;
    int index; // current printed line index

    void printArtAndPad();
};

// Helper: push multi-line formatted string to LivePrinter
void pushFormattedLines(LivePrinter& lp, const std::string& s);
