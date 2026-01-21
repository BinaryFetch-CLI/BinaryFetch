#pragma once
#include <string>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

struct DetailedScreenInfo {
    // Basic Information
    std::string name;
    std::string deviceName;
    std::string deviceID;
    bool isPrimary;

    // Resolution Information
    int native_width;
    int native_height;
    int current_width;
    int current_height;
    int desktop_width;
    int desktop_height;

    // Display Properties
    int refresh_rate;
    int bit_depth;
    std::string color_format;

    // Scaling Information
    int scale_percent;
    std::string scale_mul;
    int raw_dpi_x;
    int raw_dpi_y;

    // GPU Upscaling
    std::string upscale;
    std::string upscale_technology;
    bool has_upscaling;

    // Physical Dimensions
    float diagonal_inches;
    float width_mm;
    float height_mm;
    float ppi;

    // Position & Orientation
    int pos_x;
    int pos_y;
    int rotation;

    // Technology & Features
    std::string panel_type;
    bool hdr_capable;
    bool g_sync;
    bool freesync;
    std::string connection_type;

    // EDID Information
    std::string manufacturer;
    std::string edid_version;
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

    // Utility methods - MAKE SURE THESE ARE ALL HERE
    static std::string scaleMultiplier(int scalePercent);
    static int computeUpscaleFactor(int currentWidth, int nativeWidth);
    static float calculatePPI(int width, int height, float diagonalInches);
    static float calculateDiagonal(float widthMM, float heightMM);
    static float calculateScreenSizeInches(float widthMM, float heightMM);  // <-- THIS ONE

private:
    std::vector<DetailedScreenInfo> screens;

#if defined(_WIN32) || defined(_WIN64)
    bool populateFromDXGI();
    bool enrichWithEDID();
    bool enrichWithRegistry();
    bool enrichWithNVAPI();
    bool enrichWithADL();

    struct ExtendedEDIDInfo {
        std::string friendlyName;
        int nativeWidth;
        int nativeHeight;
        float widthMM;
        float heightMM;
        std::string manufacturer;
        std::string edidVersion;
        bool valid;
    };

    ExtendedEDIDInfo parseExtendedEDID(const unsigned char* edid, size_t size);

    std::string getFriendlyNameFromEDID(const std::wstring& deviceName);
    std::string getConnectionType(const std::wstring& deviceName);
    bool detectHDRCapability(const std::wstring& deviceName);
    bool detectGSync(const std::wstring& deviceName);
    bool detectFreeSync(const std::wstring& deviceName);
    int getBitDepth(const std::wstring& deviceName);
    std::string getColorFormat(const std::wstring& deviceName);
    std::string getPanelType(const std::string& modelName);
    std::string decodeManufacturerID(unsigned short id);
#else
    bool populateFromXrandr();
    bool populateFromDRM();
#endif
};