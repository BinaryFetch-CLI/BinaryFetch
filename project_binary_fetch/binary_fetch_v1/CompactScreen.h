#pragma once

#include <string>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

struct ScreenInfo {
    std::string name;           // Friendly display name (e.g., "ASUS VG27AQ")
    int native_width;           // Actual panel resolution width
    int native_height;          // Actual panel resolution height
    int current_width;          // Current Windows desktop resolution width
    int current_height;         // Current Windows desktop resolution height
    int refresh_rate;           // Refresh rate in Hz
    int scale_percent;          // Windows DPI scaling (e.g., 100, 125, 150, 175, 200)
    std::string scale_mul;      // Formatted scale multiplier (e.g., "1x", "1.75x")
    std::string upscale;        // GPU upscale info (e.g., "Off", "2x (NVIDIA DSR)", "4x (AMD VSR)")
};

class CompactScreen {
public:
    CompactScreen();

    // Get all detected screens
    std::vector<ScreenInfo> getScreens() const { return screens; }

    // Check if NVIDIA GPU is present
    static bool isNvidiaPresent();

    // Check if AMD GPU is present
    static bool isAMDPresent();

    // Convert scale percentage to multiplier string (e.g., 150 -> "1.5x")
    static std::string scaleMultiplier(int scalePercent);

    // Compute upscale factor from current vs native resolution
    static int computeUpscaleFactor(int currentWidth, int nativeWidth);

    // Refresh screen information
    bool refresh();

private:
    std::vector<ScreenInfo> screens;

#if defined(_WIN32) || defined(_WIN64)
    // Windows-specific population methods
    bool populateFromDXGI();
    bool enrichWithNVAPI();
    bool enrichWithADL();
    std::string getFriendlyNameFromEDID(const std::wstring& deviceName);
#else
    // POSIX population methods
    bool populateFromXrandr();
    bool populateFromDRM();
#endif

    // Helper to parse EDID for native resolution and name
    struct EDIDInfo {
        std::string friendlyName;
        int nativeWidth;
        int nativeHeight;
        bool valid;
    };
    EDIDInfo parseEDID(const unsigned char* edid, size_t size);
};