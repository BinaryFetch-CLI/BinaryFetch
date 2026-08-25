#pragma once

#include <string>
#include <vector>
using namespace std;
class DisplayInfo {
public:
    struct ScreenInfo {
        string name;

        // Applied (what Windows is using)
        int current_width = 0;
        int current_height = 0;
        int refresh_rate = 0;

        // Native panel (from EDID)
        int native_width = 0;
        int native_height = 0;

        // Scaling
        int scale_percent = 100;
        string scale_mul; // "1x", "1.25x" etc.

        // Upscaling (DSR/VSR)
        string upscale; // "1x", "2x", ...
        bool dsr_enabled = false;    // true if DSR/VSR heuristically detected
        string dsr_type;        // "DSR", "VSR" or "None"

        // Extra
        string aspect_ratio;     // e.g., "16:9"
        string native_resolution; // e.g., "3840x2160"
    };

    DisplayInfo();
    bool refresh();
    const vector<ScreenInfo>& getScreens() const;

private:
    vector<ScreenInfo> screens;

    // Core population
    bool populateFromDXGI();
    bool enrichWithNVAPI();
    bool enrichWithADL();

    // Helpers (mirrors CompactScreen)
    static string WideToUtf8(const wchar_t* w);
    static string scaleMultiplier(int scalePercent);
    static int computeUpscaleFactor(int currentWidth, int nativeWidth);
    static string computeAspectRatio(int w, int h);

    bool isNvidiaPresent();
    bool isAMDPresent();

    struct EDIDInfo {
        string friendlyName;
        int nativeWidth;
        int nativeHeight;
        bool valid;
    };

    static EDIDInfo parseEDID(const unsigned char* edid, size_t size);
    string getFriendlyNameFromEDID(const wstring& deviceName);
};
