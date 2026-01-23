/*
================================================================================
 CPUInfo.cpp — What’s going on in this file?
================================================================================

This file is responsible for collecting CPU + system runtime information on
Windows — basically recreating what Task Manager shows, but using raw APIs
instead of fancy UI magic.

We mix and match multiple Windows technologies here because Windows does NOT
give everything in one place (classic Windows moment).

What we use:
-------------
1) CPUID (intrinsics)
   - For CPU brand / model name
   - Fast, direct, and very reliable

2) WMI (Windows Management Instrumentation)
   - Used for:
	 • Base clock speed
	 • Current clock speed
	 • Socket count
	 • Process count
	 • Thread count
	 • Handle count
   - Powerful but slow and kinda sucks sometimes
   - Still the only way to get some info cleanly

3) PDH (Performance Data Helper)
   - Used for real-time CPU utilization (%)
   - Same backend Task Manager uses
   - Needs warm-up or it gives garbage data

4) WinAPI system calls
   - Core count
   - Logical processors (threads)
   - Cache sizes (L1, L2, L3)
   - System uptime
   - Virtualization status

Why this looks complicated:
----------------------------
Because Windows spreads CPU info across:
- CPUID
- WMI
- PDH
- Kernel APIs

So yeah… we stitched it all together manually :)))

================================================================================
 Function overview
================================================================================

wmi_querysingle_value()
----------------------
A helper that runs a WMI query and returns ONE value as a string.
Used everywhere to avoid rewriting the same COM + WMI boilerplate.

get_cpu_info()
--------------
Uses CPUID to grab the full CPU brand string
(Exactly what Task Manager shows)

get_cpu_utilization()
---------------------
Uses PDH counters to get real-time CPU usage (%).
This needs initialization + a delay or Windows lies :)

get_cpu_base_speed()
--------------------
Reads MaxClockSpeed from WMI and converts MHz → GHz.

get_cpu_speed()
---------------
Reads CurrentClockSpeed from WMI (current boost clock).

get_cpu_sockets()
-----------------
Counts how many physical CPU sockets exist.
Usually 1 unless you’re running a server monster.

get_cpu_cores()
---------------
Counts PHYSICAL cores using GetLogicalProcessorInformation.
Does NOT count hyperthreads.

get_cpu_logical_processors()
----------------------------
Returns total logical processors (threads).
This is cores × SMT.

get_cpu_virtualization()
------------------------
Checks if CPU virtualization is enabled in BIOS / firmware.

get_cpu_l1_cache()
get_cpu_l2_cache()
get_cpu_l3_cache()
------------------
Reads cache sizes using processor topology info.
Automatically formats KB / MB.

get_system_uptime()
-------------------
Uses GetTickCount64() to calculate how long the system
has been running (days:hours:minutes:seconds).

get_process_count()
-------------------
Counts total running processes via WMI.

get_thread_count()
------------------
Counts total system threads via performance counters.

get_handle_count()
------------------
Counts total open OS handles (files, objects, etc).

================================================================================
 TL;DR
================================================================================
This file:
- Talks directly to the OS
- Avoids external libraries
- Mimics Task Manager behavior
- Trades simplicity for accuracy and control

Yes, it’s long.
Yes, Windows APIs are messy.
But it WORKS — and that’s a win :)
================================================================================
*/







#include "CPUInfo.h"

#include <windows.h>   // Core Windows API — sometimes pain, sometimes power
#include <intrin.h>    // CPUID and low-level CPU instructions
#include <vector>      // Dynamic storage (because life isn’t fixed-size)
#include <sstream>     // Turning numbers into pretty strings
#include <wbemidl.h>   // WMI — Windows answering deep existential questions
#include <pdh.h>       // Performance counters (Task Manager vibes)
#include <comdef.h>    // COM helpers so we don’t lose our sanity
#include <iomanip>     // Formatting polish (decimals, padding, alignment)

#pragma comment(lib, "pdh.lib")      
// Auto-link PDH so CPU usage works without linker drama

#pragma comment(lib, "wbemuuid.lib") 
// Required for WMI / COM UUIDs — Windows won’t talk without this

using namespace std; // If this confuses you… we need to talk 😄


/*
    ------------------------------------------------------------
    WMI Helper Function
    ------------------------------------------------------------

    Runs a WMI query and returns ONE property as a string.

    Philosophy:
    - Ask Windows nicely
    - Take one answer
    - Leave quietly
*/
string wmi_querysingle_value(const wchar_t* query, const wchar_t* property_name)
{
    // COM & WMI plumbing (boring but unavoidable)
    HRESULT hres;
    IWbemLocator* locator = NULL;           // Finds the WMI service
    IWbemServices* services = NULL;         // Talks to WMI
    IEnumWbemClassObject* enumerator = NULL;// Iterates query results
    IWbemClassObject* clsObj = NULL;        // One result object
    ULONG uReturn = 0;

    // Default answer when Windows refuses cooperation :)
    string result = "Unknown";

    // Initialize COM (prefer multithreaded)
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
    {
        // Fallback — COM was probably already initialized differently
        hres = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
        if (FAILED(hres))
            return result;
    }

// Don't get afraid 😄 — this just tells Windows:
// "use default security, auto-pick auth services, and allow WMI to
// query system info using the current user".
// The weird NULLs and -1 are totally normal here — Windows expects them.


    /*
    ### This function call looks scary.... It’s not..... :)

    CoInitializeSecurity configures how COM (and WMI) handle security.
    The reason this call looks like a pile of NULLs and magic numbers is
    because Microsoft designed it to be *generic*, not friendly.

    Why these exact values?

    - NULL (1st parameter):
      We are NOT providing a custom security descriptor.
      Translation: "Windows, use your default security rules."

    - -1 (2nd parameter):
      Means "use ALL available authentication services".
      We don't care which one — Windows will pick what works best.

    - NULL, NULL (3rd & 4th):
      No custom authentication services, no reserved data.
      Again: defaults are perfectly fine for reading system info.

    - RPC_C_AUTHN_LEVEL_DEFAULT:
      Default authentication level.
      Enough security to safely talk to WMI without overcomplicating things.

    - RPC_C_IMP_LEVEL_IMPERSONATE:
      This is IMPORTANT.
      It allows WMI to query system information *as the current user*.
      Without this, most WMI calls will silently fail.....

    - NULL:
      No custom authentication identity.
      We’re using the current logged-in user.

    - EOAC_NONE:
      No extra COM capabilities needed.
      Simple, clean, no special tricks.

    - NULL (last one):
      Reserved. Always NULL. Always weird. Always Microsoft.

    TL;DR:
    This setup basically says:
    "Hey Windows, please let me read system information safely....
     using default rules, as the current user."

    It’s boilerplate. It’s normal. And yes — every sane WMI program does this.
*/

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

    // RPC_E_TOO_LATE just means security was already set — totally fine
    if (FAILED(hres) && hres != RPC_E_TOO_LATE)
    {
        CoUninitialize();
        return result;
    }

    // Create WMI locator (basically a GPS for Windows internals)
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

    // Connect to ROOT\\CIMV2 — the good stuff lives here
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

    // Set proxy blanket so we’re allowed to read data
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

    // Execute the WQL query (fast + forward-only)
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

    // Grab the first result — one value is all we need
    if (enumerator)
    {
        while (enumerator->Next(WBEM_INFINITE, 1, &clsObj, &uReturn) == S_OK && uReturn > 0)
        {
            VARIANT vtProp;
            VariantInit(&vtProp);

            hres = clsObj->Get(property_name, 0, &vtProp, 0, 0);
            if (SUCCEEDED(hres))
            {
                // Handle common WMI data types
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
            break; // Mission accomplished :)
        }

        enumerator->Release();
    }

    // Clean exit — no COM leaks allowed
    if (services) services->Release();
    if (locator) locator->Release();
    CoUninitialize();

    return result;
}


/*
    ------------------------------------------------------------
    CPUInfo implementation
    ------------------------------------------------------------
*/

// Returns CPU brand string (same as Task Manager)
string CPUInfo::get_cpu_info()
{
    int cpu_data[4] = { -1 };
    char cpu_brand[0x40] = { 0 };

    // CPUID magic — brand string lives across these calls
    __cpuid(cpu_data, 0x80000002);
    memcpy(cpu_brand, cpu_data, sizeof(cpu_data));

    __cpuid(cpu_data, 0x80000003);
    memcpy(cpu_brand + 16, cpu_data, sizeof(cpu_data));

    __cpuid(cpu_data, 0x80000004);
    memcpy(cpu_brand + 32, cpu_data, sizeof(cpu_data));

    return string(cpu_brand);
}

// CPU usage percentage (Task Manager style)
float CPUInfo::get_cpu_utilization()
{
    static PDH_HQUERY query = NULL;
    static PDH_HCOUNTER counter = NULL;
    static bool initialized = false;

    // One-time PDH setup
    if (!initialized)
    {
        PdhOpenQuery(NULL, 0, &query);
        PdhAddCounter(query, TEXT("\\Processor(_Total)\\% Processor Time"), 0, &counter);
        PdhCollectQueryData(query);
        initialized = true;

        // PDH needs a short delay to stabilize
        Sleep(100);
    }

    PDH_FMT_COUNTERVALUE value;
    PdhCollectQueryData(query);
    PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL, &value);

    return static_cast<float>(value.doubleValue);
}

// Maximum rated CPU speed (GHz)
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

// Current CPU speed (GHz)
string CPUInfo::get_cpu_speed()
{
    string value = wmi_querysingle_value(
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

// Physical CPU sockets (usually 1)
int CPUInfo::get_cpu_sockets()
{
    string value = wmi_querysingle_value(
        L"SELECT COUNT(*) FROM Win32_Processor",
        L"COUNT(*)"
    );

    try { return stoi(value); }
    catch (...) { return 1; }
}

// Physical CPU cores
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

// Logical processors (threads)
int CPUInfo::get_cpu_logical_processors()
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwNumberOfProcessors;
}

// Virtualization status (BIOS / firmware level)
string CPUInfo::get_cpu_virtualization()
{
    return IsProcessorFeaturePresent(PF_VIRT_FIRMWARE_ENABLED)
        ? "Enabled"
        : "Disabled";
}

// L1 cache size
string CPUInfo::get_cpu_l1_cache()
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
        if (info.Relationship == RelationCache && info.Cache.Level == 1)
            size += info.Cache.Size;

    if (!size) return "N/A";

    ostringstream ss;
    ss << (size / 1024) << " KB";
    return ss.str();
}

// L2 cache size
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

// L3 cache size
string CPUInfo::get_cpu_l3_cache()
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
        if (info.Relationship == RelationCache && info.Cache.Level == 3)
            size += info.Cache.Size;

    if (!size) return "N/A";

    ostringstream ss;
    ss << (size >= 1024 * 1024 ? size / (1024 * 1024) : size / 1024)
        << (size >= 1024 * 1024 ? " MB" : " KB");
    return ss.str();
}

// System uptime (days:hh:mm:ss)
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

// Number of running processes
int CPUInfo::get_process_count()
{
    string value = wmi_querysingle_value(
        L"SELECT COUNT(*) FROM Win32_Process",
        L"COUNT(*)"
    );

    try { return stoi(value); }
    catch (...) { return 0; }
}

// Total thread count
int CPUInfo::get_thread_count()
{
    string value = wmi_querysingle_value(
        L"SELECT ThreadCount FROM Win32_PerfFormattedData_PerfProc_Process WHERE Name='_Total'",
        L"ThreadCount"
    );

    try { return stoi(value); }
    catch (...) { return 0; }
}

// Total handle count
int CPUInfo::get_handle_count()
{
    string value = wmi_querysingle_value(
        L"SELECT HandleCount FROM Win32_PerfFormattedData_PerfProc_Process WHERE Name='_Total'",
        L"HandleCount"
    );

    try { return stoi(value); }
    catch (...) { return 0; }
}
