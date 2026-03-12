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

#include "include\CPUInfo.h"

#include <windows.h>   // Core Windows API — sometimes pain, sometimes power
#include <intrin.h>    // CPUID and low-level CPU instructions
#include <vector>      // Dynamic storage (because life isn't fixed-size)
#include <sstream>     // Turning numbers into pretty strings
#include <wbemidl.h>   // WMI — Windows answering deep existential questions
#include <pdh.h>       // Performance counters (Task Manager vibes)
#include <comdef.h>    // COM helpers so we don't lose our sanity
#include <iomanip>     // Formatting polish (decimals, padding, alignment)

#pragma comment(lib, "pdh.lib")      
// Auto-link PDH so CPU usage works without linker drama

#pragma comment(lib, "wbemuuid.lib") 
// Required for WMI / COM UUIDs — Windows won't talk without this

using namespace std; // If this confuses you… we need to talk 😄

/*
documentation (1) : WMI helper function for single-value queries

    WMI Helper Function — the reliable messenger between you and Windows internals

    This function handles all the COM/WMI boilerplate so we don't have to repeat
    the same 50 lines of code for every single WMI query.

    How it works:
    -------------
    1. Initialize COM (Windows' component system)
    2. Set up security so we can read system data
    3. Connect to the WMI service (specifically ROOT\CIMV2, where hardware info lives)
    4. Execute a WQL query (WMI's version of SQL)
    5. Extract ONE property value from the first result
    6. Clean up everything properly (no memory leaks!)

    Why so much ceremony?
    ---------------------
    WMI is built on COM (Component Object Model), which is Microsoft's 90s-era
    technology for software components talking to each other. COM requires:
    - Explicit initialization
    - Security configuration
    - Manual reference counting
    - Careful cleanup

    This function encapsulates all that complexity so our other functions can
    just ask for data and get a simple string back.

    The alternative would be repeating this 50-line dance 15 times...
    and nobody wants that :)
*/

// Section (1) : WMI helper function for single-value queries
string wmi_querysingle_value(const wchar_t* query, const wchar_t* property_name)
{
    HRESULT hres;
    IWbemLocator* locator = NULL;
    IWbemServices* services = NULL;
    IEnumWbemClassObject* enumerator = NULL;
    IWbemClassObject* clsObj = NULL;
    ULONG uReturn = 0;

    string result = "Unknown";

    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
    {
        hres = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
        if (FAILED(hres))
            return result;
    }

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

    if (services) services->Release();
    if (locator) locator->Release();
    CoUninitialize();

    return result;
}

/*
documentation (2) : CPU brand string extraction using CPUID

    🧠 How this function gets your CPU name (don't panic, it's cooler than it looks)

    CPUs don't store their brand name ("Ryzen 5 5600G", "Intel i7-12700K", etc.)
    as a normal string you can just ask for. Instead, they expose it through
    a low-level instruction called CPUID.

    Think of CPUID as:
    "Hey CPU, tell me about yourself."

    The CPU brand string is split into 3 chunks of 16 bytes each.
    To get the full name, we have to ask for all 3 parts and glue them together.

    Why these magic numbers?
    - 0x80000002 → first 16 characters
    - 0x80000003 → next 16 characters
    - 0x80000004 → last 16 characters

    Each __cpuid call fills an array of 4 integers (4 × 4 bytes = 16 bytes).
    We copy each chunk into the correct position in cpu_brand.

    After all three calls:
    cpu_brand contains a proper null-terminated C-string.
    We then convert it into a std::string and return it.

    This is exactly how tools like Task Manager, CPU-Z, and neofetch
    get the CPU model name on Windows.

    Yes, it looks low-level.
    Yes, it's a little magical.
    And yes — this is the correct and intended way 🙂
*/

// Section (2) : CPU brand string extraction using CPUID
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

/*
documentation (3) : CPU usage percentage (Task Manager style)

    CPU usage (Task Manager style) — what's really happening here :)

    Windows does not give you "CPU usage %" as a simple function call.
    Instead, it exposes performance counters through PDH
    (Performance Data Helper), which is the same system Task Manager uses.

    Why all the static variables?
    - PDH queries and counters are expensive to create.
    - Creating them every frame would be slow and unnecessary.
    - So we initialize them once, then reuse them forever =))

    How this works step-by-step:
    1. Open a PDH query (this is the container for performance counters).
    2. Add the "\\Processor(_Total)\\% Processor Time" counter.
       This represents overall CPU usage across all cores.
    3. Collect initial data (PDH needs a baseline to compare against).
    4. Wait a short moment so PDH has time to calculate a real delta.

    On every call after initialization:
    - We ask PDH to collect fresh data.
    - We read the formatted value as a double (percentage).
    - We return it as a float, just like Task Manager does.

    If this looks over-engineered:
    congrats, you just discovered Windows performance APIs :)
*/

// Section (3) : CPU usage percentage (Task Manager style)
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

/*
documentation (4) : Maximum rated CPU speed (base clock)

    Maximum rated CPU speed (base clock) — explained without panic :)

    This value represents the CPU's *official* maximum base frequency,
    not the current speed and not boost clocks.

    Where does it come from?
    - Windows exposes this through WMI (Windows Management Instrumentation).
    - The Win32_Processor class provides MaxClockSpeed in **MHz**.
    - This is the same number Task Manager shows as "Base speed".

    Why WMI?
    - CPUID does NOT reliably expose base clock.
    - PDH only reports usage, not hardware ratings.
    - WMI is the cleanest and most accurate source here.

    Flow:
    1. Query Win32_Processor → MaxClockSpeed
    2. Value comes back as a string (usually in MHz)
    3. Convert MHz → GHz (divide by 1000)
    4. Format nicely with two decimals

    If anything goes wrong:
    - Missing data
    - Invalid value
    - Conversion failure
    We return "N/A" instead of crashing or lying :)

    Stable, boring, and correct — exactly what system info code should be.
*/

// Section (4) : Maximum rated CPU speed (base clock)
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

/*
documentation (5) : Current CPU speed (real-time boost clock)

    Current CPU speed — what the CPU is actually running at RIGHT NOW

    Unlike base speed (which is static), current speed changes dynamically:
    - Intel Turbo Boost
    - AMD Precision Boost
    - Power saving downclocking
    - Thermal throttling

    This function reads the REAL-TIME clock speed, which can be:
    - Higher than base (when boosting)
    - Lower than base (when idling)
    - Exactly at base (when at nominal load)

    Technical details:
    ------------------
    Source: WMI Win32_Processor.CurrentClockSpeed (in MHz)
    This is the same value Task Manager shows in the Performance tab.

    The OS reads this from CPU performance counters, not from a static table.
    It's what the CPU is actually doing, not what it's rated for.

    Why not use CPUID?
    CPUID only tells you what the CPU CAN do (max frequency).
    It doesn't tell you what it's DOING right now.

    This is live data — refresh it often to see CPU frequency changes!
*/

// Section (5) : Current CPU speed (real-time boost clock)
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

/*
documentation (6) : Physical CPU socket count

    CPU sockets — how many physical CPUs are installed

    Most consumer systems have: 1 socket
    Servers/workstations can have: 2, 4, or even 8 sockets

    Why does this matter?
    ---------------------
    - Each socket is a separate physical CPU package
    - Sockets are NOT cores — they're the actual chips on the motherboard
    - Multi-socket systems have NUMA (Non-Uniform Memory Access) considerations
    - Task Manager shows sockets as separate "CPU" graphs

    How we count them:
    ------------------
    Simple WMI query: SELECT COUNT(*) FROM Win32_Processor

    Win32_Processor enumerates each physical CPU package.
    If you have a dual-socket Xeon system, this returns 2.
    If you have a normal desktop, this returns 1.

    Edge case handling:
    - Invalid result → default to 1 (safe assumption for consumer hardware)
    - Conversion failure → default to 1
    - WMI failure → default to 1

    Because honestly, if you're running multi-socket, you probably know it :)
*/

// Section (6) : Physical CPU socket count
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

/*
documentation (7) : Physical CPU core count

    Physical cores — the actual silicon, not the virtual threads

    IMPORTANT: This counts PHYSICAL cores, not logical processors.
    Example: Intel i7-12700K = 12 cores, 20 threads
    This function returns: 12 (not 20!)

    Why not just use GetSystemInfo()?
    --------------------------------
    GetSystemInfo() returns LOGICAL processors (threads).
    We need GetLogicalProcessorInformation() to distinguish:
    - Physical cores (RelationProcessorCore)
    - NUMA nodes (RelationNumaNode)
    - Cache (RelationCache)
    - Processor packages (RelationProcessorPackage)

    How it works:
    -------------
    1. Call GetLogicalProcessorInformation() twice:
       - First call: get required buffer size
       - Second call: fill buffer with actual data
    2. Walk through the array
    3. Count only RelationProcessorCore entries
    4. Return the total

    This is the same method Task Manager uses for "Cores" display.
    It's accurate even with:
    - Hybrid architectures (P-cores + E-cores)
    - Disabled cores
    - Asymmetric core configurations

    No hyperthreading/SMT confusion here — pure physical silicon count!
*/

// Section (7) : Physical CPU core count
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

/*
documentation (8) : Logical processor count (threads)

    Logical processors — all the threads your CPU can run simultaneously

    This is: Physical cores × SMT factor
    Where SMT factor is usually 2 (Hyper-Threading, SMT)

    Examples:
    ---------
    - 6 cores, no HT = 6 logical processors
    - 6 cores, with HT = 12 logical processors
    - 8P + 8E cores, with HT = 24 logical processors

    Why use GetSystemInfo()?
    ------------------------
    Simple, reliable, and always accurate.
    dwNumberOfProcessors in SYSTEM_INFO = total threads Windows sees.

    This matches:
    - Task Manager "Logical processors"
    - Device Manager processor count
    - msconfig boot advanced options
    - Windows thread scheduler

    Important distinction:
    ---------------------
    This is NOT the same as "CPU cores"!
    Cores = physical execution units
    Logical processors = virtual threads (can be more than cores)

    For most performance considerations, logical processors matter more
    because that's how many threads Windows can schedule simultaneously.
*/

// Section (8) : Logical processor count (threads)
int CPUInfo::get_cpu_logical_processors()
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwNumberOfProcessors;
}

/*
documentation (9) : CPU virtualization status (BIOS/firmware)

    Virtualization — is your CPU ready for VMs and hypervisors?

    This checks if hardware virtualization is ENABLED in:
    - BIOS/UEFI settings (Intel VT-x, AMD-V)
    - Firmware configuration
    - Windows hypervisor platform

    What virtualization enables:
    ---------------------------
    - Windows Subsystem for Linux (WSL2)
    - Hyper-V virtual machines
    - Docker Desktop
    - Android emulators
    - VirtualBox/VMware (with hardware acceleration)

    How it works:
    -------------
    IsProcessorFeaturePresent(PF_VIRT_FIRMWARE_ENABLED)

    This API checks multiple things:
    1. CPU hardware support (does the chip have VT-x/AMD-V?)
    2. BIOS/UEFI enablement (is it turned on in firmware?)
    3. No hypervisor conflict (is another hypervisor already running?)

    Returns "Enabled" if ALL conditions are met.
    Returns "Disabled" if ANY condition fails.

    Note: This doesn't check Windows feature enablement
    (like Hyper-V Windows feature). Just the hardware/firmware readiness.
*/

// Section (9) : CPU virtualization status (BIOS/firmware)
string CPUInfo::get_cpu_virtualization()
{
    return IsProcessorFeaturePresent(PF_VIRT_FIRMWARE_ENABLED)
        ? "Enabled"
        : "Disabled";
}

/*
documentation (10) : L1 cache size per core

    L1 cache — the CPU's lightning-fast memory (closest to the cores)

    L1 is split into:
    - L1 Instruction cache (for code)
    - L1 Data cache (for data)
    This function returns the TOTAL L1 per core.

    Characteristics:
    ----------------
    - Smallest cache (typically 32-64 KB per core)
    - Fastest cache (1-2 cycle latency)
    - Per-core dedicated (not shared)
    - Critical for single-thread performance

    How we calculate it:
    -------------------
    1. GetLogicalProcessorInformation() returns cache topology
    2. Filter for Level == 1 (L1 cache)
    3. Sum Size across all L1 cache entries
    4. Convert bytes to KB

    Why sum across entries?
    Because GetLogicalProcessorInformation reports cache PER LOGICAL PROCESSOR.
    But L1 is usually per PHYSICAL core.
    We sum to get total L1 across all cores.

    Output format: "XXX KB"
    Example: "256 KB" (for 4 cores × 64 KB L1 each)
*/

// Section (10) : L1 cache size per core
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

/*
documentation (11) : L2 cache size

    L2 cache — the middle child between L1 speed and L3 capacity

    L2 characteristics:
    ------------------
    - Larger than L1 (256 KB - 1 MB per core)
    - Slower than L1 (10-20 cycle latency)
    - Often shared between cores in a cluster
    - Balances speed and capacity

    Architecture variations:
    -----------------------
    - Intel: Usually per-core (1 MB per core)
    - AMD Zen: Shared between core pairs (512 KB per pair)
    - Apple M1: Shared between performance cores
    - Hybrid CPUs: Different L2 sizes for P-cores vs E-cores

    How we handle it:
    -----------------
    1. Get all cache information from Windows
    2. Filter for Level == 2 (L2 cache)
    3. Sum all L2 cache sizes
    4. Smart formatting:
       - < 1 MB → show as KB
       - ≥ 1 MB → show as MB

    Output examples:
    ---------------
    - "512 KB" (older CPUs)
    - "2 MB" (modern 4-core CPU)
    - "10 MB" (high-end desktop)
    - "N/A" (if detection fails)

    This matches what CPU-Z and HWiNFO show for "L2 Cache".
*/

// Section (11) : L2 cache size
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

/*
documentation (12) : L3 cache size (shared, last-level cache)

    L3 cache — the CPU's shared memory pool (big and strategic)

    L3 is special because:
    ---------------------
    - Shared among ALL cores
    - Largest cache (8-64 MB typical, up to 768 MB on server CPUs)
    - Slowest cache (30-50 cycle latency)
    - Critical for multi-threaded performance
    - Acts as a coherence buffer between cores

    Why L3 matters:
    ---------------
    - Reduces RAM access (which is 100x slower than L3)
    - Enables efficient core-to-core communication
    - Determines gaming performance at high resolutions
    - Affects professional application throughput

    Detection method:
    ----------------
    Same as L1/L2 but filtering for Level == 3.

    Important: We SUM across all entries because:
    - Some CPUs report L3 per NUMA node
    - Some report L3 per CCX (AMD)
    - Some report total L3 once
    Summing ensures we get the correct total.

    Smart formatting:
    ----------------
    - < 1 MB → show as KB (rare for L3, but handles edge cases)
    - ≥ 1 MB → show as MB (normal for all modern CPUs)

    Example outputs: "16 MB", "32 MB", "64 MB", "N/A"
*/

// Section (12) : L3 cache size (shared, last-level cache)
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

/*
documentation (13) : System uptime calculation

    System uptime — how long since the last reboot

    This isn't just "how long has Windows been running" — it's literally
    how long since the hardware last powered on. Useful for:
    - Server stability monitoring
    - Diagnosing random reboots
    - Bragging rights on homelab forums :)

    Calculation method:
    ------------------
    GetTickCount64() returns milliseconds since boot.
    We convert:
    - Milliseconds → seconds
    - Seconds → minutes
    - Minutes → hours
    - Hours → days

    Format: "days:hh:mm:ss"
    Examples:
    - "0:01:30:45" = 1 hour, 30 minutes, 45 seconds
    - "14:23:15:30" = 14 days, 23 hours, 15 minutes, 30 seconds

    Why not use WMI for this?
    ------------------------
    WMI has Win32_OperatingSystem.LastBootUpTime, but:
    - It's slower to query
    - Requires timezone conversion
    - GetTickCount64() is simpler and faster

    Edge cases handled:
    ------------------
    - System tick counter rollover (after 49.7 days) — GetTickCount64 handles it
    - Sleep/hibernate time — NOT included (uptime pauses during sleep)
    - Formatting always 2 digits for hours/minutes/seconds (00-99)

    This matches Task Manager's "Up time" in the Performance tab.
*/

// Section (13) : System uptime calculation
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

/*
documentation (14) : Running process count

    Process count — everything running on your system right now

    This includes:
    - User applications (Chrome, Word, games)
    - System services (svchost.exe instances)
    - Background processes (antivirus, updaters)
    - Windows components (explorer.exe, winlogon.exe)

    Why monitor process count?
    --------------------------
    - High counts can indicate malware (process injection)
    - Resource usage correlation (processes vs CPU/RAM)
    - Service monitoring (expected processes running?)
    - Cleanup after software uninstalls (zombie processes)

    How it works:
    -------------
    Simple WMI query: SELECT COUNT(*) FROM Win32_Process

    Win32_Process includes EVERY process, regardless of:
    - Session (console, service, user)
    - Architecture (32-bit, 64-bit)
    - State (running, suspended, terminated)
    - Privilege (system, user, protected)

    Typical values:
    --------------
    - Fresh boot: ~120-150 processes
    - Normal use: ~150-250 processes
    - Heavy use: 250-400+ processes
    - Problematic: 500+ processes (investigate!)

    Note: Each browser tab counts as a process in modern browsers!
*/

// Section (14) : Running process count
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

/*
documentation (15) : Total system thread count

    Thread count — all execution contexts in the system

    Threads vs Processes:
    --------------------
    - Process = container (memory, handles, security context)
    - Thread = execution unit (runs code, has stack, scheduled by OS)

    One process can have multiple threads.
    Example: Chrome might have 100+ threads across its processes.

    Why thread count matters:
    ------------------------
    - Thread switching overhead (context switches)
    - Scheduler pressure (more threads = more scheduling decisions)
    - Concurrency potential (how much can actually run in parallel)
    - System responsiveness indicator

    Source:
    -------
    WMI performance counter: Win32_PerfFormattedData_PerfProc_Process
    We query for "_Total" which sums threads across ALL processes.

    This is the same counter Task Manager shows in the Performance tab
    as "Threads" (bottom of the CPU graph).

    Typical values:
    --------------
    - Fresh boot: ~2000-3000 threads
    - Normal use: ~4000-8000 threads
    - Heavy use: 10000-20000+ threads
    - Servers: Can exceed 50000 threads

    High thread counts aren't necessarily bad — modern OSes handle them well.
*/

// Section (15) : Total system thread count
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

/*
documentation (16) : Total system handle count

    Handle count — all open resources in the system

    Handles are references to system objects:
    - Files (CreateFile)
    - Registry keys (RegOpenKey)
    - Processes/Threads (OpenProcess, OpenThread)
    - Windows/GUI objects
    - Mutexes, Events, Semaphores
    - Memory mappings
    - Sockets, Pipes

    Why monitor handles?
    --------------------
    - Resource leaks (handle leaks cause gradual system degradation)
    - Process isolation (how many resources each process holds)
    - System limits (Windows has per-process and system-wide handle limits)
    - Performance (too many handles = slower handle table lookups)

    Source:
    -------
    Same WMI class as thread count, different counter.
    Win32_PerfFormattedData_PerfProc_Process.HandleCount for "_Total"

    This is what Task Manager shows as "Handles" in Performance tab.

    Typical values:
    --------------
    - Fresh boot: ~50,000-80,000 handles
    - Normal use: ~100,000-200,000 handles
    - Heavy use: 300,000-500,000+ handles
    - System limit: 16,777,216 handles (24-bit limit per session)

    Handle leaks are sneaky — they don't show as memory leaks
    but can cause "out of handles" errors and system instability.
*/

// Section (16) : Total system handle count
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