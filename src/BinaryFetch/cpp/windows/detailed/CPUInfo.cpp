#include "CPUInfo.h"

#include <windows.h>
#include <intrin.h>
#include <vector>
#include <sstream>
#include <wbemidl.h>
#include <pdh.h>
#include <comdef.h>
#include <iomanip>
using namespace std;

#pragma comment(lib, "pdh.lib")
#pragma comment(lib, "wbemuuid.lib")

using namespace std;

// WMI helper: query single value from WMI
string wmi_querysingle_value(const wchar_t* query, const wchar_t* property_name)
{
    HRESULT hres;
    IWbemLocator* locator = NULL;
    IWbemServices* services = NULL;
    IEnumWbemClassObject* enumerator = NULL;
    IWbemClassObject* clsObj = NULL;
    ULONG uReturn = 0;

    string result = "Unknown";

    // Initialize COM
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
    {
        hres = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
        if (FAILED(hres))
            return result;
    }

    // Set security
    hres = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL
    );

    if (FAILED(hres) && hres != RPC_E_TOO_LATE)
    {
        CoUninitialize();
        return result;
    }

    // Create WMI locator
    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID*)&locator
    );
    if (FAILED(hres))
    {
        CoUninitialize();
        return result;
    }

    // Connect to WMI
    hres = locator->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL, NULL, 0, NULL, 0, 0,
        &services
    );
    if (FAILED(hres))
    {
        locator->Release();
        CoUninitialize();
        return result;
    }

    // Set proxy blanket
    hres = CoSetProxyBlanket(
        services,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE
    );
    if (FAILED(hres))
    {
        services->Release();
        locator->Release();
        CoUninitialize();
        return result;
    }

    // Execute query
    hres = services->ExecQuery(
        _bstr_t(L"WQL"),
        _bstr_t(query),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &enumerator
    );
    if (FAILED(hres))
    {
        services->Release();
        locator->Release();
        CoUninitialize();
        return result;
    }

    // Extract result
    if (enumerator)
    {
        while (enumerator->Next(WBEM_INFINITE, 1, &clsObj, &uReturn) == S_OK && uReturn > 0)
        {
            VARIANT vtProp;
            VariantInit(&vtProp);

            hres = clsObj->Get(property_name, 0, &vtProp, 0, 0);
            if (SUCCEEDED(hres))
            {
                if (vtProp.vt == VT_BSTR && vtProp.bstrVal)
                    result = _bstr_t(vtProp.bstrVal);
                else if (vtProp.vt == VT_I4)
                    result = to_string(vtProp.intVal);
                else if (vtProp.vt == VT_UI4)
                    result = to_string(vtProp.uintVal);
                else if (vtProp.vt == VT_UI2)
                    result = to_string(vtProp.uiVal);

                VariantClear(&vtProp);
            }

            clsObj->Release();
            break;
        }

        enumerator->Release();
    }

    // Cleanup
    if (services) services->Release();
    if (locator) locator->Release();
    CoUninitialize();

    return result;
}

// Get CPU brand string using CPUID
string CPUInfo::get_cpu_info()
{
    int cpu_data[4] = { -1 };
    char cpu_brand[0x40] = { 0 };

    __cpuid(cpu_data, 0x80000002);
    memcpy(cpu_brand, cpu_data, sizeof(cpu_data));

    __cpuid(cpu_data, 0x80000003);
    memcpy(cpu_brand + 16, cpu_data, sizeof(cpu_data));

    __cpuid(cpu_data, 0x80000004);
    memcpy(cpu_brand + 32, cpu_data, sizeof(cpu_data));

    return string(cpu_brand);
}

// Get CPU utilization using PDH
float CPUInfo::get_cpu_utilization()
{
    static PDH_HQUERY query = NULL;
    static PDH_HCOUNTER counter = NULL;
    static bool initialized = false;

    if (!initialized)
    {
        PdhOpenQuery(NULL, 0, &query);
        PdhAddCounter(query, TEXT("\\Processor(_Total)\\% Processor Time"), 0, &counter);
        PdhCollectQueryData(query);
        initialized = true;

        Sleep(100);
    }

    PDH_FMT_COUNTERVALUE value;
    PdhCollectQueryData(query);
    PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL, &value);

    return static_cast<float>(value.doubleValue);
}

// Get base clock speed
string CPUInfo::get_cpu_base_speed()
{
    string value = wmi_querysingle_value(
        L"SELECT MaxClockSpeed FROM Win32_Processor",
        L"MaxClockSpeed"
    );

    if (value == "Unknown" || value.empty()) return "N/A";

    try
    {
        float ghz = stof(value) / 1000.0f;
        ostringstream ss;
        ss << fixed << setprecision(2) << ghz << " GHz";
        return ss.str();
    }
    catch (...)
    {
        return "N/A";
    }
}

// Get current clock speed
string CPUInfo::get_cpu_speed()
{
    string value = wmi_querysingle_value
    (
        L"SELECT CurrentClockSpeed FROM Win32_Processor",
        L"CurrentClockSpeed"
    );

    if (value == "Unknown" || value.empty()) return "N/A";

    try
    {
        float ghz = stof(value) / 1000.0f;
        ostringstream ss;
        ss << fixed << setprecision(2) << ghz << " GHz";
        return ss.str();
    }
    catch (...)
    {
        return "N/A";
    }
}

// Get number of physical CPU sockets
int CPUInfo::get_cpu_sockets()
{
    string value = wmi_querysingle_value
    (
        L"SELECT COUNT(*) FROM Win32_Processor",
        L"COUNT(*)"
    );

    try { return stoi(value); }
    catch (...) { return 1; }
}

// Get number of physical cores
int CPUInfo::get_cpu_cores()
{
    DWORD length = 0;
    GetLogicalProcessorInformation(NULL, &length);

    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        return -1;

    vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer(
        length / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION)
    );

    if (!GetLogicalProcessorInformation(buffer.data(), &length))
        return -1;

    int cores = 0;
    for (auto& info : buffer)
        if (info.Relationship == RelationProcessorCore)
            cores++;

    return cores;
}

// Get number of logical processors
int CPUInfo::get_cpu_logical_processors()
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwNumberOfProcessors;
}

// Get virtualization status
string CPUInfo::get_cpu_virtualization()
{
    return IsProcessorFeaturePresent(PF_VIRT_FIRMWARE_ENABLED)
        ? "Enabled"
        : "Disabled";
}

// Get L1 cache size
string CPUInfo::get_cpu_l1_cache()
{
    DWORD length = 0;
    GetLogicalProcessorInformation(NULL, &length);

    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        return "N/A";

    vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer
    (
        length / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION)
    );

    if (!GetLogicalProcessorInformation(buffer.data(), &length))
        return "N/A";

    DWORD size = 0;
    for (auto& info : buffer)
        if (info.Relationship == RelationCache && info.Cache.Level == 1)
            size += info.Cache.Size;

    if (!size) return "N/A";

    ostringstream ss;
    ss << (size / 1024) << " KB";
    return ss.str();
}

// Get L2 cache size
string CPUInfo::get_cpu_l2_cache()
{
    DWORD length = 0;
    GetLogicalProcessorInformation(NULL, &length);

    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        return "N/A";

    vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer(
        length / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION)
    );

    if (!GetLogicalProcessorInformation(buffer.data(), &length))
        return "N/A";

    DWORD size = 0;
    for (auto& info : buffer)
        if (info.Relationship == RelationCache && info.Cache.Level == 2)
            size += info.Cache.Size;

    if (!size) return "N/A";

    ostringstream ss;
    ss << (size >= 1024 * 1024 ? size / (1024 * 1024) : size / 1024)
        << (size >= 1024 * 1024 ? " MB" : " KB");
    return ss.str();
}

// Get L3 cache size
string CPUInfo::get_cpu_l3_cache()
{
    DWORD length = 0;
    GetLogicalProcessorInformation(NULL, &length);

    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        return "N/A";

    vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer
    (
        length / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION)
    );

    if (!GetLogicalProcessorInformation(buffer.data(), &length))
        return "N/A";

    DWORD size = 0;
    for (auto& info : buffer)
        if (info.Relationship == RelationCache && info.Cache.Level == 3)
            size += info.Cache.Size;

    if (!size) return "N/A";

    ostringstream ss;
    ss << (size >= 1024 * 1024 ? size / (1024 * 1024) : size / 1024)
        << (size >= 1024 * 1024 ? " MB" : " KB");
    return ss.str();
}

// Get system uptime
string CPUInfo::get_system_uptime()
{
    ULONGLONG ms = GetTickCount64();

    ULONGLONG seconds = ms / 1000;
    ULONGLONG minutes = seconds / 60;
    ULONGLONG hours = minutes / 60;
    ULONGLONG days = hours / 24;

    ostringstream ss;
    ss << days << ":"
        << setw(2) << setfill('0') << (hours % 24) << ":"
        << setw(2) << (minutes % 60) << ":"
        << setw(2) << (seconds % 60);

    return ss.str();
}

// Get running process count
int CPUInfo::get_process_count()
{
    string value = wmi_querysingle_value
    (
        L"SELECT COUNT(*) FROM Win32_Process",
        L"COUNT(*)"
    );

    try { return stoi(value); }
    catch (...) { return 0; }
}

// Get total system thread count
int CPUInfo::get_thread_count()
{
    string value = wmi_querysingle_value
    (
        L"SELECT ThreadCount FROM Win32_PerfFormattedData_PerfProc_Process WHERE Name='_Total'",
        L"ThreadCount"
    );

    try { return stoi(value); }
    catch (...) { return 0; }
}

// Get total system handle count
int CPUInfo::get_handle_count()
{
    string value = wmi_querysingle_value
    (
        L"SELECT HandleCount FROM Win32_PerfFormattedData_PerfProc_Process WHERE Name='_Total'",
        L"HandleCount"
    );

    try { return stoi(value); }
    catch (...) { return 0; }
}