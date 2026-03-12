#include "include\CompactSystem.h"
#include <windows.h>
#include <string>
#include <iostream>
using namespace std;
string readRegistryValue(HKEY root, const string& subkey, const string& valueName) {
    HKEY hKey;
    if (RegOpenKeyExA(root, subkey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return "Unknown";

    char value[256];
    DWORD value_length = sizeof(value);
    DWORD type = 0;

    if (RegQueryValueExA(hKey, valueName.c_str(), nullptr, &type, reinterpret_cast<LPBYTE>(value), &value_length) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return "Unknown";
    }

    RegCloseKey(hKey);

    if (type == REG_SZ || type == REG_EXPAND_SZ)
        return string(value);
    else
        return "Unknown";
}

string CompactSystem::getBIOSInfo() {
    // Registry paths for BIOS
    string biosVersion = readRegistryValue(
        HKEY_LOCAL_MACHINE,
        "HARDWARE\\DESCRIPTION\\System\\BIOS",
        "BIOSVersion"
    );

    string biosVendor = readRegistryValue(
        HKEY_LOCAL_MACHINE,
        "HARDWARE\\DESCRIPTION\\System\\BIOS",
        "BIOSVendor"
    );

    string biosDate = readRegistryValue(
        HKEY_LOCAL_MACHINE,
        "HARDWARE\\DESCRIPTION\\System\\BIOS",
        "BIOSReleaseDate"
    );

    return biosVendor + " " + biosVersion + " (" + biosDate + ")";
}

string CompactSystem::getMotherboardInfo() {
    // Registry paths for Motherboard (BaseBoard)
    string boardProduct = readRegistryValue(
        HKEY_LOCAL_MACHINE,
        "HARDWARE\\DESCRIPTION\\System\\BIOS",
        "BaseBoardProduct"
    );

    string boardManufacturer = readRegistryValue(
        HKEY_LOCAL_MACHINE,
        "HARDWARE\\DESCRIPTION\\System\\BIOS",
        "BaseBoardManufacturer"
    );

    return boardManufacturer + " " + boardProduct;
}
