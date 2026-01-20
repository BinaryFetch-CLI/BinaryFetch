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

#include <windows.h>  // Windows API (sometimes shit, some times magic) 
#include <intrin.h>   // Low-level CPU instructions (CPUID stuff goes brrr)
#include <vector>     // Dynamic arrays because fixed-size arrays are too restrictive
#include <sstream>    // String building playground (turn numbers into pretty text)
#include <wbemidl.h>  // WMI access – asking Windows deep questions about the system
#include <pdh.h>      // Performance Data Helper – CPU usage, Task Manager style
#include <comdef.h>   // COM helpers so Windows APIs don’t make us cry
#include <iomanip>    // Fancy formatting (decimals, padding, alignment, drip)



#pragma comment(lib, "pdh.lib")      
// Auto-link PDH (Performance Data Helper) so CPU usage works without linker drama

#pragma comment(lib, "wbemuuid.lib") 
// Auto-link WMI UUIDs because COM + WMI won’t cooperate without this guy


using namespace std; // if you don't whatt  is std...C'mon, get a life

//helper fucntion for WMI queries-----------------------------------------------------------------
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
		if (FAILED(hres)) return result;
	}

	hres = CoInitializeSecurity(NULL, -1, NULL, NULL,
		RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
	if (FAILED(hres) && hres != RPC_E_TOO_LATE)
	{
		CoUninitialize();
		return result;
	}

	hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&locator);
	if (FAILED(hres))
	{
		CoUninitialize();
		return result;
	}

	hres = locator->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &services);
	if (FAILED(hres))
	{
		locator->Release();
		CoUninitialize();
		return result;
	}

	hres = CoSetProxyBlanket(services, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
		RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
	if (FAILED(hres))
	{
		services->Release();
		locator->Release();
		CoUninitialize();
		return result;
	}

	hres = services->ExecQuery(_bstr_t(L"WQL"), _bstr_t(query),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &enumerator);
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
				if (vtProp.vt == VT_BSTR && vtProp.bstrVal != NULL)
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

// get the cpu model and brand (like task manager shows)
string CPUInfo::get_cpu_info()
{
	int cpu_data[4] = { -1 };
	char cpu_brand[0x40];
	memset(cpu_brand, 0, sizeof(cpu_brand));

	//call cpu _cpuid with 0x80000002, 0x80000003, 0x80000004 to get brand string
	__cpuid(cpu_data, 0x80000002);
	memcpy(cpu_brand, cpu_data, sizeof(cpu_data));

	__cpuid(cpu_data, 0x80000003);
	memcpy(cpu_brand + 16, cpu_data, sizeof(cpu_data));

	__cpuid(cpu_data, 0x80000004);
	memcpy(cpu_brand + 32, cpu_data, sizeof(cpu_data));

	return string(cpu_brand);
}

// get utilization percentage (like task manager)
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

	return (float)value.doubleValue;
}

// get base speed in GHz (like task manager shows)
string CPUInfo::get_cpu_base_speed()
{
	string base_clock_speed = wmi_querysingle_value(L"SELECT MaxClockSpeed FROM Win32_Processor", L"MaxClockSpeed");
	if (base_clock_speed == "Unknown" || base_clock_speed.empty()) return "N/A";
	try
	{
		float speed = stof(base_clock_speed) / 1000.0f;
		ostringstream ss;
		ss << fixed << setprecision(2) << speed << " GHz";
		return ss.str();
	}
	catch (...)
	{
		return "N/A";
	}
}

// get current speed in GHz (like task manager shows)
string CPUInfo::get_cpu_speed()
{
	string current_clock_speed = wmi_querysingle_value(L"SELECT CurrentClockSpeed FROM Win32_Processor", L"CurrentClockSpeed");
	if (current_clock_speed == "Unknown" || current_clock_speed.empty()) return "N/A";
	try
	{
		float speed = stof(current_clock_speed) / 1000.0f;
		ostringstream ss;
		ss << fixed << setprecision(2) << speed << " GHz";
		return ss.str();
	}
	catch (...)
	{
		return "N/A";
	}
}

// get sockets (usually 1 for consumer PCs)
int CPUInfo::get_cpu_sockets()
{
	string sockets = wmi_querysingle_value(L"SELECT COUNT(*) FROM Win32_Processor", L"COUNT(*)");
	if (sockets == "Unknown" || sockets.empty()) return 1;
	try
	{
		return stoi(sockets);
	}
	catch (...)
	{
		return 1;
	}
}

// get number of cores (physical)
int CPUInfo::get_cpu_cores()
{
	DWORD buffer_length = 0;
	GetLogicalProcessorInformation(NULL, &buffer_length);
	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
	{
		return -1;
	}

	vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer(buffer_length / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
	if (!GetLogicalProcessorInformation(buffer.data(), &buffer_length))
	{
		return -1;
	}

	int physical_cores = 0;
	for (auto& info : buffer)
	{
		if (info.Relationship == RelationProcessorCore)
		{
			physical_cores++;
		}
	}
	return physical_cores;
}

// get number of logical processors (threads)
int CPUInfo::get_cpu_logical_processors()
{
	SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);
	return sys_info.dwNumberOfProcessors;
}

// get virtualization status
string CPUInfo::get_cpu_virtualization()
{
	BOOL isEnabled = IsProcessorFeaturePresent(PF_VIRT_FIRMWARE_ENABLED);
	return std::string(isEnabled ? "Enabled" : "Disabled");
}

// get L1 cache
string CPUInfo::get_cpu_l1_cache()
{
	DWORD buffer_length = 0;
	GetLogicalProcessorInformation(NULL, &buffer_length);
	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
	{
		return "N/A";
	}

	vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer(buffer_length / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
	if (!GetLogicalProcessorInformation(buffer.data(), &buffer_length))
	{
		return "N/A";
	}

	DWORD l1_cache = 0;
	for (auto& info : buffer)
	{
		if (info.Relationship == RelationCache && info.Cache.Level == 1)
		{
			l1_cache += info.Cache.Size;
		}
	}

	if (l1_cache == 0) return "N/A";

	ostringstream ss;
	ss << (l1_cache / 1024) << " KB";
	return ss.str();
}

// get L2 cache
string CPUInfo::get_cpu_l2_cache()
{
	DWORD buffer_length = 0;
	GetLogicalProcessorInformation(NULL, &buffer_length);
	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
	{
		return "N/A";
	}

	vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer(buffer_length / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
	if (!GetLogicalProcessorInformation(buffer.data(), &buffer_length))
	{
		return "N/A";
	}

	DWORD l2_cache = 0;
	for (auto& info : buffer)
	{
		if (info.Relationship == RelationCache && info.Cache.Level == 2)
		{
			l2_cache += info.Cache.Size;
		}
	}

	if (l2_cache == 0) return "N/A";

	ostringstream ss;
	if (l2_cache >= 1024 * 1024)
		ss << (l2_cache / (1024 * 1024)) << " MB";
	else
		ss << (l2_cache / 1024) << " KB";
	return ss.str();
}

// get L3 cache
string CPUInfo::get_cpu_l3_cache()
{
	DWORD buffer_length = 0;
	GetLogicalProcessorInformation(NULL, &buffer_length);
	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
	{
		return "N/A";
	}

	vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer(buffer_length / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
	if (!GetLogicalProcessorInformation(buffer.data(), &buffer_length))
	{
		return "N/A";
	}

	DWORD l3_cache = 0;
	for (auto& info : buffer)
	{
		if (info.Relationship == RelationCache && info.Cache.Level == 3)
		{
			l3_cache += info.Cache.Size;
		}
	}

	if (l3_cache == 0) return "N/A";

	ostringstream ss;
	if (l3_cache >= 1024 * 1024)
		ss << (l3_cache / (1024 * 1024)) << " MB";
	else
		ss << (l3_cache / 1024) << " KB";
	return ss.str();
}

// get uptime (system uptime)
string CPUInfo::get_system_uptime()
{
	ULONGLONG uptime_ms = GetTickCount64();
	ULONGLONG seconds = uptime_ms / 1000;
	ULONGLONG minutes = seconds / 60;
	ULONGLONG hours = minutes / 60;
	ULONGLONG days = hours / 24;

	ostringstream ss;
	ss << days << ":" << setfill('0') << setw(2) << (hours % 24) << ":"
		<< setw(2) << (minutes % 60) << ":" << setw(2) << (seconds % 60);
	return ss.str();
}

// get number of processes
int CPUInfo::get_process_count()
{
	string processes = wmi_querysingle_value(L"SELECT COUNT(*) FROM Win32_Process", L"COUNT(*)");
	if (processes == "Unknown" || processes.empty()) return 0;
	try
	{
		return stoi(processes);
	}
	catch (...)
	{
		return 0;
	}
}

// get number of threads
int CPUInfo::get_thread_count()
{
	string threads = wmi_querysingle_value(L"SELECT ThreadCount FROM Win32_PerfFormattedData_PerfProc_Process WHERE Name='_Total'", L"ThreadCount");
	if (threads == "Unknown" || threads.empty()) return 0;
	try
	{
		return stoi(threads);
	}
	catch (...)
	{
		return 0;
	}
}

// get number of handles
int CPUInfo::get_handle_count()
{
	string handles = wmi_querysingle_value(L"SELECT HandleCount FROM Win32_PerfFormattedData_PerfProc_Process WHERE Name='_Total'", L"HandleCount");
	if (handles == "Unknown" || handles.empty()) return 0;
	try
	{
		return stoi(handles);
	}
	catch (...)
	{
		return 0;
	}
}