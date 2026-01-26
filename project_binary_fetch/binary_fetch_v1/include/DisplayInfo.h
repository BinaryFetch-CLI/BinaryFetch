#pragma once

#include <string>
#include <vector>

class DisplayInfo {
public:
    struct ScreenInfo {
        std::string name;

        // Applied (what Windows is using)
        int current_width = 0;
        int current_height = 0;
        int refresh_rate = 0;

        // Native panel (from EDID)
        int native_width = 0;
        int native_height = 0;

        // Scaling
        int scale_percent = 100;
        std::string scale_mul; // "1x", "1.25x" etc.

        // Upscaling (DSR/VSR)
        std::string upscale; // "1x", "2x", ...
        bool dsr_enabled = false;    // true if DSR/VSR heuristically detected
        std::string dsr_type;        // "DSR", "VSR" or "None"

        // Extra
        std::string aspect_ratio;     // e.g., "16:9"
        std::string native_resolution; // e.g., "3840x2160"
    };

    DisplayInfo();
    bool refresh();
    const std::vector<ScreenInfo>& getScreens() const;

private:
    std::vector<ScreenInfo> screens;

    // Core population
    bool populateFromDXGI();
    bool enrichWithNVAPI();
    bool enrichWithADL();

    // Helpers (mirrors CompactScreen)
    static std::string WideToUtf8(const wchar_t* w);
    static std::string scaleMultiplier(int scalePercent);
    static int computeUpscaleFactor(int currentWidth, int nativeWidth);
    static std::string computeAspectRatio(int w, int h);

    bool isNvidiaPresent();
    bool isAMDPresent();

    struct EDIDInfo {
        std::string friendlyName;
        int nativeWidth;
        int nativeHeight;
        bool valid;
    };

    static EDIDInfo parseEDID(const unsigned char* edid, size_t size);
    std::string getFriendlyNameFromEDID(const std::wstring& deviceName);
};
