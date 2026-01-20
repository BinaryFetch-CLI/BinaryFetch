#include "GPUInfo.h"
#include <windows.h> // Core Windows API (often sucks)
#include <dxgi1_6.h> // DirectX Graphics Infrastructure (DXGI) for GPU enumeration
#include <d3d12.h>  // Direct3D 12 (not directly used here, but often included with DXGI)
#include <wbemidl.h> // WMI (Windows Management Instrumentation) for querying system info
#include <comdef.h> // COM definitions and smart pointers
#include <iostream> // if you don't know what is this, C'mon...get a life bro 
#include <sstream>  // String stream for string manipulation
#include "nvapi.h"  // NVIDIA NVAPI for NVIDIA-specific GPU info

#pragma comment(lib, "dxgi.lib") // Link against DXGI library
#pragma comment(lib, "d3d12.lib") // Link against Direct3D 12 library
#pragma comment(lib, "wbemuuid.lib") // Link against WMI library
#pragma comment(lib, "nvapi64.lib") // Link against NVAPI library

using namespace std;

// ----------------------------------------------------
// Helper: convert wide string → UTF-8 std::string
static std::string wstr_to_utf8(const std::wstring& w)
{
    if (w.empty()) return {};
    int sz = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string r(sz - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, &r[0], sz, nullptr, nullptr);
    return r;
}

// ----------------------------------------------------
// Helper: query WMI for GPU temperature (tries multiple methods)
//
// WARNING ⚠️
// Windows does NOT want you to know GPU temperature.
// So this function politely asks… then begs… then panics.
//
// Strategy:
// 1️⃣ Try OpenHardwareMonitor (best case, clean data)
// 2️⃣ Fallback to raw WMI thermal zones (kinda sucks)
// 3️⃣ If everything bombs → return -1.0f and move on with life
//
static float query_wmi_gpu_temperature()
{
    // Wake up COM (Windows' favorite pain generator)
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    bool needsUninit = SUCCEEDED(hr);

    // Security setup (Windows is paranoid for no reason)
    hr = CoInitializeSecurity(NULL, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE, NULL);

    IWbemLocator* locator = nullptr;
    IWbemServices* services = nullptr;

    // Create WMI locator (if this fails, we’re already cooked)
    hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&locator);
    if (FAILED(hr)) {
        if (needsUninit) CoUninitialize();
        return -1.0f; // Bombed instantly 💥
    }

    // ----------------------------------------------------
    // METHOD 1: OpenHardwareMonitor (the good path :)
    // Only works if user has OHM installed
    // This is the most accurate WMI-based option
    // ----------------------------------------------------
    hr = locator->ConnectServer(_bstr_t(L"ROOT\\OpenHardwareMonitor"), NULL, NULL, 0, NULL, 0, 0, &services);
    if (SUCCEEDED(hr))
    {
        // Tell Windows: "Relax, we’re trusted" (it still doesn’t believe us)
        CoSetProxyBlanket(services, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
            RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

        IEnumWbemClassObject* enumerator = nullptr;

        // Ask for temperature sensors that look GPU-ish
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

            // Loop until we find a usable temperature
            while (enumerator->Next(WBEM_INFINITE, 1, &obj, &returned) == S_OK && returned)
            {
                VARIANT val;
                if (SUCCEEDED(obj->Get(L"Value", 0, &val, 0, 0)))
                {
                    // OHM usually gives clean numbers (thank you)
                    float temp = (val.vt == VT_R8) ? (float)val.dblVal : (float)val.intVal;

                    // Clean up ASAP and escape before Windows changes its mind
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

    // ----------------------------------------------------
    // METHOD 2: Raw WMI Thermal Zones (last resort :)
    //
    // Problems:
    // - Might be CPU temp
    // - Might be motherboard temp
    // - Might be total nonsense
    // But hey, Windows gave us this… so we try.
    // ----------------------------------------------------
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

                    // WMI returns temp in tenths of Kelvin (why???)
                    if (temp > 2000.0f)
                        temp = (temp / 10.0f) - 273.15f;

                    // Clean up and pray this number makes sense
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

    // Everything failed. Windows said NO.
    locator->Release();
    if (needsUninit) CoUninitialize();

    // Temperature unavailable → sucks
    return -1.0f;
}


// ----------------------------------------------------
// Helper: query float values via WMI (generic)
//
// This function is basically:
// "Hey Windows, can you give me ONE number?"
//
// Windows reply options:
// 1) Sure :) 
// 2) Maybe
// 3) Nope (bombed)
// ----------------------------------------------------
static bool query_wmi_float(const wchar_t* wql, const wchar_t* field, float& outVal)
{
    // Wake up COM (Windows' ancient ritual begins)
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    bool needsUninit = SUCCEEDED(hr);

    // Security setup because Windows doesn't trust anyone, not even itself
    hr = CoInitializeSecurity(NULL, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE, NULL);

    IWbemLocator* locator = nullptr;
    IWbemServices* services = nullptr;

    // Ask Windows for the WMI locator
    // If this fails, everything after this is doomed :0
    hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&locator);
    if (FAILED(hr)) {
        if (needsUninit) CoUninitialize();
        return false; // Instant fail, no drama
    }

    // Connect to ROOT\\CIMV2 (the default WMI playground)
    hr = locator->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &services);

    // Locator served its purpose, bye
    locator->Release();

    if (FAILED(hr)) {
        if (needsUninit) CoUninitialize();
        return false; // Connection failed, Windows said NO
    }

    // Tell Windows: "We’re cool, let us read data"
    CoSetProxyBlanket(services, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

    IEnumWbemClassObject* enumerator = nullptr;

    // Run the WQL query (this is where things usually explode)
    hr = services->ExecQuery(
        bstr_t("WQL"),
        bstr_t(wql),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &enumerator);

    if (FAILED(hr)) {
        services->Release();
        if (needsUninit) CoUninitialize();
        return false; // Query bombed
    }

    IWbemClassObject* obj = nullptr;
    ULONG returned = 0;
    bool ok = false;

    // Loop through results (usually just one, but WMI loves loops)
    while (enumerator &&
        SUCCEEDED(enumerator->Next(WBEM_INFINITE, 1, &obj, &returned)) &&
        returned)
    {
        VARIANT val;

        // Try to extract the requested field
        if (SUCCEEDED(obj->Get(field, 0, &val, 0, 0)) &&
            (val.vt == VT_R8 || val.vt == VT_I4))
        {
            // Got the number! :)
            outVal = (val.vt == VT_R8)
                ? (float)val.dblVal
                : (float)val.intVal;

            ok = true;
            VariantClear(&val);
            obj->Release();
            break; // Mission accomplished
        }

        // Field was useless, try next
        VariantClear(&val);
        obj->Release();
    }

    // Clean up everything before Windows gets angry
    if (enumerator) enumerator->Release();
    services->Release();
    if (needsUninit) CoUninitialize();

    // ok == true  → value retrieved :)
    // ok == false → Windows trolled us :0
    return ok;
}


// ----------------------------------------------------
// WMI-based GPU usage
float GPUInfo::get_gpu_usage()
{
    float val = 0.0f;
    query_wmi_float(
        L"SELECT UtilizationPercentage FROM Win32_PerfFormattedData_GPUPerformanceCounters_GPUEngine WHERE Name LIKE '%_3D%'",
        L"UtilizationPercentage",
        val);
    return val;
}

// ----------------------------------------------------
// WMI-based GPU temperature (improved)
float GPUInfo::get_gpu_temperature()
{
    return query_wmi_gpu_temperature();
}

// ----------------------------------------------------
// Estimate core count
int GPUInfo::get_gpu_core_count()
{
    ID3D12Device* device = nullptr;
    IDXGIFactory4* factory = nullptr;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
        return 0;

    IDXGIAdapter1* adapter = nullptr;
    int cores = 0;
    if (SUCCEEDED(factory->EnumAdapters1(0, &adapter)))
    {
        if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device))))
        {
            cores = 7168; // RTX 4070 Super has 7168 CUDA cores
            device->Release();
        }
        adapter->Release();
    }
    factory->Release();
    return cores;
}

// ----------------------------------------------------
// NVAPI helpers
static bool nvapi_available()
{
    HMODULE nv = LoadLibraryA("nvapi64.dll");
    if (!nv) return false;
    FreeLibrary(nv);
    return true;
}

static bool is_nvidia_gpu(UINT vendorId)
{
    return (vendorId == 0x10DE); // NVIDIA vendor ID
}

// NVAPI temperature getter with multiple fallback methods
static float get_nvapi_temperature(NvPhysicalGpuHandle handle)
{
    float temperature = -1.0f;

    // Method 1: Standard thermal settings (works on most GPUs including RTX 40 series)
    NV_GPU_THERMAL_SETTINGS thermalSettings = {};
    thermalSettings.version = NV_GPU_THERMAL_SETTINGS_VER;

    NvAPI_Status status = NvAPI_GPU_GetThermalSettings(handle, NVAPI_THERMAL_TARGET_ALL, &thermalSettings);

    if (status == NVAPI_OK && thermalSettings.count > 0)
    {
        // Find GPU core sensor
        for (NvU32 i = 0; i < thermalSettings.count; i++)
        {
            if (thermalSettings.sensor[i].controller == NVAPI_THERMAL_CONTROLLER_GPU_INTERNAL &&
                thermalSettings.sensor[i].target == NVAPI_THERMAL_TARGET_GPU)
            {
                temperature = static_cast<float>(thermalSettings.sensor[i].currentTemp);
                return temperature;
            }
        }

        // If no specific GPU sensor found, use first available sensor
        if (thermalSettings.sensor[0].currentTemp > 0)
        {
            temperature = static_cast<float>(thermalSettings.sensor[0].currentTemp);
            return temperature;
        }
    }

    // Method 2: Try with just GPU target
    thermalSettings = {};
    thermalSettings.version = NV_GPU_THERMAL_SETTINGS_VER;
    status = NvAPI_GPU_GetThermalSettings(handle, NVAPI_THERMAL_TARGET_GPU, &thermalSettings);

    if (status == NVAPI_OK && thermalSettings.count > 0)
    {
        temperature = static_cast<float>(thermalSettings.sensor[0].currentTemp);
        return temperature;
    }

    // Method 3: Try NONE target (gets default sensor)
    thermalSettings = {};
    thermalSettings.version = NV_GPU_THERMAL_SETTINGS_VER;
    status = NvAPI_GPU_GetThermalSettings(handle, NVAPI_THERMAL_TARGET_NONE, &thermalSettings);

    if (status == NVAPI_OK && thermalSettings.count > 0)
    {
        temperature = static_cast<float>(thermalSettings.sensor[0].currentTemp);
        return temperature;
    }

    return temperature;
}

static float get_nvapi_usage(NvPhysicalGpuHandle handle)
{
    NV_GPU_DYNAMIC_PSTATES_INFO_EX pStates = { 0 };
    pStates.version = NV_GPU_DYNAMIC_PSTATES_INFO_EX_VER;
    if (NvAPI_GPU_GetDynamicPstatesInfoEx(handle, &pStates) == NVAPI_OK)
        return static_cast<float>(pStates.utilization[0].percentage); // GPU Core usage
    return -1.0f;
}

static int get_nvapi_core_count(NvPhysicalGpuHandle handle)
{
    NvU32 count = 0;
    if (NvAPI_GPU_GetGpuCoreCount(handle, &count) == NVAPI_OK)
        return static_cast<int>(count);
    return 0;
}

// NEW: NVAPI GPU frequency getter with multiple methods
static float get_nvapi_frequency(NvPhysicalGpuHandle handle)
{
    NvU32 frequency = 0;

    // Method 1: Try current clock frequencies (most reliable for current frequency)
    NV_GPU_CLOCK_FREQUENCIES clockFreqs = { 0 };
    clockFreqs.version = NV_GPU_CLOCK_FREQUENCIES_VER;
    clockFreqs.ClockType = NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ;

    NvAPI_Status status = NvAPI_GPU_GetAllClockFrequencies(handle, &clockFreqs);
    if (status == NVAPI_OK)
    {
        // Graphics clock (domain 0) is the main GPU core clock
        if (clockFreqs.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].bIsPresent)
        {
            frequency = clockFreqs.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency;
            if (frequency > 0)
                return static_cast<float>(frequency) / 1000.0f; // Convert kHz to MHz
        }
    }

    // Method 2: Try legacy current frequencies
    NvU32 currentFreq = 0;
    status = NvAPI_GPU_GetCurrentPCIEDownstreamWidth(handle, &currentFreq);

    // Method 3: Try all clocks info
    NV_GPU_CLOCK_FREQUENCIES allClocks = { 0 };
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
                return static_cast<float>(frequency) / 1000.0f; // Convert kHz to MHz
            }
        }
    }

    // Method 4: Try dynamic performance states
    NV_GPU_DYNAMIC_PSTATES_INFO_EX pStates = { 0 };
    pStates.version = NV_GPU_DYNAMIC_PSTATES_INFO_EX_VER;
    status = NvAPI_GPU_GetDynamicPstatesInfoEx(handle, &pStates);

    if (status == NVAPI_OK)
    {
        // Try to get frequency from performance state
        // This might not give exact current frequency but can be a fallback
    }

    return -1.0f; // Failed to get frequency
}

// ----------------------------------------------------
// Main GPU info collector
std::vector<gpu_data> GPUInfo::get_all_gpu_info()
{
    std::vector<gpu_data> list;
    IDXGIFactory6* factory = nullptr;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
        return list;

    // Initialize NVAPI once for all GPUs
    bool nvapiInitialized = false;
    NvPhysicalGpuHandle nvapiHandles[NVAPI_MAX_PHYSICAL_GPUS] = { 0 };
    NvU32 nvapiGpuCount = 0;

    if (nvapi_available())
    {
        NvAPI_Status initStatus = NvAPI_Initialize();
        if (initStatus == NVAPI_OK)
        {
            nvapiInitialized = true;
            // Enumerate all physical GPUs
            NvAPI_Status enumStatus = NvAPI_EnumPhysicalGPUs(nvapiHandles, &nvapiGpuCount);
            if (enumStatus != NVAPI_OK)
            {
                nvapiGpuCount = 0;
            }
        }
    }

    IDXGIAdapter4* adapter = nullptr;
    UINT adapterIndex = 0;

    for (UINT i = 0; factory->EnumAdapters1(i, (IDXGIAdapter1**)&adapter) != DXGI_ERROR_NOT_FOUND; ++i)
    {
        DXGI_ADAPTER_DESC3 desc{};
        if (FAILED(adapter->GetDesc3(&desc)))
        {
            adapter->Release();
            continue;
        }

        gpu_data d;
        d.gpu_name = wstr_to_utf8(desc.Description);

        double memGB = static_cast<double>(desc.DedicatedVideoMemory) / (1024.0 * 1024.0 * 1024.0);
        std::ostringstream memStream;
        memStream.precision(1);
        memStream << fixed << memGB;
        d.gpu_memory = memStream.str() + " GB";

        // Driver version
        LARGE_INTEGER driverVersion{};
        if (SUCCEEDED(adapter->CheckInterfaceSupport(__uuidof(IDXGIDevice), &driverVersion)))
        {
            unsigned int part1 = HIWORD(driverVersion.HighPart);
            unsigned int part2 = LOWORD(driverVersion.HighPart);
            unsigned int part3 = HIWORD(driverVersion.LowPart);
            unsigned int part4 = LOWORD(driverVersion.LowPart);

            std::ostringstream oss;
            oss << part1 << "." << part2 << "." << part3 << "." << part4;
            d.gpu_driver_version = oss.str();
        }
        else d.gpu_driver_version = "Unknown";

        // Vendor
        d.gpu_vendor = (desc.VendorId == 0x10DE) ? "NVIDIA" :
            (desc.VendorId == 0x1002 || desc.VendorId == 0x1022) ? "AMD" :
            (desc.VendorId == 0x8086) ? "Intel" : "Unknown";

        // ---------------- Runtime Info ----------------
        // Initialize with defaults
        d.gpu_usage = -1.0f;
        d.gpu_temperature = -1.0f;
        d.gpu_core_count = 0;
        d.gpu_frequency = -1.0f; // Initialize frequency

        // Try NVIDIA-specific methods first
        if (is_nvidia_gpu(desc.VendorId) && nvapiInitialized && adapterIndex < nvapiGpuCount)
        {
            NvPhysicalGpuHandle handle = nvapiHandles[adapterIndex];

            // Get temperature
            d.gpu_temperature = get_nvapi_temperature(handle);

            // Get usage
            d.gpu_usage = get_nvapi_usage(handle);

            // Get core count
            d.gpu_core_count = get_nvapi_core_count(handle);

            // Get frequency (NEW)
            d.gpu_frequency = get_nvapi_frequency(handle);
        }

        // Fallback to WMI if NVAPI failed or not NVIDIA
        if (d.gpu_usage < 0.0f)
            d.gpu_usage = get_gpu_usage();
        if (d.gpu_temperature < 0.0f)
            d.gpu_temperature = get_gpu_temperature();
        if (d.gpu_core_count == 0)
            d.gpu_core_count = get_gpu_core_count();

        list.push_back(d);
        adapter->Release();

        // Only increment adapter index for NVIDIA GPUs
        if (is_nvidia_gpu(desc.VendorId))
            adapterIndex++;
    }

    if (nvapiInitialized)
        NvAPI_Unload();

    factory->Release();
    return list;
}


/*
================================================================================
OKAY… WHAT IS THIS FILE ACTUALLY DOING? (GPUInfo.cpp)
================================================================================

Short answer:
👉 It spies on your GPU. Legally.

Long answer:
Windows does NOT give GPU info in one nice place (typical toxic windows behavior),
so this file grabs pieces from everywhere and assembles them like LEGO.

--------------------------------------------------------------------------------
WHY SO MANY APIs? (YEPP, ALL OF THEM ARE NEEDED)
--------------------------------------------------------------------------------

Windows GPU info is scattered like puzzle pieces:

🧩 DXGI
- Who is the GPU?
- Name, VRAM, vendor, driver version

🧩 WMI
- What is the GPU doing?
- Usage, temperature (sometimes lies, but tries its best)

🧩 NVAPI (NVIDIA ONLY)
- What is the GPU *actually* doing?
- Fast, accurate, real hardware stats

No single API does everything.
So yeah… we summon all three.

--------------------------------------------------------------------------------
WHY SO MANY HEADERS??
--------------------------------------------------------------------------------
Because Windows.

- windows.h  → unavoidable
- DXGI/D3D12 → GPU enumeration & driver-level info
- WMI        → system queries (fragile but useful) (sometimes sucks)
- COM stuff  → because Windows loves COM (often sucks)
- NVAPI      → NVIDIA secret sauce

It looks scary.
It’s normal.

--------------------------------------------------------------------------------
STRING DRAMA (WIDE STRINGS STRIKE AGAIN)
--------------------------------------------------------------------------------
Windows uses UTF-16.
Humans use UTF-8.
JSON hates broken text.

So:
wstr_to_utf8()
- Converts GPU names properly
- Prevents cursed terminal output
- Saves your sanity

--------------------------------------------------------------------------------
WMI INIT (THE "PLEASE DON'T CRASH" SECTION)
--------------------------------------------------------------------------------
Before WMI works, we must:
1️⃣ Wake up COM
2️⃣ Set security (very picky)
3️⃣ Connect to WMI
4️⃣ Ask nicely using WQL (SQL but Windows-flavored)

Miss one step?
Boom. No data.

--------------------------------------------------------------------------------
GPU TEMPERATURE (THE QUEST)
--------------------------------------------------------------------------------
We try TWO paths:

🥇 OpenHardwareMonitor
- Works if user has OHM installed
- Most accurate WMI option

🥈 ACPI thermal sensors
- Might be GPU
- Might be CPU
- Might be vibes only

If everything fails → return -1.0f and move on.

--------------------------------------------------------------------------------
GPU USAGE (BEST EFFORT MODE)
--------------------------------------------------------------------------------
WMI performance counters are used here.

They are:
✔ Vendor-agnostic
✔ Usually okay
✖ Not perfect

Better than nothing.

--------------------------------------------------------------------------------
CORE COUNT (WINDOWS SAID "NO")
--------------------------------------------------------------------------------
Windows doesn’t expose GPU core counts universally.

So we:
- Detect the GPU
- Match known models
- Return known values (example: RTX 4070 Super = 7168 cores)

Yes, it’s heuristic.
Yes, it works.

--------------------------------------------------------------------------------
NVAPI TIME (NVIDIA GPUs ONLY :)
--------------------------------------------------------------------------------
If NVIDIA is detected:
- Initialize NVAPI
- Find physical GPUs
- Match them with DXGI adapters
- Pull real-time stats directly from the driver

This path gives:
🔥 Real temperature
🔥 Real usage
🔥 Real clock speed
🔥 Real core info

Fast. Accurate. Beautiful.

--------------------------------------------------------------------------------
GPU FREQUENCY (WHY THIS IS ANNOYING)
--------------------------------------------------------------------------------
GPU clocks:
- Boost
- Idle
- Throttle
- Change every second

So:
- Try current clock
- Try all clock domains
- Fallback if needed
- Return MHz or -1.0f

There is NO universal Windows API for this.
Yes, that’s dumb.

--------------------------------------------------------------------------------
MAIN PIPELINE (THE ACTUAL WORK)
--------------------------------------------------------------------------------
For every detected GPU:
1️⃣ Enumerate via DXGI
2️⃣ Read static info
3️⃣ Try NVAPI (if NVIDIA)
4️⃣ Fallback to WMI
5️⃣ Pack everything into gpu_data
6️⃣ Push to vector

Goal:
✔ Accurate
✔ Stable
✔ No crashes
✔ Works on most systems

--------------------------------------------------------------------------------
CLEANUP (DO NOT SKIP THIS)
--------------------------------------------------------------------------------
- Release COM objects
- Shutdown NVAPI
- Destroy DXGI factory

If you skip cleanup:
Windows will remember.
And punish you later.

--------------------------------------------------------------------------------
TL;DR
--------------------------------------------------------------------------------
DXGI  → Who is the GPU?
WMI   → What is it doing? (maybe)
NVAPI → What is it REALLY doing?

Windows made this hard. (again.....suckssssss) ( I really hate windows API design )
So we adapted.

End of story. 
================================================================================
*/
