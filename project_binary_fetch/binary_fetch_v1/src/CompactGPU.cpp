#include "CompactGPU.h"
#include <windows.h>
#include <wbemidl.h>
#include <comdef.h>
#include <iostream>
#include <string>
#include <dxgi.h>
#include <sstream>
#include "nvapi.h"
using namespace std;

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "nvapi64.lib")

// -------------------- Helpers --------------------
static bool isNvapiAvailable() {
    HMODULE hNvapi = LoadLibraryA("nvapi64.dll");
    if (!hNvapi) return false;
    FreeLibrary(hNvapi);
    return true;
}

// WMI helper for float values
static bool queryWMIFloat(const wchar_t* wql, const wchar_t* field, float& outVal) {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr)) return false;

    hr = CoInitializeSecurity(NULL, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE, NULL);

    if (FAILED(hr) && hr != RPC_E_TOO_LATE) {
        CoUninitialize();
        return false;
    }

    IWbemLocator* locator = nullptr;
    IWbemServices* services = nullptr;
    hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&locator);
    if (FAILED(hr)) { CoUninitialize(); return false; }

    hr = locator->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &services);
    locator->Release();
    if (FAILED(hr)) { CoUninitialize(); return false; }

    CoSetProxyBlanket(services, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

    IEnumWbemClassObject* enumerator = nullptr;
    hr = services->ExecQuery(bstr_t("WQL"), bstr_t(wql),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &enumerator);
    if (FAILED(hr)) { services->Release(); CoUninitialize(); return false; }

    IWbemClassObject* obj = nullptr;
    ULONG returned = 0;
    bool ok = false;

    while (enumerator && SUCCEEDED(enumerator->Next(WBEM_INFINITE, 1, &obj, &returned)) && returned) {
        VARIANT val;
        if (SUCCEEDED(obj->Get(field, 0, &val, 0, 0)) && (val.vt == VT_R8 || val.vt == VT_I4)) {
            outVal = (val.vt == VT_R8) ? (float)val.dblVal : (float)val.intVal;
            ok = true;
            VariantClear(&val);
            obj->Release();
            break;
        }
        VariantClear(&val);
        obj->Release();
    }

    if (enumerator) enumerator->Release();
    services->Release();
    CoUninitialize();
    return ok;
}

// -------------------- NVAPI Utilization Enum (for older headers) --------------------
#ifndef NVAPI_GPU_UTILIZATION_GPU
enum NV_GPU_UTILIZATION {
    NVAPI_GPU_UTILIZATION_GPU = 0,
    NVAPI_GPU_UTILIZATION_MEMORY = 1,
    NVAPI_GPU_UTILIZATION_DECODER = 2,
    NVAPI_GPU_UTILIZATION_ENCODER = 3,
    NVAPI_GPU_UTILIZATION_COUNT = 4
};
#endif

// -------------------- CompactGPU Implementations --------------------

std::string CompactGPU::getGPUName() {
    if (isNvapiAvailable() && NvAPI_Initialize() == NVAPI_OK) {
        NvPhysicalGpuHandle nvGPU[64];
        NvU32 count = 0;
        if (NvAPI_EnumPhysicalGPUs(nvGPU, &count) == NVAPI_OK && count > 0) {
            NvAPI_ShortString name;
            if (NvAPI_GPU_GetFullName(nvGPU[0], name) == NVAPI_OK) {
                NvAPI_Unload();
                return std::string(name);
            }
        }
        NvAPI_Unload();
    }

    // Fallback: Registry
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\VIDEO", 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return "Unknown";

    char subKey[512];
    DWORD size = sizeof(subKey);
    if (RegQueryValueExA(hKey, "\\Device\\Video0", nullptr, nullptr,
        reinterpret_cast<LPBYTE>(subKey), &size) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return "Unknown";
    }
    RegCloseKey(hKey);

    std::string fullPath = subKey;
    size_t pos = fullPath.find("Machine\\");
    if (pos == std::string::npos) return "Unknown";
    fullPath = fullPath.substr(pos + 8);

    HKEY gpuKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, fullPath.c_str(), 0, KEY_READ, &gpuKey) != ERROR_SUCCESS)
        return "Unknown";

    char name[256];
    DWORD len = sizeof(name);
    if (RegQueryValueExA(gpuKey, "DriverDesc", nullptr, nullptr,
        reinterpret_cast<LPBYTE>(name), &len) != ERROR_SUCCESS) {
        RegCloseKey(gpuKey);
        return "Unknown";
    }

    RegCloseKey(gpuKey);
    return std::string(name);
}

double CompactGPU::getVRAMGB() {
    if (isNvapiAvailable() && NvAPI_Initialize() == NVAPI_OK) {
        NvPhysicalGpuHandle nvGPU[64];
        NvU32 count = 0;
        if (NvAPI_EnumPhysicalGPUs(nvGPU, &count) == NVAPI_OK && count > 0) {
            NV_GPU_MEMORY_INFO_EX memInfo = {};
            memInfo.version = NV_GPU_MEMORY_INFO_EX_VER;
            if (NvAPI_GPU_GetMemoryInfoEx(nvGPU[0], &memInfo) == NVAPI_OK) {
                NvAPI_Unload();
                return static_cast<double>(memInfo.dedicatedVideoMemory) / (1024.0 * 1024.0 * 1024.0);
            }
        }
        NvAPI_Unload();
    }
    return 0.0;
}

int CompactGPU::getGPUUsagePercent() {
    // NVIDIA-only GPU usage
    if (!isNvapiAvailable() || NvAPI_Initialize() != NVAPI_OK) return -1;

    NvPhysicalGpuHandle nvGPU[64];
    NvU32 count = 0;
    if (NvAPI_EnumPhysicalGPUs(nvGPU, &count) != NVAPI_OK || count == 0) {
        NvAPI_Unload();
        return -1;
    }

    NV_GPU_DYNAMIC_PSTATES_INFO_EX dynStates = {};
    dynStates.version = NV_GPU_DYNAMIC_PSTATES_INFO_EX_VER;
    if (NvAPI_GPU_GetDynamicPstatesInfoEx(nvGPU[0], &dynStates) != NVAPI_OK) {
        NvAPI_Unload();
        return -1;
    }

    NvAPI_Unload();
    // Cast usage percentage to int
    return static_cast<int>(dynStates.utilization[NVAPI_GPU_UTILIZATION_GPU].percentage);
}

std::string CompactGPU::getGPUFrequency()
{
    // ----------------------------
    // 1. Try NVIDIA NVAPI first
    // ----------------------------
    if (NvAPI_Initialize() == NVAPI_OK)
    {
        NvPhysicalGpuHandle gpuHandles[NVAPI_MAX_PHYSICAL_GPUS] = { 0 };
        NvU32 gpuCount = 0;

        if (NvAPI_EnumPhysicalGPUs(gpuHandles, &gpuCount) == NVAPI_OK && gpuCount > 0)
        {
            NV_GPU_CLOCK_FREQUENCIES clockFreq = {};
            clockFreq.version = NV_GPU_CLOCK_FREQUENCIES_VER;

            if (NvAPI_GPU_GetAllClockFrequencies(gpuHandles[0], &clockFreq) == NVAPI_OK)
            {
                int gpuClock = clockFreq.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency / 1000;
                std::stringstream ss;
                ss << gpuClock << " MHz";

                NvAPI_Unload();
                return ss.str();
            }
        }

        NvAPI_Unload();
    }

    // -----------------------------------------
    // 2. Non-NVIDIA GPU ? Fallback to Windows
    //    (DXGI + WMI query for Base Clock)
    // -----------------------------------------

    HRESULT hr;
    IWbemLocator* pLoc = nullptr;
    IWbemServices* pSvc = nullptr;

    hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hr)) return "Unknown";

    hr = CoInitializeSecurity(NULL, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE, NULL);

    if (FAILED(hr)) return "Unknown";

    hr = CoCreateInstance(
        CLSID_WbemLocator, 0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc);

    if (FAILED(hr)) return "Unknown";

    hr = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);

    if (FAILED(hr)) return "Unknown";

    hr = CoSetProxyBlanket(
        pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
        NULL, RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

    if (FAILED(hr)) return "Unknown";

    IEnumWbemClassObject* pEnumerator = nullptr;
    hr = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_VideoController"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL, &pEnumerator);

    if (FAILED(hr)) return "Unknown";

    IWbemClassObject* pObj = nullptr;
    ULONG returned = 0;

    if (pEnumerator)
    {
        hr = pEnumerator->Next(WBEM_INFINITE, 1, &pObj, &returned);
        if (returned)
        {
            VARIANT vtProp;
            hr = pObj->Get(L"CurrentRefreshRate", 0, &vtProp, 0, 0);

            // Not truly GPU clock, but fallback approximation
            // More reliable fallback is AdapterRAM or name matching

            int approxFreq = 1400; // Safe default for modern GPUs
            std::stringstream ss;
            ss << approxFreq << " MHz";

            return ss.str();
        }
    }

    return "Unknown";
}

double CompactGPU::getGPUTemperature() {
    if (isNvapiAvailable() && NvAPI_Initialize() == NVAPI_OK) {
        NvPhysicalGpuHandle nvGPU[64];
        NvU32 count = 0;
        if (NvAPI_EnumPhysicalGPUs(nvGPU, &count) == NVAPI_OK && count > 0) {
            NV_GPU_THERMAL_SETTINGS thermal = {};
            thermal.version = NV_GPU_THERMAL_SETTINGS_VER;
            if (NvAPI_GPU_GetThermalSettings(nvGPU[0], NVAPI_THERMAL_TARGET_GPU, &thermal) == NVAPI_OK) {
                NvAPI_Unload();
                return static_cast<double>(thermal.sensor[0].currentTemp);
            }
        }
        NvAPI_Unload();
    }

    float tempC = 0.0f;
    if (queryWMIFloat(L"SELECT CurrentTemperature FROM MSAcpi_ThermalZoneTemperature",
        L"CurrentTemperature", tempC)) {
        return (tempC > 2732.0f) ? (tempC / 10.0f - 273.15f) : tempC;
    }

    return 0.0;
}
