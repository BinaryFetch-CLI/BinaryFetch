#include "SystemInfo.h"
#include <windows.h>
#include <iostream>

SystemInfo::SystemInfo() {
    // Nothing to initialize, using registry only
}

SystemInfo::~SystemInfo() {
    // Nothing to clean
}

// Registry reading function (internal)
std::string SystemInfo::read_registry_value(const std::string& subkey, const std::string& valueName) {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, subkey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return "N/A";

    char value[256] = { 0 };
    DWORD size = sizeof(value);
    DWORD type = 0;

    if (RegQueryValueExA(hKey, valueName.c_str(), nullptr, &type, reinterpret_cast<LPBYTE>(value), &size) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return "N/A";
    }

    RegCloseKey(hKey);
    return std::string(value);
}

// BIOS info
std::string SystemInfo::get_bios_vendor() {
    return read_registry_value("HARDWARE\\DESCRIPTION\\System\\BIOS", "BIOSVendor");
}

std::string SystemInfo::get_bios_version() {
    return read_registry_value("HARDWARE\\DESCRIPTION\\System\\BIOS", "BIOSVersion");
}

std::string SystemInfo::get_bios_date() {
    return read_registry_value("HARDWARE\\DESCRIPTION\\System\\BIOS", "BIOSReleaseDate");
}

// Motherboard info
std::string SystemInfo::get_motherboard_model() {
    return read_registry_value("HARDWARE\\DESCRIPTION\\System\\BIOS", "BaseBoardProduct");
}

std::string SystemInfo::get_motherboard_manufacturer() {
    return read_registry_value("HARDWARE\\DESCRIPTION\\System\\BIOS", "BaseBoardManufacturer");
}
