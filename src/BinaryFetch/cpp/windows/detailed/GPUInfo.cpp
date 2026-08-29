#include "GPUInfo.h"
#include <windows.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <wbemidl.h>
#include <comdef.h>
#include <iostream>
#include <sstream>
#include "nvapi.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "nvapi64.lib")

using namespace std;

// Converts a wide string to UTF-8.
static string wstr_to_utf8(const wstring& w)
{
    if (w.empty()) return {};
    int sz = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    string r(sz - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, &r[0], sz, nullptr, nullptr);
    return r;
}

// Queries GPU temperature via WMI.
//
// There is no vendor-neutral Windows API for GPU temperature, so this
// tries two WMI sources in order of reliability:
//   1. OpenHardwareMonitor's WMI namespace, if OHM is installed and running.
//      This is the most accurate source when available.
//   2. The generic ACPI thermal zone, as a last resort. This is frequently
//      a motherboard or CPU sensor rather than the GPU, so it is only used
//      when nothing better exists.
// Returns -1.0f if neither source is available.
static float query_wmi_gpu_temperature()
{
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    bool needsUninit = SUCCEEDED(hr);

    hr = CoInitializeSecurity(NULL, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE, NULL);

    IWbemLocator* locator = nullptr;
    IWbemServices* services = nullptr;

    hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&locator);
    if (FAILED(hr)) {
        if (needsUninit) CoUninitialize();
        return -1.0f;
    }

    // Method 1: OpenHardwareMonitor. Only works if the user has it
    // installed and its WMI provider is running.
    hr = locator->ConnectServer(_bstr_t(L"ROOT\\OpenHardwareMonitor"), NULL, NULL, 0, NULL, 0, 0, &services);
    if (SUCCEEDED(hr))
    {
        CoSetProxyBlanket(services, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
            RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

        IEnumWbemClassObject* enumerator = nullptr;

        hr = services->ExecQuery(
            bstr_t("WQL"),
            bstr_t(L"SELECT Value FROM Sensor WHERE SensorType='Temperature' AND (Name LIKE '%GPU%' OR Parent LIKE '%GPU%')"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            NULL,
            &enumerator);

        if (SUCCEEDED(hr))
        {
            IWbemClassObject* obj = nullptr;
            ULONG returned = 0;

            while (enumerator->Next(WBEM_INFINITE, 1, &obj, &returned) == S_OK && returned)
            {
                VARIANT val;
                if (SUCCEEDED(obj->Get(L"Value", 0, &val, 0, 0)))
                {
                    float temp = (val.vt == VT_R8) ? (float)val.dblVal : (float)val.intVal;

                    VariantClear(&val);
                    obj->Release();
                    enumerator->Release();
                    services->Release();
                    locator->Release();
                    if (needsUninit) CoUninitialize();
                    return temp;
                }
                VariantClear(&val);
                obj->Release();
            }
            if (enumerator) enumerator->Release();
        }
        services->Release();
    }

    // Method 2: ACPI thermal zone. May report a non-GPU sensor; used only
    // because nothing more specific is available without third-party tools.
    hr = locator->ConnectServer(_bstr_t(L"ROOT\\WMI"), NULL, NULL, 0, NULL, 0, 0, &services);
    if (SUCCEEDED(hr))
    {
        CoSetProxyBlanket(services, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
            RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

        IEnumWbemClassObject* enumerator = nullptr;
        hr = services->ExecQuery(
            bstr_t("WQL"),
            bstr_t(L"SELECT CurrentTemperature FROM MSAcpi_ThermalZoneTemperature"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            NULL,
            &enumerator);

        if (SUCCEEDED(hr))
        {
            IWbemClassObject* obj = nullptr;
            ULONG returned = 0;
            if (enumerator->Next(WBEM_INFINITE, 1, &obj, &returned) == S_OK && returned)
            {
                VARIANT val;
                if (SUCCEEDED(obj->Get(L"CurrentTemperature", 0, &val, 0, 0)))
                {
                    float temp = (val.vt == VT_R8) ? (float)val.dblVal : (float)val.intVal;

                    // WMI reports this in tenths of a degree Kelvin.
                    if (temp > 2000.0f)
                        temp = (temp / 10.0f) - 273.15f;

                    VariantClear(&val);
                    obj->Release();
                    enumerator->Release();
                    services->Release();
                    locator->Release();
                    if (needsUninit) CoUninitialize();
                    return temp;
                }
                VariantClear(&val);
                obj->Release();
            }
            if (enumerator) enumerator->Release();
        }
        services->Release();
    }

    locator->Release();
    if (needsUninit) CoUninitialize();

    return -1.0f;
}

// Runs a WQL query and extracts a single numeric field from the first
// matching result. Returns false if the query fails or returns nothing.
static bool query_wmi_float(const wchar_t* wql, const wchar_t* field, float& outVal)
{
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    bool needsUninit = SUCCEEDED(hr);

    hr = CoInitializeSecurity(NULL, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE, NULL);

    IWbemLocator* locator = nullptr;
    IWbemServices* services = nullptr;

    hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&locator);
    if (FAILED(hr)) {
        if (needsUninit) CoUninitialize();
        return false;
    }

    hr = locator->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &services);
    locator->Release();

    if (FAILED(hr)) {
        if (needsUninit) CoUninitialize();
        return false;
    }

    CoSetProxyBlanket(services, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

    IEnumWbemClassObject* enumerator = nullptr;

    hr = services->ExecQuery(
        bstr_t("WQL"),
        bstr_t(wql),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &enumerator);

    if (FAILED(hr)) {
        services->Release();
        if (needsUninit) CoUninitialize();
        return false;
    }

    IWbemClassObject* obj = nullptr;
    ULONG returned = 0;
    bool ok = false;

    while (enumerator &&
        SUCCEEDED(enumerator->Next(WBEM_INFINITE, 1, &obj, &returned)) &&
        returned)
    {
        VARIANT val;

        if (SUCCEEDED(obj->Get(field, 0, &val, 0, 0)) &&
            (val.vt == VT_R8 || val.vt == VT_I4))
        {
            outVal = (val.vt == VT_R8)
                ? (float)val.dblVal
                : (float)val.intVal;

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
    if (needsUninit) CoUninitialize();

    return ok;
}

// Vendor-neutral GPU usage fallback, used when NVAPI is unavailable.
// Reads the 3D engine utilization counter exposed by the GPU scheduler.
float GPUInfo::get_gpu_usage()
{
    float val = 0.0f;

    query_wmi_float(
        L"SELECT UtilizationPercentage FROM Win32_PerfFormattedData_GPUPerformanceCounters_GPUEngine WHERE Name LIKE '%_3D%'",
        L"UtilizationPercentage",
        val);

    return val;
}

// Vendor-neutral GPU temperature fallback, used when NVAPI is unavailable.
float GPUInfo::get_gpu_temperature()
{
    return query_wmi_gpu_temperature();
}

// Vendor-neutral GPU core count fallback, used when NVAPI is unavailable
// or the GPU is not NVIDIA.
//
// Windows exposes no public API for shader/CUDA core count on any vendor.
// NVAPI's NvAPI_GPU_GetGpuCoreCount (used in get_nvapi_core_count below)
// is the only reliable source, and only covers NVIDIA. There is no
// equivalent for AMD or Intel, so this fallback intentionally returns 0
// rather than guessing a number for an unknown GPU model.
int GPUInfo::get_gpu_core_count()
{
    return 0;
}

// Checks whether the NVIDIA NVAPI library is present on this system.
static bool nvapi_available()
{
    HMODULE nv = LoadLibraryA("nvapi64.dll");
    if (!nv) return false;
    FreeLibrary(nv);
    return true;
}

static bool is_nvidia_gpu(UINT vendorId)
{
    return (vendorId == 0x10DE);
}

// Reads GPU temperature via NVAPI, trying progressively broader thermal
// targets until one returns a usable sensor reading.
static float get_nvapi_temperature(NvPhysicalGpuHandle handle)
{
    float temperature = -1.0f;

    NV_GPU_THERMAL_SETTINGS thermalSettings = {};
    thermalSettings.version = NV_GPU_THERMAL_SETTINGS_VER;

    NvAPI_Status status =
        NvAPI_GPU_GetThermalSettings(handle, NVAPI_THERMAL_TARGET_ALL, &thermalSettings);

    if (status == NVAPI_OK && thermalSettings.count > 0)
    {
        // Prefer the internal GPU die sensor specifically.
        for (NvU32 i = 0; i < thermalSettings.count; i++)
        {
            if (thermalSettings.sensor[i].controller == NVAPI_THERMAL_CONTROLLER_GPU_INTERNAL &&
                thermalSettings.sensor[i].target == NVAPI_THERMAL_TARGET_GPU)
            {
                return static_cast<float>(thermalSettings.sensor[i].currentTemp);
            }
        }

        if (thermalSettings.sensor[0].currentTemp > 0)
        {
            return static_cast<float>(thermalSettings.sensor[0].currentTemp);
        }
    }

    thermalSettings = {};
    thermalSettings.version = NV_GPU_THERMAL_SETTINGS_VER;
    status = NvAPI_GPU_GetThermalSettings(handle, NVAPI_THERMAL_TARGET_GPU, &thermalSettings);

    if (status == NVAPI_OK && thermalSettings.count > 0)
    {
        return static_cast<float>(thermalSettings.sensor[0].currentTemp);
    }

    thermalSettings = {};
    thermalSettings.version = NV_GPU_THERMAL_SETTINGS_VER;
    status = NvAPI_GPU_GetThermalSettings(handle, NVAPI_THERMAL_TARGET_NONE, &thermalSettings);

    if (status == NVAPI_OK && thermalSettings.count > 0)
    {
        return static_cast<float>(thermalSettings.sensor[0].currentTemp);
    }

    return temperature;
}

// Reads GPU core utilization percentage via NVAPI.
static float get_nvapi_usage(NvPhysicalGpuHandle handle)
{
    NV_GPU_DYNAMIC_PSTATES_INFO_EX pStates = {};
    pStates.version = NV_GPU_DYNAMIC_PSTATES_INFO_EX_VER;

    // utilization[0] is the GPU core domain.
    if (NvAPI_GPU_GetDynamicPstatesInfoEx(handle, &pStates) == NVAPI_OK)
        return static_cast<float>(pStates.utilization[0].percentage);

    return -1.0f;
}

// Reads the real CUDA/shader core count directly from the driver via
// NVAPI. This is the actual, per-GPU value, not an estimate.
static int get_nvapi_core_count(NvPhysicalGpuHandle handle)
{
    NvU32 count = 0;
    if (NvAPI_GPU_GetGpuCoreCount(handle, &count) == NVAPI_OK)
        return static_cast<int>(count);
    return 0;
}

// Reads the current graphics clock via NVAPI.
static float get_nvapi_frequency(NvPhysicalGpuHandle handle)
{
    NvU32 frequency = 0;

    NV_GPU_CLOCK_FREQUENCIES clockFreqs = {};
    clockFreqs.version = NV_GPU_CLOCK_FREQUENCIES_VER;
    clockFreqs.ClockType = NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ;

    NvAPI_Status status = NvAPI_GPU_GetAllClockFrequencies(handle, &clockFreqs);
    if (status == NVAPI_OK)
    {
        if (clockFreqs.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].bIsPresent)
        {
            frequency = clockFreqs.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency;
            if (frequency > 0)
                return static_cast<float>(frequency) / 1000.0f; // kHz to MHz
        }
    }

    // Fall back to the first present clock domain if the graphics
    // domain specifically was not reported.
    NV_GPU_CLOCK_FREQUENCIES allClocks = {};
    allClocks.version = NV_GPU_CLOCK_FREQUENCIES_VER;
    allClocks.ClockType = NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ;

    status = NvAPI_GPU_GetAllClockFrequencies(handle, &allClocks);
    if (status == NVAPI_OK)
    {
        for (int i = 0; i < NVAPI_MAX_GPU_PUBLIC_CLOCKS; i++)
        {
            if (allClocks.domain[i].bIsPresent && allClocks.domain[i].frequency > 0)
            {
                frequency = allClocks.domain[i].frequency;
                return static_cast<float>(frequency) / 1000.0f;
            }
        }
    }

    return -1.0f;
}

// Enumerates all GPUs via DXGI and fills in per-GPU details. NVIDIA GPUs
// get their usage, temperature, core count, and frequency from NVAPI
// (the accurate, vendor-provided source); any GPU where NVAPI data is
// unavailable falls back to the vendor-neutral WMI-based getters above.
vector<gpu_data> GPUInfo::get_all_gpu_info()
{
    vector<gpu_data> list;

    IDXGIFactory6* factory = nullptr;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
        return list;

    bool nvapiInitialized = false;
    NvPhysicalGpuHandle nvapiHandles[NVAPI_MAX_PHYSICAL_GPUS] = {};
    NvU32 nvapiGpuCount = 0;

    if (nvapi_available())
    {
        if (NvAPI_Initialize() == NVAPI_OK)
        {
            nvapiInitialized = true;
            if (NvAPI_EnumPhysicalGPUs(nvapiHandles, &nvapiGpuCount) != NVAPI_OK)
                nvapiGpuCount = 0;
        }
    }

    IDXGIAdapter4* adapter = nullptr;
    UINT adapterIndex = 0;

    for (UINT i = 0;
        factory->EnumAdapters1(i, (IDXGIAdapter1**)&adapter) != DXGI_ERROR_NOT_FOUND;
        ++i)
    {
        DXGI_ADAPTER_DESC3 desc{};
        if (FAILED(adapter->GetDesc3(&desc)))
        {
            adapter->Release();
            continue;
        }

        gpu_data d;

        d.gpu_name = wstr_to_utf8(desc.Description);

        double memGB = static_cast<double>(desc.DedicatedVideoMemory) /
            (1024.0 * 1024.0 * 1024.0);
        ostringstream memStream;
        memStream.precision(1);
        memStream << fixed << memGB;
        d.gpu_memory = memStream.str() + " GB";

        LARGE_INTEGER driverVersion{};
        if (SUCCEEDED(adapter->CheckInterfaceSupport(__uuidof(IDXGIDevice), &driverVersion)))
        {
            ostringstream oss;
            oss << HIWORD(driverVersion.HighPart) << "."
                << LOWORD(driverVersion.HighPart) << "."
                << HIWORD(driverVersion.LowPart) << "."
                << LOWORD(driverVersion.LowPart);
            d.gpu_driver_version = oss.str();
        }
        else
            d.gpu_driver_version = "Unknown";

        d.gpu_vendor =
            (desc.VendorId == 0x10DE) ? "NVIDIA" :
            (desc.VendorId == 0x1002 || desc.VendorId == 0x1022) ? "AMD" :
            (desc.VendorId == 0x8086) ? "Intel" : "Unknown";

        d.gpu_usage = -1.0f;
        d.gpu_temperature = -1.0f;
        d.gpu_core_count = 0;
        d.gpu_frequency = -1.0f;

        if (is_nvidia_gpu(desc.VendorId) && nvapiInitialized && adapterIndex < nvapiGpuCount)
        {
            NvPhysicalGpuHandle handle = nvapiHandles[adapterIndex];
            d.gpu_temperature = get_nvapi_temperature(handle);
            d.gpu_usage = get_nvapi_usage(handle);
            d.gpu_core_count = get_nvapi_core_count(handle);
            d.gpu_frequency = get_nvapi_frequency(handle);
        }

        if (d.gpu_usage < 0.0f)
            d.gpu_usage = get_gpu_usage();
        if (d.gpu_temperature < 0.0f)
            d.gpu_temperature = get_gpu_temperature();
        if (d.gpu_core_count == 0)
            d.gpu_core_count = get_gpu_core_count();

        list.push_back(d);
        adapter->Release();

        if (is_nvidia_gpu(desc.VendorId))
            adapterIndex++;
    }

    if (nvapiInitialized)
        NvAPI_Unload();

    factory->Release();
    return list;
}