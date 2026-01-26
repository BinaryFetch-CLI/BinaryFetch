#include "CompactOS.h"
#include <sstream>
#include <iomanip>
#include <Windows.h>
#include <winternl.h>

// Typedef for RtlGetVersion
typedef LONG(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOEXW);

//---------------- Get OS Name ------------------
std::string CompactOS::getOSName()
{
    RTL_OSVERSIONINFOEXW rovi = { 0 };
    rovi.dwOSVersionInfoSize = sizeof(rovi);

    HMODULE hMod = GetModuleHandleW(L"ntdll.dll");
    if (hMod)
    {
        RtlGetVersionPtr fx = (RtlGetVersionPtr)GetProcAddress(hMod, "RtlGetVersion");
        if (fx) fx(&rovi);
    }

    // Only return OS name, without version/build
    if (rovi.dwMajorVersion == 10 && rovi.dwBuildNumber >= 22000)
        return "Windows 11 ";
    if (rovi.dwMajorVersion == 10)
        return "Windows 10 ";
    if (rovi.dwMajorVersion == 6 && rovi.dwMinorVersion == 3)
        return "Windows 8.1 ";
    if (rovi.dwMajorVersion == 6 && rovi.dwMinorVersion == 2)
        return "Windows 8 ";
    if (rovi.dwMajorVersion == 6 && rovi.dwMinorVersion == 1)
        return "Windows 7 ";
    if (rovi.dwMajorVersion == 6 && rovi.dwMinorVersion == 0)
        return "Windows Vista ";
    if (rovi.dwMajorVersion == 5 && rovi.dwMinorVersion == 2)
        return "Windows Server 2003/XP x64 ";
    if (rovi.dwMajorVersion == 5 && rovi.dwMinorVersion == 1)
        return "Windows XP ";

    return "Unknown Windows Version";
}

//---------------- Get OS Build ------------------
std::string CompactOS::getOSBuild()
{
    RTL_OSVERSIONINFOEXW rovi = { 0 };
    rovi.dwOSVersionInfoSize = sizeof(rovi);

    HMODULE hMod = GetModuleHandleW(L"ntdll.dll");
    if (hMod)
    {
        RtlGetVersionPtr fx = (RtlGetVersionPtr)GetProcAddress(hMod, "RtlGetVersion");
        if (fx) fx(&rovi);
    }

    std::ostringstream oss;
    oss << rovi.dwMajorVersion << "." << rovi.dwMinorVersion
        << " (Build " << rovi.dwBuildNumber << ")";
    return oss.str();
}

//---------------- Get OS Uptime -----------------
std::string CompactOS::getUptime()
{
    ULONGLONG ms = GetTickCount64();
    ULONGLONG seconds = ms / 1000;
    int days = (int)(seconds / 86400);
    int hours = (int)((seconds % 86400) / 3600);
    int minutes = (int)((seconds % 3600) / 60);

    std::ostringstream oss;
    if (days > 0) oss << days << "d ";
    if (hours > 0) oss << hours << "h ";
    oss << minutes << "m";
    return oss.str();
}

//---------------- Get System Architecture -------
std::string CompactOS::getArchitecture()
{
    SYSTEM_INFO si = { 0 };
    GetNativeSystemInfo(&si);

    switch (si.wProcessorArchitecture)
    {
    case PROCESSOR_ARCHITECTURE_AMD64: return "64-bit";
    case PROCESSOR_ARCHITECTURE_INTEL: return "32-bit";
    case PROCESSOR_ARCHITECTURE_ARM64: return "ARM64";
    default: return "Unknown";
    }
}
