#pragma once
#include <string>
#include <vector>
#include <windows.h>

struct DetailedScreenInfo {
    // Basic Information
    std::string name;                  // Friendly display name from EDID
    std::string deviceName;            // Windows device name (e.g., \\.\DISPLAY1)
    std::string deviceID;              // Hardware ID (e.g., MONITOR\DEL4067\...)
    bool isPrimary;                    // Is this the primary display?

    // Resolution Information
    int native_width;                  // Physical panel resolution width
    int native_height;                 // Physical panel resolution height
    int current_width;                 // Current Windows resolution width
    int current_height;                // Current Windows resolution height
    int desktop_width;                 // Desktop coordinate space width
    int desktop_height;                // Desktop coordinate space height

    // Display Properties
    int refresh_rate;                  // Current refresh rate in Hz
    int max_refresh_rate;              // Maximum supported refresh rate
    int bit_depth;                     // Color bit depth (8, 10, 12, etc.)
    std::string color_format;          // RGB, YCbCr444, YCbCr422, etc.

    // Scaling Information
    int scale_percent;                 // Windows DPI scaling percentage
    std::string scale_mul;             // Formatted multiplier (e.g., "1.75x")
    int raw_dpi_x;                     // Raw DPI value X
    int raw_dpi_y;                     // Raw DPI value Y

    // GPU Upscaling
    std::string upscale;               // Upscale factor (e.g., "4x")
    std::string upscale_technology;    // DSR, VSR, or None
    bool has_upscaling;                // Is upscaling active?

    // Physical Dimensions
    float diagonal_inches;             // Screen size in inches
    float width_mm;                    // Physical width in mm
    float height_mm;                   // Physical height in mm
    float ppi;                         // Pixels per inch

    // Position & Orientation
    int pos_x;                         // Desktop X coordinate
    int pos_y;                         // Desktop Y coordinate
    int rotation;                      // 0, 90, 180, 270 degrees

    // Technology & Features
    std::string panel_type;            // IPS, TN, VA, OLED, etc.
    bool hdr_capable;                  // HDR support
    bool g_sync;                       // G-SYNC support
    bool freesync;                     // FreeSync support
    std::string connection_type;       // HDMI, DisplayPort, DVI, etc.

    // EDID Information
    std::string manufacturer;          // Manufacturer ID
    std::string serial_number;         // Serial number
    int manufacture_year;              // Year of manufacture
    int manufacture_week;              // Week of manufacture
    std::string edid_version;          // EDID version
};

class DetailedScreen {
public:
    DetailedScreen();

    // Main methods
    std::vector<DetailedScreenInfo> getScreens() const { return screens; }
    bool refresh();

    // GPU Detection
    static bool isNvidiaPresent();
    static bool isAMDPresent();
    static std::string getGPUVendor();

    // Utility methods
    static std::string scaleMultiplier(int scalePercent);
    static int computeUpscaleFactor(int currentWidth, int nativeWidth);
    static float calculatePPI(int width, int height, float diagonalInches);
    static float calculateDiagonal(float widthMM, float heightMM);

private:
    std::vector<DetailedScreenInfo> screens;

    // Core population methods
    bool populateFromDXGI();
    bool enrichWithEDID();
    bool enrichWithRegistry();
    bool enrichWithNVAPI();
    bool enrichWithADL();

    // Enhanced EDID parsing
    struct ExtendedEDIDInfo {
        std::string friendlyName;
        int nativeWidth;
        int nativeHeight;
        float widthMM;
        float heightMM;
        std::string manufacturer;
        std::string serialNumber;
        int manufactureYear;
        int manufactureWeek;
        std::string edidVersion;
        int maxRefreshRate;
        bool valid;
    };

    ExtendedEDIDInfo parseExtendedEDID(const unsigned char* edid, size_t size);

    // Helper methods
    std::string getFriendlyNameFromEDID(const std::wstring& deviceName);
    std::string getConnectionType(const std::wstring& deviceName);
    bool detectHDRCapability(const std::wstring& deviceName);
    bool detectGSync(const std::wstring& deviceName);
    bool detectFreeSync(const std::wstring& deviceName);
    int getBitDepth(const std::wstring& deviceName);
    std::string getColorFormat(const std::wstring& deviceName);
    std::string getPanelType(const std::string& modelName);

    // Manufacturer ID decoder
    std::string decodeManufacturerID(unsigned short id);
};