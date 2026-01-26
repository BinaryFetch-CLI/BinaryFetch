#include "DisplayInfo.h"

#include <windows.h>
#include <dxgi1_6.h>
#include <ShellScalingApi.h>
#include <SetupApi.h>
#include <cfgmgr32.h>
#include <string>
#include <vector>
#include <cmath>
#include <cstdio>
#include <algorithm>
#include <cwctype>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "Shcore.lib")
#pragma comment(lib, "SetupAPI.lib")
#pragma comment(lib, "cfgmgr32.lib")

// ----------------- Utilities (mirrors CompactScreen) -----------------

std::string DisplayInfo::WideToUtf8(const wchar_t* w) {
    if (!w) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) return {};
    std::string s(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w, -1, &s[0], len, nullptr, nullptr);
    if (!s.empty() && s.back() == '\0') s.pop_back();
    return s;
}

std::string DisplayInfo::scaleMultiplier(int scalePercent) {
    float mul = scalePercent / 100.0f;
    char buf[32];
    if (fabsf(mul - roundf(mul)) < 0.001f) {
        snprintf(buf, sizeof(buf), "%.0fx", mul);
        return std::string(buf);
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
        return s;
    }
}

int DisplayInfo::computeUpscaleFactor(int currentWidth, int nativeWidth) {
    if (nativeWidth <= 0 || currentWidth <= 0) return 1;
    float ratio = static_cast<float>(currentWidth) / static_cast<float>(nativeWidth);
    if (ratio < 1.25f) return 1;
    return static_cast<int>(std::round(ratio));
}

static int gcd_int(int a, int b) {
    if (a <= 0 || b <= 0) return 1;
    while (b != 0) {
        int t = a % b;
        a = b;
        b = t;
    }
    return a;
}

std::string DisplayInfo::computeAspectRatio(int w, int h) {
    if (w <= 0 || h <= 0) return "Unknown";
    int g = gcd_int(w, h);
    int aw = w / g;
    int ah = h / g;
    return std::to_string(aw) + ":" + std::to_string(ah);
}

bool DisplayInfo::isNvidiaPresent() {
    DISPLAY_DEVICEW dd{};
    dd.cb = sizeof(dd);
    for (DWORD i = 0; EnumDisplayDevicesW(nullptr, i, &dd, 0); ++i) {
        if (dd.DeviceString && (wcsstr(dd.DeviceString, L"NVIDIA") || wcsstr(dd.DeviceString, L"GeForce"))) {
            return true;
        }
    }
    return false;
}

bool DisplayInfo::isAMDPresent() {
    DISPLAY_DEVICEW dd{};
    dd.cb = sizeof(dd);
    for (DWORD i = 0; EnumDisplayDevicesW(nullptr, i, &dd, 0); ++i) {
        if (dd.DeviceString && (wcsstr(dd.DeviceString, L"AMD") || wcsstr(dd.DeviceString, L"Radeon") || wcsstr(dd.DeviceString, L"ATI"))) {
            return true;
        }
    }
    return false;
}

// ----------------- EDID parsing (kept same logic) -----------------

DisplayInfo::EDIDInfo DisplayInfo::parseEDID(const unsigned char* edid, size_t size) {
    EDIDInfo info = { "", 0, 0, false };
    if (!edid || size < 128) return info;

    // Validate EDID header (same validation as CompactScreen)
    if (edid[0] != 0x00 || edid[1] != 0xFF || edid[7] != 0x00) return info;

    // Extract native resolution from first detailed timing descriptor (bytes 54-71)
    if (size >= 72) {
        unsigned short hActive = ((edid[58] >> 4) << 8) | edid[56];
        unsigned short vActive = ((edid[61] >> 4) << 8) | edid[59];
        if (hActive > 0 && vActive > 0) {
            info.nativeWidth = hActive;
            info.nativeHeight = vActive;
            info.valid = true;
        }
    }

    // Extract monitor name from descriptor blocks (0xFC descriptor type)
    for (int i = 54; i < 126; i += 18) {
        if (i + 17 >= static_cast<int>(size)) break;
        if (edid[i] == 0x00 && edid[i + 1] == 0x00 && edid[i + 3] == 0xFC) {
            std::string name;
            for (int j = 5; j < 18; ++j) {
                if (edid[i + j] == 0x0A || edid[i + j] == 0x00) break;
                if (edid[i + j] >= 0x20 && edid[i + j] <= 0x7E) {
                    name += static_cast<char>(edid[i + j]);
                }
            }
            while (!name.empty() && name.back() == ' ') name.pop_back();
            if (!name.empty()) info.friendlyName = name;
            break;
        }
    }
    return info;
}

std::string DisplayInfo::getFriendlyNameFromEDID(const std::wstring& deviceName) {
    // Preserve CompactScreen registry scanning logic to obtain friendly name
    std::wstring monitorHardwareId;
    DISPLAY_DEVICEW ddMon{};
    ddMon.cb = sizeof(ddMon);
    for (DWORD iMon = 0; EnumDisplayDevicesW(deviceName.c_str(), iMon, &ddMon, 0); ++iMon) {
        if (ddMon.StateFlags & DISPLAY_DEVICE_ACTIVE) {
            monitorHardwareId = ddMon.DeviceID;
            break;
        }
    }

    HKEY hKeyMonitors;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Enum\\DISPLAY",
        0, KEY_READ, &hKeyMonitors) != ERROR_SUCCESS) {
        return "Generic PnP Monitor";
    }

    std::string friendlyName = "Generic PnP Monitor";
    WCHAR subKeyName[256];
    DWORD subKeyIndex = 0;

    std::wstring vendorPart;
    if (!monitorHardwareId.empty()) {
        size_t p1 = monitorHardwareId.find(L'\\');
        if (p1 != std::wstring::npos) {
            size_t p2 = monitorHardwareId.find(L'\\', p1 + 1);
            if (p2 != std::wstring::npos && p2 > p1 + 1)
                vendorPart = monitorHardwareId.substr(p1 + 1, p2 - p1 - 1);
            else
                vendorPart = monitorHardwareId.substr(p1 + 1);
        }
    }

    std::wstring vendLower;
    if (!vendorPart.empty()) {
        vendLower = vendorPart;
        std::transform(vendLower.begin(), vendLower.end(), vendLower.begin(), towlower);
    }

    // First pass: vendor-specific
    while (RegEnumKeyW(hKeyMonitors, subKeyIndex++, subKeyName, 256) == ERROR_SUCCESS) {
        std::wstring subKeyStr(subKeyName);
        if (!vendLower.empty()) {
            std::wstring subLower = subKeyStr;
            std::transform(subLower.begin(), subLower.end(), subLower.begin(), towlower);
            if (subLower.find(vendLower) != 0) continue;
        }

        HKEY hKeyMonitor;
        if (RegOpenKeyExW(hKeyMonitors, subKeyName, 0, KEY_READ, &hKeyMonitor) == ERROR_SUCCESS) {
            WCHAR deviceKeyName[256];
            DWORD deviceKeyIndex = 0;
            while (RegEnumKeyW(hKeyMonitor, deviceKeyIndex++, deviceKeyName, 256) == ERROR_SUCCESS) {
                HKEY hKeyDevice;
                if (RegOpenKeyExW(hKeyMonitor, deviceKeyName, 0, KEY_READ, &hKeyDevice) == ERROR_SUCCESS) {
                    HKEY hKeyDeviceParams;
                    if (RegOpenKeyExW(hKeyDevice, L"Device Parameters", 0, KEY_READ, &hKeyDeviceParams) == ERROR_SUCCESS) {
                        BYTE edidData[256];
                        DWORD edidSize = sizeof(edidData);
                        if (RegQueryValueExW(hKeyDeviceParams, L"EDID", nullptr, nullptr, edidData, &edidSize) == ERROR_SUCCESS) {
                            EDIDInfo edidInfo = parseEDID(edidData, edidSize);
                            if (!edidInfo.friendlyName.empty()) {
                                friendlyName = edidInfo.friendlyName;
                                RegCloseKey(hKeyDeviceParams);
                                RegCloseKey(hKeyDevice);
                                RegCloseKey(hKeyMonitor);
                                RegCloseKey(hKeyMonitors);
                                return friendlyName;
                            }
                        }
                        RegCloseKey(hKeyDeviceParams);
                    }
                    RegCloseKey(hKeyDevice);
                }
            }
            RegCloseKey(hKeyMonitor);
        }
    }

    // Fallback: scan everything
    subKeyIndex = 0;
    while (RegEnumKeyW(hKeyMonitors, subKeyIndex++, subKeyName, 256) == ERROR_SUCCESS) {
        HKEY hKeyMonitor;
        if (RegOpenKeyExW(hKeyMonitors, subKeyName, 0, KEY_READ, &hKeyMonitor) == ERROR_SUCCESS) {
            WCHAR deviceKeyName[256];
            DWORD deviceKeyIndex = 0;
            while (RegEnumKeyW(hKeyMonitor, deviceKeyIndex++, deviceKeyName, 256) == ERROR_SUCCESS) {
                HKEY hKeyDevice;
                if (RegOpenKeyExW(hKeyMonitor, deviceKeyName, 0, KEY_READ, &hKeyDevice) == ERROR_SUCCESS) {
                    HKEY hKeyDeviceParams;
                    if (RegOpenKeyExW(hKeyDevice, L"Device Parameters", 0, KEY_READ, &hKeyDeviceParams) == ERROR_SUCCESS) {
                        BYTE edidData[256];
                        DWORD edidSize = sizeof(edidData);
                        if (RegQueryValueExW(hKeyDeviceParams, L"EDID", nullptr, nullptr, edidData, &edidSize) == ERROR_SUCCESS) {
                            EDIDInfo edidInfo = parseEDID(edidData, edidSize);
                            if (!edidInfo.friendlyName.empty()) {
                                friendlyName = edidInfo.friendlyName;
                                RegCloseKey(hKeyDeviceParams);
                                RegCloseKey(hKeyDevice);
                                RegCloseKey(hKeyMonitor);
                                RegCloseKey(hKeyMonitors);
                                return friendlyName;
                            }
                        }
                        RegCloseKey(hKeyDeviceParams);
                    }
                    RegCloseKey(hKeyDevice);
                }
            }
            RegCloseKey(hKeyMonitor);
        }
    }

    RegCloseKey(hKeyMonitors);
    return friendlyName;
}

// ----------------- Core DXGI population (kept intact, extended) -----------------

DisplayInfo::DisplayInfo() {
    refresh();
}

bool DisplayInfo::refresh() {
    screens.clear();
    if (!populateFromDXGI()) return false;
    enrichWithNVAPI();
    enrichWithADL();
    return !screens.empty();
}

const std::vector<DisplayInfo::ScreenInfo>& DisplayInfo::getScreens() const {
    return screens;
}

bool DisplayInfo::populateFromDXGI() {
    IDXGIFactory1* factory = nullptr;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) return false;

    bool hasNvidia = isNvidiaPresent();
    bool hasAMD = isAMDPresent();

    IDXGIAdapter1* adapter = nullptr;
    for (UINT a = 0; factory->EnumAdapters1(a, &adapter) != DXGI_ERROR_NOT_FOUND; ++a) {
        IDXGIOutput* output = nullptr;

        for (UINT o = 0; adapter->EnumOutputs(o, &output) != DXGI_ERROR_NOT_FOUND; ++o) {
            IDXGIOutput6* output6 = nullptr;
            if (SUCCEEDED(output->QueryInterface(IID_PPV_ARGS(&output6)))) {
                DXGI_OUTPUT_DESC1 desc1{};
                if (SUCCEEDED(output6->GetDesc1(&desc1))) {

                    // ===== GET CURRENT RESOLUTION (WHAT WINDOWS SHOWS) =====
                    DEVMODEW dm{};
                    dm.dmSize = sizeof(dm);
                    int currentW = 0, currentH = 0, refresh = 60;

                    if (EnumDisplaySettingsExW(desc1.DeviceName, ENUM_CURRENT_SETTINGS, &dm, 0)) {
                        currentW = static_cast<int>(dm.dmPelsWidth);
                        currentH = static_cast<int>(dm.dmPelsHeight);
                        if (dm.dmDisplayFrequency > 1) refresh = dm.dmDisplayFrequency;
                    }

                    // ===== GET MONITOR FRIENDLY NAME =====
                    std::wstring deviceNameW = desc1.DeviceName;
                    std::string friendlyName = getFriendlyNameFromEDID(deviceNameW);

                    // ===== GET NATIVE PANEL RESOLUTION (FROM EDID) =====
                    int nativeW = 0, nativeH = 0;
                    HKEY hKeyMonitors;
                    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                        L"SYSTEM\\CurrentControlSet\\Enum\\DISPLAY",
                        0, KEY_READ, &hKeyMonitors) == ERROR_SUCCESS) {
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
                                        HKEY hKeyDeviceParams;
                                        if (RegOpenKeyExW(hKeyDevice, L"Device Parameters", 0, KEY_READ, &hKeyDeviceParams) == ERROR_SUCCESS) {
                                            BYTE edidData[256];
                                            DWORD edidSize = sizeof(edidData);

                                            if (RegQueryValueExW(hKeyDeviceParams, L"EDID", nullptr, nullptr, edidData, &edidSize) == ERROR_SUCCESS) {
                                                EDIDInfo edidInfo = parseEDID(edidData, edidSize);
                                                if (edidInfo.valid && edidInfo.nativeWidth > 0) {
                                                    nativeW = edidInfo.nativeWidth;
                                                    nativeH = edidInfo.nativeHeight;
                                                    RegCloseKey(hKeyDeviceParams);
                                                    RegCloseKey(hKeyDevice);
                                                    RegCloseKey(hKeyMonitor);
                                                    goto edid_found;
                                                }
                                            }
                                            RegCloseKey(hKeyDeviceParams);
                                        }
                                        RegCloseKey(hKeyDevice);
                                    }
                                }
                                RegCloseKey(hKeyMonitor);
                            }
                        }
                    edid_found:
                        RegCloseKey(hKeyMonitors);
                    }

                    // Fallback: if we can't get native resolution, assume currently applied resolution is native
                    if (nativeW <= 0 || nativeH <= 0) {
                        nativeW = currentW;
                        nativeH = currentH;
                    }

                    // ===== GET DPI SCALING (WINDOWS UI SCALE) =====
                    int scalePercent = 100;

                    // Method 1: Read from Registry (most accurate - what Windows Settings shows)
                    HKEY hKeyDisplay;
                    std::wstring devicePath = desc1.DeviceName;

                    // Try to get the device ID for registry lookup
                    DISPLAY_DEVICEW ddMon{};
                    ddMon.cb = sizeof(ddMon);

                    for (DWORD iMon = 0; EnumDisplayDevicesW(desc1.DeviceName, iMon, &ddMon, 0); ++iMon) {
                        if (ddMon.StateFlags & DISPLAY_DEVICE_ACTIVE) {
                            // Extract monitor ID from DeviceID (e.g., "MONITOR\\DEL4067\\{...}")
                            std::wstring monitorID = ddMon.DeviceID;

                            // Try to find scaling in registry
                            std::wstring regPath = L"SYSTEM\\CurrentControlSet\\Control\\GraphicsDrivers\\Configuration";
                            HKEY hKeyConfig;

                            if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, regPath.c_str(), 0, KEY_READ, &hKeyConfig) == ERROR_SUCCESS) {
                                // Enumerate configuration subkeys
                                WCHAR configKeyName[256];
                                DWORD configIndex = 0;

                                while (RegEnumKeyW(hKeyConfig, configIndex++, configKeyName, 256) == ERROR_SUCCESS) {
                                    HKEY hKeyConfigSub;
                                    if (RegOpenKeyExW(hKeyConfig, configKeyName, 0, KEY_READ, &hKeyConfigSub) == ERROR_SUCCESS) {
                                        // Look for "00" subkey
                                        HKEY hKey00;
                                        if (RegOpenKeyExW(hKeyConfigSub, L"00", 0, KEY_READ, &hKey00) == ERROR_SUCCESS) {
                                            // Try to read DpiValue
                                            DWORD dpiValue = 0;
                                            DWORD dpiSize = sizeof(dpiValue);

                                            if (RegQueryValueExW(hKey00, L"DpiValue", nullptr, nullptr, (BYTE*)&dpiValue, &dpiSize) == ERROR_SUCCESS) {
                                                if (dpiValue > 0 && dpiValue != 0xFFFFFFFF) {
                                                    scalePercent = static_cast<int>(std::round((dpiValue / 96.0f) * 100.0f));
                                                    RegCloseKey(hKey00);
                                                    RegCloseKey(hKeyConfigSub);
                                                    RegCloseKey(hKeyConfig);
                                                    goto dpi_found;
                                                }
                                            }

                                            RegCloseKey(hKey00);
                                        }
                                        RegCloseKey(hKeyConfigSub);
                                    }
                                }
                                RegCloseKey(hKeyConfig);
                            }
                            break;
                        }
                    }

                dpi_found:

                    // Method 2: GetDpiForMonitor with MDT_EFFECTIVE_DPI
                    if (scalePercent == 100) {
                        UINT dpiX = 96, dpiY = 96;
                        if (SUCCEEDED(GetDpiForMonitor(desc1.Monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY))) {
                            scalePercent = static_cast<int>(std::round((dpiX / 96.0f) * 100.0f));
                        }
                    }

                    // Method 3: GetDeviceCaps on monitor's DC
                    if (scalePercent == 100) {
                        HDC hdc = CreateDCW(L"DISPLAY", desc1.DeviceName, nullptr, nullptr);
                        if (hdc) {
                            int logPixelsX = GetDeviceCaps(hdc, LOGPIXELSX);
                            if (logPixelsX > 0 && logPixelsX != 96) {
                                scalePercent = static_cast<int>(std::round((logPixelsX / 96.0f) * 100.0f));
                            }
                            DeleteDC(hdc);
                        }
                    }

                    // Method 4: Calculate from desktop coordinates vs current resolution
                    if (scalePercent == 100 || scalePercent == 122) { // Also retry if we got weird value
                        int desktopW = desc1.DesktopCoordinates.right - desc1.DesktopCoordinates.left;
                        int desktopH = desc1.DesktopCoordinates.bottom - desc1.DesktopCoordinates.top;

                        if (desktopW > 0 && currentW > 0 && desktopW != currentW) {
                            float scaleRatio = static_cast<float>(currentW) / static_cast<float>(desktopW);
                            int calculatedScale = static_cast<int>(std::round(scaleRatio * 100.0f));

                            // Snap to common Windows scaling values
                            if (calculatedScale >= 95 && calculatedScale <= 105) scalePercent = 100;
                            else if (calculatedScale >= 120 && calculatedScale <= 130) scalePercent = 125;
                            else if (calculatedScale >= 145 && calculatedScale <= 155) scalePercent = 150;
                            else if (calculatedScale >= 170 && calculatedScale <= 180) scalePercent = 175;
                            else if (calculatedScale >= 195 && calculatedScale <= 205) scalePercent = 200;
                            else if (calculatedScale >= 220 && calculatedScale <= 230) scalePercent = 225;
                            else if (calculatedScale >= 245 && calculatedScale <= 255) scalePercent = 250;
                            else scalePercent = calculatedScale; // Use raw value if not near common scale
                        }
                    }

                    // ===== COMPUTE DSR/VSR UPSCALE FACTOR =====
                    int upscaleFactor = computeUpscaleFactor(currentW, nativeW);
                    std::string upscaleStr = "1x";
                    if (upscaleFactor > 1) {
                        char tmp[16];
                        snprintf(tmp, sizeof(tmp), "%dx", upscaleFactor);
                        upscaleStr = tmp;
                    }

                    // ===== BUILD SCREEN INFO (preserve original naming but use nativeW/nativeH for native fields) =====
                    ScreenInfo info;
                    info.name = friendlyName;

                    // store native panel resolution (from EDID or fallback)
                    info.native_width = nativeW;
                    info.native_height = nativeH;
                    info.native_resolution = std::to_string(nativeW) + "x" + std::to_string(nativeH);

                    // applied resolution (what Windows shows)
                    info.current_width = currentW;
                    info.current_height = currentH;
                    info.refresh_rate = refresh;

                    // scaling/upscale
                    info.scale_percent = scalePercent;
                    info.scale_mul = scaleMultiplier(scalePercent);
                    info.upscale = upscaleStr;

                    // aspect ratio (from applied resolution)
                    info.aspect_ratio = computeAspectRatio(currentW, currentH);

                    // DSR/VSR heuristics
                    info.dsr_enabled = false;
                    info.dsr_type = "None";
                    if (upscaleFactor > 1) {
                        if (hasNvidia) {
                            info.dsr_enabled = true;
                            info.dsr_type = "DSR";
                        }
                        else if (hasAMD) {
                            info.dsr_enabled = true;
                            info.dsr_type = "VSR";
                        }
                        else {
                            info.dsr_enabled = true; // unknown vendor but upscaling >1
                            info.dsr_type = "Unknown";
                        }
                    }

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

bool DisplayInfo::enrichWithNVAPI() {
#ifdef USE_NVAPI
    // NVAPI integration can be added here for more accurate DSR detection
    // (left intentionally empty to preserve build when NVAPI isn't used)
#endif
    return true;
}

bool DisplayInfo::enrichWithADL() {
#ifdef USE_ADL
    // ADL integration can be added here for more accurate VSR detection
    // (left intentionally empty to preserve build when ADL isn't used)
#endif
    return true;
}
