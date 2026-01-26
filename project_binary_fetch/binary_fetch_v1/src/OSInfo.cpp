#include "OSInfo.h"
#include <Windows.h>
#include <VersionHelpers.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <winreg.h>
#include <tchar.h>
#pragma comment(lib, "wbemuuid.lib")

// Get Windows version using RtlGetVersion----------------------------------------------------------------------------------
typedef LONG(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

std::string OSInfo::GetOSVersion() {
    HMODULE hMod = GetModuleHandleW(L"ntdll.dll");
    if (hMod) {
        RtlGetVersionPtr fn = (RtlGetVersionPtr)GetProcAddress(hMod, "RtlGetVersion");
        if (fn) {
            RTL_OSVERSIONINFOW rovi = { 0 };
            rovi.dwOSVersionInfoSize = sizeof(rovi);
            if (fn(&rovi) == 0) {
                return "Windows " + std::to_string(rovi.dwMajorVersion) + "." +
                    std::to_string(rovi.dwMinorVersion) + " Build " +
                    std::to_string(rovi.dwBuildNumber);
            }
        }
    }
    return "Unknown Windows version";
}

// Get 32-bit or 64-bit architecture------------------------------------------------------------------------------------
std::string OSInfo::GetOSArchitecture() {
    BOOL is64bitOS = FALSE;
#ifdef _WIN64
    is64bitOS = TRUE; // 64-bit program on 64-bit Windows
#else
    // 32-bit program, check if OS is 64-bit
    BOOL bWow64 = FALSE;
    if (IsWow64Process(GetCurrentProcess(), &bWow64)) {
        is64bitOS = bWow64;
    }
#endif
    return is64bitOS ? "64-bit" : "32-bit";
}

// Get Windows edition (Home, Pro, Enterprise) via WMI--------------------------------------------------------------------------
std::string OSInfo::GetOSName() {
    HRESULT hres;

    // Initialize COM
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) return "Unknown Edition";

    // Initialize WMI
    hres = CoInitializeSecurity(NULL, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE, NULL);
    if (FAILED(hres)) return "Unknown Edition";

    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc);
    if (FAILED(hres)) return "Unknown Edition";

    IWbemServices* pSvc = NULL;
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0,
        NULL, 0, 0, &pSvc);
    if (FAILED(hres)) {
        pLoc->Release();
        return "Unknown Edition";
    }

    hres = CoSetProxyBlanket(
        pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE);
    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        return "Unknown Edition";
    }

    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT Caption FROM Win32_OperatingSystem"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);
    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        return "Unknown Edition";
    }

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;
    std::string osName = "Unknown Edition";

    if (pEnumerator) {
        if (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK) {
            VARIANT vtProp;
            if (SUCCEEDED(pclsObj->Get(L"Caption", 0, &vtProp, 0, 0))) {
                _bstr_t bstrOS(vtProp.bstrVal);
                osName = std::string((char*)bstrOS);
            }
            VariantClear(&vtProp);
            pclsObj->Release();
        }
        pEnumerator->Release();
    }

    pSvc->Release();
    pLoc->Release();
    CoUninitialize();

    return osName;
}
//function to get os serial number-----------------------------------------------------------------------------------------
std::string OSInfo::get_os_serial_number()
{
    std::string serial_number = "Unknown"; //initially it's unknown

    if (FAILED(CoInitializeEx(0, COINIT_MULTITHREADED)))
        return serial_number;
    CoInitializeSecurity(NULL, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE, NULL);

    IWbemLocator* locator = nullptr;
    CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&locator);

    IWbemServices* service = nullptr;
    locator->ConnectServer (_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &service);
    CoSetProxyBlanket(service, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

    IEnumWbemClassObject* enumerator = nullptr;
    service->ExecQuery(bstr_t("WQL"),
        bstr_t("SELECT SerialNumber FROM Win32_OperatingSystem"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL, &enumerator);

    IWbemClassObject* obj = nullptr;
    ULONG ret = 0;
    if (enumerator && enumerator->Next(WBEM_INFINITE, 1, &obj, &ret) == S_OK)
    {
        VARIANT vtProp;
        obj->Get(L"SerialNumber", 0, &vtProp, 0, 0);
        _bstr_t bstr(vtProp.bstrVal);
        VariantClear(&vtProp);
        obj->Release();
    }

    if (enumerator) enumerator->Release();
    service->Release();
    locator->Release();

    return serial_number;
}
// function to show os uptime----------------------------------------------------------------------------------------------
std::string OSInfo::get_os_uptime()
{
    // Get the number of milliseconds since the system started
    ULONGLONG ms = GetTickCount64();

    // Convert milliseconds to total seconds
    ULONGLONG total_seconds = ms / 1000;

    // Calculate days, hours, and minutes from total seconds
    ULONGLONG days = total_seconds / (24 * 3600);                // total days
    ULONGLONG hours = (total_seconds % (24 * 3600)) / 3600;      // remaining hours after days
    ULONGLONG minutes = (total_seconds % 3600) / 60;             // remaining minutes after hours

    // String to store the final uptime
    std::string result = "";

    // Add days to the string if there are any
    if (days > 0)
    {
        if (days == 1)
        {
            result += std::to_string(days) + " day, ";           // singular
        }
        else
        {
            result += std::to_string(days) + " days, ";          // plural
        }
    }
    // Add hours to the string if there are any
    if (hours > 0)
    {
        if (hours == 1)
        {
            result += std::to_string(hours) + " hour, ";         // singular
        }
        else
        {
            result += std::to_string(hours) + " hours, ";       // plural
        }
    }
    // Always add minutes (even if zero)
    if (minutes == 1)
    {
        result += std::to_string(minutes) + " minute";          // singular
    }
    else
    {
        result += std::to_string(minutes) + " minutes";         // plural
    }
    // Return the final human-readable uptime string
    return result;
}


//function to get os install date-------------------------------------------------------------------------------------------
std::string OSInfo::get_os_install_date()
{
    HRESULT hres;
    hres = CoInitializeEx(0, COINITBASE_MULTITHREADED);
    if (FAILED(hres)) return "Unknown";

    hres = CoInitializeSecurity(
        NULL,                     // Security descriptor
        -1,                       // Count of authentication services
        NULL,                     // Authentication services
        NULL,                     // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,// Authentication level
        RPC_C_IMP_LEVEL_IMPERSONATE, // Impersonation level
        NULL,                     // Authentication list
        EOAC_NONE,                // Capabilities
        NULL                      // Reserved

        );
    if (FAILED(hres)) return "Unknown";

    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);
    if (FAILED(hres)) return "Unknown";

    IWbemServices* pSvc = NULL;
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),  // Namespace
        NULL,                      // User
        NULL,                      // Password
        NULL,                      // Locale
        0,                         // Security flags
        NULL,                      // Authority
        NULL,                      // Context
        &pSvc                      // IWbemServices pointer
    );
    if (FAILED(hres)) { pLoc->Release(); return "Unknown"; }

    CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE);

    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT InstallDate FROM Win32_OperatingSystem"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL, &pEnumerator);
    if (FAILED(hres)) { pSvc->Release(); pLoc->Release(); return "Unknown"; }

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;
    std::string installDate = "Unknown";

    if (pEnumerator && pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK) {
        VARIANT vtProp;
        if (SUCCEEDED(pclsObj->Get(L"InstallDate", 0, &vtProp, 0, 0))) {
            // Convert WMI datetime string to readable format YYYY-MM-DD
            std::wstring wstr(vtProp.bstrVal, SysStringLen(vtProp.bstrVal));
            installDate = std::string(wstr.begin(), wstr.begin() + 4) + "-" +  // Year
                std::string(wstr.begin() + 4, wstr.begin() + 6) + "-" + // Month
                std::string(wstr.begin() + 6, wstr.begin() + 8);        // Day
        }
        VariantClear(&vtProp);
        pclsObj->Release();
    }

    if (pEnumerator) pEnumerator->Release();
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();

    return installDate;

}

//get os kernel version (major.major.build)
std::string OSInfo::get_os_kernel_info()
{
    std::string result = "WIN32_NT "; // platform prefix

    // Step 1: Get Major.Minor.Build
    HMODULE hMod = GetModuleHandleW(L"ntdll.dll");
    if (hMod)
    {
        typedef LONG(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
        RtlGetVersionPtr fn = (RtlGetVersionPtr)GetProcAddress(hMod, "RtlGetVersion");
        if (fn)
        {
            RTL_OSVERSIONINFOW rovi = { 0 };
            rovi.dwOSVersionInfoSize = sizeof(rovi);
            if (fn(&rovi) == 0)
            {
                result += std::to_string(rovi.dwMajorVersion) + "." +
                    std::to_string(rovi.dwMinorVersion) + "." +
                    std::to_string(rovi.dwBuildNumber);
            }
        }
    }

    // Step 2: Get UBR (update build revision)
    HKEY hKey;
    DWORD ubr = 0;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        DWORD type = 0;
        DWORD data_size = sizeof(DWORD);
        RegQueryValueExW(hKey, L"UBR", nullptr, &type, reinterpret_cast<BYTE*>(&ubr), &data_size);
        RegCloseKey(hKey);
        if (ubr != 0)
        {
            result += "." + std::to_string(ubr);
        }
    }

    // Step 3: Get Display Version / ReleaseId (like 23H2)
    wchar_t releaseId[20] = { 0 };
    DWORD bufSize = sizeof(releaseId);
    HKEY hKey2;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        0, KEY_READ, &hKey2) == ERROR_SUCCESS)
    {
        RegQueryValueExW(hKey2, L"DisplayVersion", nullptr, nullptr,
            reinterpret_cast<BYTE*>(releaseId), &bufSize);
        RegCloseKey(hKey2);
        if (wcslen(releaseId) > 0)
        {
            std::wstring ws(releaseId);
            std::string version(ws.begin(), ws.end());
            result += " (" + version + ")";
        }
    }

    return result;
}