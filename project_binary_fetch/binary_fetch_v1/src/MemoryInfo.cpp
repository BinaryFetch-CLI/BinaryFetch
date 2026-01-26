#include "MemoryInfo.h"
#include <windows.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <iostream>
#include <iomanip>
#include <string>

#pragma comment(lib, "wbemuuid.lib")

MemoryInfo::MemoryInfo() {
    fetchSystemMemory();
    fetchModulesInfo();
}

void MemoryInfo::fetchSystemMemory() {
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status)) {
        // Convert bytes to GB, rounding up to nearest GB
        totalGB = static_cast<int>((status.ullTotalPhys + (1024 * 1024 * 1024) - 1) / (1024 * 1024 * 1024));
        freeGB = static_cast<int>(status.ullAvailPhys / (1024 * 1024 * 1024));
    }
    else {
        totalGB = 0;
        freeGB = 0;
    }
}

void MemoryInfo::fetchModulesInfo() {
    // Initialize COM
    HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) return;

    hres = CoInitializeSecurity(
        NULL, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE, NULL
    );
    if (FAILED(hres)) {
        CoUninitialize();
        return;
    }

    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(
        CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc
    );
    if (FAILED(hres)) {
        CoUninitialize();
        return;
    }

    IWbemServices* pSvc = NULL;
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL, NULL, 0, NULL, 0, 0, &pSvc
    );
    if (FAILED(hres)) {
        pLoc->Release();
        CoUninitialize();
        return;
    }

    hres = CoSetProxyBlanket(
        pSvc,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE
    );
    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;
    }

    IEnumWbemClassObject* pEnumerator = NULL;
    // Get more properties including SMBIOSMemoryType which is more reliable
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT Capacity, Speed, SMBIOSMemoryType, Manufacturer, PartNumber FROM Win32_PhysicalMemory"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL, &pEnumerator
    );

    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;
    }

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (0 == uReturn) break;

        VARIANT vtProp;
        MemoryModule module;

        // Capacity in bytes -> GB
        hr = pclsObj->Get(L"Capacity", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hr)) {
            unsigned long long bytes = 0;
            if (vtProp.vt == VT_BSTR) {
                bytes = _wtoi64(vtProp.bstrVal);
            }
            else if (vtProp.vt == VT_I8 || vtProp.vt == VT_UI8) {
                bytes = vtProp.ullVal;
            }
            else if (vtProp.vt == VT_I4 || vtProp.vt == VT_UI4) {
                bytes = vtProp.uintVal;
            }

            if (bytes > 0) {
                int gb = static_cast<int>(bytes / (1024 * 1024 * 1024));
                // Handle cases where GB might not divide evenly
                if (bytes % (1024 * 1024 * 1024) != 0) {
                    gb++;  // Round up to nearest GB
                }
                module.capacity = std::to_string(gb) + "GB";
            }
            else {
                module.capacity = "Unknown";
            }
        }
        else {
            module.capacity = "Unknown";
        }
        VariantClear(&vtProp);

        // Speed
        hr = pclsObj->Get(L"Speed", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hr)) {
            if (vtProp.vt == VT_I4 || vtProp.vt == VT_UI4) {
                module.speed = std::to_string(vtProp.uintVal) + " MHz";
            }
            else if (vtProp.vt == VT_BSTR) {
                // Convert BSTR to std::string
                std::wstring ws(vtProp.bstrVal);
                std::string speedStr(ws.begin(), ws.end());
                module.speed = speedStr + " MHz";
            }
            else {
                module.speed = "Unknown MHz";
            }
        }
        else {
            module.speed = "Unknown MHz";
        }
        VariantClear(&vtProp);

        // Try SMBIOSMemoryType first (more reliable)
        hr = pclsObj->Get(L"SMBIOSMemoryType", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hr) && vtProp.vt != VT_NULL) {
            switch (vtProp.uintVal) {
            case 26: module.type = "DDR4"; break;        // 0x1A
            case 24: module.type = "DDR3"; break;        // 0x18
            case 27: module.type = "DDR5"; break;        // 0x1B
            case 20: module.type = "DDR"; break;         // 0x14
            case 21: module.type = "DDR2"; break;        // 0x15
            case 19: module.type = "DDR2-FB-DIMM"; break;// 0x13

            default:
            {
                VARIANT vtProp2;
                HRESULT hr2 = pclsObj->Get(L"MemoryType", 0, &vtProp2, 0, 0);
                if (SUCCEEDED(hr2) && vtProp2.vt != VT_NULL) {
                    switch (vtProp2.uintVal) {
                    case 24: module.type = "DDR3"; break;
                    case 26: module.type = "DDR4"; break;
                    case 20: module.type = "DDR"; break;
                    case 21: module.type = "DDR2"; break;
                    case 27: module.type = "DDR5"; break;
                    default: module.type = "Unknown"; break;
                    }
                }
                else {
                    module.type = "Unknown";
                }
                VariantClear(&vtProp2);
            }
            break;
            }
        }

        else {
            // Fallback to MemoryType
            VARIANT vtProp2;
            hr = pclsObj->Get(L"MemoryType", 0, &vtProp2, 0, 0);
            if (SUCCEEDED(hr) && vtProp2.vt != VT_NULL) {
                switch (vtProp2.uintVal) {
                case 24: module.type = "DDR3"; break;
                case 26: module.type = "DDR4"; break;
                case 20: module.type = "DDR"; break;
                case 21: module.type = "DDR2"; break;
                case 27: module.type = "DDR5"; break;
                default: module.type = "Unknown"; break;
                }
            }
            else {
                module.type = "Unknown";
            }
            VariantClear(&vtProp2);
        }
        VariantClear(&vtProp);

        modules.push_back(module);
        pclsObj->Release();
    }

    pEnumerator->Release();
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();
}

int MemoryInfo::getTotal() const { return totalGB; }
int MemoryInfo::getFree() const { return freeGB; }
int MemoryInfo::getUsedPercentage() const {
    if (totalGB == 0) return 0;
    // Ensure percentage doesn't exceed 100%
    double percentage = (static_cast<double>(totalGB - freeGB) / totalGB) * 100;
    if (percentage > 100.0) percentage = 100.0;
    if (percentage < 0.0) percentage = 0.0;
    return static_cast<int>(percentage);
}
const std::vector<MemoryModule>& MemoryInfo::getModules() const { return modules; }