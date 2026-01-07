#include "DetailedScreen.h"
#include <windows.h>
#include <dxgi1_6.h>
#include <ShellScalingApi.h>
#include <SetupApi.h>
#include <cfgmgr32.h>
#include <cmath>
#include <cstdio>
#include <algorithm>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "Shcore.lib")
#pragma comment(lib, "SetupAPI.lib")
#pragma comment(lib, "cfgmgr32.lib")

DetailedScreen::DetailedScreen() {
    refresh();
}

bool DetailedScreen::refresh() {
    screens.clear();
    if (!populateFromDXGI()) return false;
    enrichWithEDID();
    enrichWithRegistry();
    enrichWithNVAPI();
    enrichWithADL();
    return !screens.empty();
}

// Enhanced EDID parser with extended information
DetailedScreen::ExtendedEDIDInfo DetailedScreen::parseExtendedEDID(const unsigned char* edid, size_t size) {
    ExtendedEDIDInfo info = {};
    info.valid = false;

    if (!edid || size < 128) return info;

    // Validate EDID header
    if (edid[0] != 0x00 || edid[1] != 0xFF || edid[7] != 0x00) return info;

    // Manufacturer ID (bytes 8-9)
    unsigned short mfgID = (edid[8] << 8) | edid[9];
    info.manufacturer = decodeManufacturerID(mfgID);

    // Serial number (bytes 12-15)
    unsigned int serial = (edid[15] << 24) | (edid[14] << 16) | (edid[13] << 8) | edid[12];
    if (serial != 0) {
        char serialBuf[32];
        snprintf(serialBuf, sizeof(serialBuf), "%u", serial);
        info.serialNumber = serialBuf;
    }

    // Manufacture date (bytes 16-17)
    info.manufactureWeek = edid[16];
    info.manufactureYear = 1990 + edid[17];

    // EDID version (bytes 18-19)
    char versionBuf[16];
    snprintf(versionBuf, sizeof(versionBuf), "%d.%d", edid[18], edid[19]);
    info.edidVersion = versionBuf;

    // Physical dimensions (bytes 21-22) in cm
    if (edid[21] > 0 && edid[22] > 0) {
        info.widthMM = edid[21] * 10.0f;
        info.heightMM = edid[22] * 10.0f;
    }

    // Native resolution from detailed timing descriptor (bytes 54-71)
    if (size >= 72) {
        unsigned short hActive = ((edid[58] >> 4) << 8) | edid[56];
        unsigned short vActive = ((edid[61] >> 4) << 8) | edid[59];

        if (hActive > 0 && vActive > 0) {
            info.nativeWidth = hActive;
            info.nativeHeight = vActive;

            // Pixel clock and refresh rate calculation
            unsigned short pixelClock = (edid[55] << 8) | edid[54]; // in 10kHz
            if (pixelClock > 0) {
                unsigned short hTotal = hActive + ((edid[58] & 0x0F) << 8) | edid[57];
                unsigned short vTotal = vActive + ((edid[61] & 0x0F) << 8) | edid[60];
                if (hTotal > 0 && vTotal > 0) {
                    info.maxRefreshRate = (pixelClock * 10000) / (hTotal * vTotal);
                }
            }
        }
    }

    // Monitor name from descriptor blocks
    for (int i = 54; i < 126; i += 18) {
        if (i + 17 >= size) break;
        if (edid[i] == 0x00 && edid[i + 1] == 0x00 && edid[i + 3] == 0xFC) {
            std::string name;
            for (int j = 5; j < 18; ++j) {
                if (edid[i + j] == 0x0A || edid[i + j] == 0x00) break;
                if (edid[i + j] >= 0x20 && edid[i + j] <= 0x7E) {
                    name += static_cast<char>(edid[i + j]);
                }
            }
            while (!name.empty() && name.back() == ' ') name.pop_back();
            if (!name.empty()) {
                info.friendlyName = name;
            }
            break;
        }
    }

    info.valid = (info.nativeWidth > 0 && info.nativeHeight > 0);
    return info;
}

// Decode 3-letter manufacturer code from EDID
std::string DetailedScreen::decodeManufacturerID(unsigned short id) {
    if (id == 0) return "Unknown";

    char mfg[4];
    mfg[0] = ((id >> 10) & 0x1F) + 'A' - 1;
    mfg[1] = ((id >> 5) & 0x1F) + 'A' - 1;
    mfg[2] = (id & 0x1F) + 'A' - 1;
    mfg[3] = '\0';

    return std::string(mfg);
}

// Calculate PPI from resolution and physical size
float DetailedScreen::calculatePPI(int width, int height, float diagonalInches) {
    if (diagonalInches <= 0) return 0.0f;
    float diagonalPixels = sqrtf(width * width + height * height);
    return diagonalPixels / diagonalInches;
}

// Calculate diagonal size from width and height in mm
float DetailedScreen::calculateDiagonal(float widthMM, float heightMM) {
    if (widthMM <= 0 || heightMM <= 0) return 0.0f;
    float diagonalMM = sqrtf(widthMM * widthMM + heightMM * heightMM);
    return diagonalMM / 25.4f; // Convert to inches
}

// Get connection type (HDMI, DisplayPort, etc.)
std::string DetailedScreen::getConnectionType(const std::wstring& deviceName) {
    // Try to determine from registry or device properties
    HKEY hKey;
    std::wstring regPath = L"SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e968-e325-11ce-bfc1-08002be10318}";

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, regPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        WCHAR subKeyName[256];
        DWORD index = 0;

        while (RegEnumKeyW(hKey, index++, subKeyName, 256) == ERROR_SUCCESS) {
            HKEY hSubKey;
            if (RegOpenKeyExW(hKey, subKeyName, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                WCHAR connType[128] = {};
                DWORD size = sizeof(connType);

                // Look for connection type hints in registry
                if (RegQueryValueExW(hSubKey, L"ConnectorType", nullptr, nullptr,
                    (BYTE*)connType, &size) == ERROR_SUCCESS) {
                    RegCloseKey(hSubKey);
                    RegCloseKey(hKey);

                    // Parse connector type value
                    if (wcsstr(connType, L"DisplayPort")) return "DisplayPort";
                    if (wcsstr(connType, L"HDMI")) return "HDMI";
                    if (wcsstr(connType, L"DVI")) return "DVI";
                    if (wcsstr(connType, L"USB-C")) return "USB-C";
                }
                RegCloseKey(hSubKey);
            }
        }
        RegCloseKey(hKey);
    }

    return "Unknown";
}

// Detect HDR capability
bool DetailedScreen::detectHDRCapability(const std::wstring& deviceName) {
    // Check Windows registry for HDR support
    HKEY hKey;
    std::wstring regPath = L"SYSTEM\\CurrentControlSet\\Control\\GraphicsDrivers\\Configuration";

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, regPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        WCHAR configKeyName[256];
        DWORD configIndex = 0;

        while (RegEnumKeyW(hKey, configIndex++, configKeyName, 256) == ERROR_SUCCESS) {
            HKEY hConfigKey;
            if (RegOpenKeyExW(hKey, configKeyName, 0, KEY_READ, &hConfigKey) == ERROR_SUCCESS) {
                HKEY h00;
                if (RegOpenKeyExW(hConfigKey, L"00", 0, KEY_READ, &h00) == ERROR_SUCCESS) {
                    DWORD hdrValue = 0;
                    DWORD size = sizeof(hdrValue);

                    if (RegQueryValueExW(h00, L"AdvancedColorEnabled", nullptr, nullptr,
                        (BYTE*)&hdrValue, &size) == ERROR_SUCCESS) {
                        RegCloseKey(h00);
                        RegCloseKey(hConfigKey);
                        RegCloseKey(hKey);
                        return (hdrValue != 0);
                    }
                    RegCloseKey(h00);
                }
                RegCloseKey(hConfigKey);
            }
        }
        RegCloseKey(hKey);
    }

    return false;
}

// Get bit depth
int DetailedScreen::getBitDepth(const std::wstring& deviceName) {
    HDC hdc = CreateDCW(L"DISPLAY", deviceName.c_str(), nullptr, nullptr);
    if (!hdc) return 8;

    int bitDepth = GetDeviceCaps(hdc, BITSPIXEL);
    DeleteDC(hdc);

    return bitDepth;
}

// Enhanced population from DXGI with all details
bool DetailedScreen::populateFromDXGI() {
    IDXGIFactory1* factory = nullptr;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) return false;

    IDXGIAdapter1* adapter = nullptr;
    for (UINT a = 0; factory->EnumAdapters1(a, &adapter) != DXGI_ERROR_NOT_FOUND; ++a) {
        IDXGIOutput* output = nullptr;

        for (UINT o = 0; adapter->EnumOutputs(o, &output) != DXGI_ERROR_NOT_FOUND; ++o) {
            IDXGIOutput6* output6 = nullptr;
            if (SUCCEEDED(output->QueryInterface(IID_PPV_ARGS(&output6)))) {
                DXGI_OUTPUT_DESC1 desc1{};
                if (SUCCEEDED(output6->GetDesc1(&desc1))) {
                    DetailedScreenInfo info = {};

                    // Device name and coordinates
                    info.deviceName = std::string(desc1.DeviceName, desc1.DeviceName + wcslen(desc1.DeviceName));
                    info.pos_x = desc1.DesktopCoordinates.left;
                    info.pos_y = desc1.DesktopCoordinates.top;
                    info.desktop_width = desc1.DesktopCoordinates.right - desc1.DesktopCoordinates.left;
                    info.desktop_height = desc1.DesktopCoordinates.bottom - desc1.DesktopCoordinates.top;

                    // Get current resolution
                    DEVMODEW dm{};
                    dm.dmSize = sizeof(dm);
                    if (EnumDisplaySettingsExW(desc1.DeviceName, ENUM_CURRENT_SETTINGS, &dm, 0)) {
                        info.current_width = dm.dmPelsWidth;
                        info.current_height = dm.dmPelsHeight;
                        info.refresh_rate = dm.dmDisplayFrequency;
                        info.rotation = dm.dmDisplayOrientation * 90;
                        info.bit_depth = dm.dmBitsPerPel;
                    }

                    // Check if primary
                    DEVMODEW primaryDM{};
                    primaryDM.dmSize = sizeof(primaryDM);
                    EnumDisplaySettingsExW(nullptr, ENUM_CURRENT_SETTINGS, &primaryDM, 0);
                    info.isPrimary = (desc1.DesktopCoordinates.left == 0 &&
                        desc1.DesktopCoordinates.top == 0);

                    // Get DPI scaling (using multiple methods from CompactScreen)
                    UINT dpiX = 96, dpiY = 96;
                    if (SUCCEEDED(GetDpiForMonitor(desc1.Monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY))) {
                        info.scale_percent = static_cast<int>(std::round((dpiX / 96.0f) * 100.0f));
                        info.raw_dpi_x = dpiX;
                        info.raw_dpi_y = dpiY;
                        info.scale_mul = scaleMultiplier(info.scale_percent);
                    }

                    // Get extended EDID information
                    std::wstring deviceNameW(desc1.DeviceName);
                    info.name = getFriendlyNameFromEDID(deviceNameW);

                    // Additional properties
                    info.connection_type = getConnectionType(deviceNameW);
                    info.hdr_capable = detectHDRCapability(deviceNameW);
                    info.color_format = "RGB"; // Default, can be enhanced

                    screens.push_back(info);
                }
                output6->Release();
            }
            output->Release();
        }
        adapter->Release();
    }

    factory->Release();
    return !screens.empty();
}

// Enrich with full EDID data
bool DetailedScreen::enrichWithEDID() {
    HKEY hKeyMonitors;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Enum\\DISPLAY",
        0, KEY_READ, &hKeyMonitors) != ERROR_SUCCESS) {
        return false;
    }

    for (auto& screen : screens) {
        WCHAR subKeyName[256];
        DWORD subKeyIndex = 0;

        while (RegEnumKeyW(hKeyMonitors, subKeyIndex++, subKeyName, 256) == ERROR_SUCCESS) {
            HKEY hKeyMonitor;
            if (RegOpenKeyExW(hKeyMonitors, subKeyName, 0, KEY_READ, &hKeyMonitor) == ERROR_SUCCESS) {
                WCHAR deviceKeyName[256];
                DWORD deviceKeyIndex = 0;

                while (RegEnumKeyW(hKeyMonitor, deviceKeyIndex++, deviceKeyName, 256) == ERROR_SUCCESS) {
                    HKEY hKeyDevice;
                    if (RegOpenKeyExW(hKeyMonitor, deviceKeyName, 0, KEY_READ, &hKeyDevice) == ERROR_SUCCESS) {
                        HKEY hKeyParams;
                        if (RegOpenKeyExW(hKeyDevice, L"Device Parameters", 0, KEY_READ, &hKeyParams) == ERROR_SUCCESS) {
                            BYTE edidData[256];
                            DWORD edidSize = sizeof(edidData);

                            if (RegQueryValueExW(hKeyParams, L"EDID", nullptr, nullptr,
                                edidData, &edidSize) == ERROR_SUCCESS) {
                                ExtendedEDIDInfo edidInfo = parseExtendedEDID(edidData, edidSize);

                                if (edidInfo.valid) {
                                    screen.native_width = edidInfo.nativeWidth;
                                    screen.native_height = edidInfo.nativeHeight;
                                    screen.width_mm = edidInfo.widthMM;
                                    screen.height_mm = edidInfo.heightMM;
                                    screen.manufacturer = edidInfo.manufacturer;
                                    screen.serial_number = edidInfo.serialNumber;
                                    screen.manufacture_year = edidInfo.manufactureYear;
                                    screen.manufacture_week = edidInfo.manufactureWeek;
                                    screen.edid_version = edidInfo.edidVersion;
                                    screen.max_refresh_rate = edidInfo.maxRefreshRate;

                                    // Calculate diagonal and PPI
                                    if (screen.width_mm > 0 && screen.height_mm > 0) {
                                        screen.diagonal_inches = calculateDiagonal(screen.width_mm, screen.height_mm);
                                        screen.ppi = calculatePPI(screen.native_width, screen.native_height,
                                            screen.diagonal_inches);
                                    }

                                    // Compute upscaling
                                    int upscaleFactor = computeUpscaleFactor(screen.current_width, screen.native_width);
                                    screen.has_upscaling = (upscaleFactor > 1);

                                    if (screen.has_upscaling) {
                                        char buf[16];
                                        snprintf(buf, sizeof(buf), "%dx", upscaleFactor);
                                        screen.upscale = buf;

                                        if (isNvidiaPresent()) {
                                            screen.upscale_technology = "DSR";
                                        }
                                        else if (isAMDPresent()) {
                                            screen.upscale_technology = "VSR";
                                        }
                                    }
                                    else {
                                        screen.upscale = "1x";
                                        screen.upscale_technology = "None";
                                    }
                                }
                            }
                            RegCloseKey(hKeyParams);
                        }
                        RegCloseKey(hKeyDevice);
                    }
                }
                RegCloseKey(hKeyMonitor);
            }
        }
    }

    RegCloseKey(hKeyMonitors);
    return true;
}

// Use implementations from CompactScreen
bool DetailedScreen::isNvidiaPresent() {
    DISPLAY_DEVICEW dd{};
    dd.cb = sizeof(dd);
    for (DWORD i = 0; EnumDisplayDevicesW(nullptr, i, &dd, 0); ++i) {
        if (dd.DeviceString && (wcsstr(dd.DeviceString, L"NVIDIA") ||
            wcsstr(dd.DeviceString, L"GeForce"))) {
            return true;
        }
    }
    return false;
}

bool DetailedScreen::isAMDPresent() {
    DISPLAY_DEVICEW dd{};
    dd.cb = sizeof(dd);
    for (DWORD i = 0; EnumDisplayDevicesW(nullptr, i, &dd, 0); ++i) {
        if (dd.DeviceString && (wcsstr(dd.DeviceString, L"AMD") ||
            wcsstr(dd.DeviceString, L"Radeon") || wcsstr(dd.DeviceString, L"ATI"))) {
            return true;
        }
    }
    return false;
}

std::string DetailedScreen::getGPUVendor() {
    if (isNvidiaPresent()) return "NVIDIA";
    if (isAMDPresent()) return "AMD";
    return "Unknown";
}

std::string DetailedScreen::scaleMultiplier(int scalePercent) {
    float mul = scalePercent / 100.0f;
    char buf[32];
    if (fabsf(mul - roundf(mul)) < 0.001f) {
        snprintf(buf, sizeof(buf), "%.0fx", mul);
    }
    else {
        snprintf(buf, sizeof(buf), "%.2fx", mul);
        std::string s(buf);
        size_t dot = s.find('.');
        if (dot != std::string::npos) {
            while (!s.empty() && s.back() == '0') s.pop_back();
            if (!s.empty() && s.back() == '.') s.pop_back();
            s += 'x';
            return s;
        }
    }
    return std::string(buf);
}

int DetailedScreen::computeUpscaleFactor(int currentWidth, int nativeWidth) {
    if (nativeWidth <= 0 || currentWidth <= 0) return 1;
    float ratio = static_cast<float>(currentWidth) / static_cast<float>(nativeWidth);
    if (ratio < 1.25f) return 1;
    return static_cast<int>(std::round(ratio));
}

// Implement other methods similarly...
bool DetailedScreen::enrichWithRegistry() { return true; }
bool DetailedScreen::enrichWithNVAPI() { return true; }
bool DetailedScreen::enrichWithADL() { return true; }
std::string DetailedScreen::getFriendlyNameFromEDID(const std::wstring& deviceName) {
    // Reuse implementation from CompactScreen
    return "Display";
}