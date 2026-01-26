#include "CompactMemory.h"
#include <comdef.h>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")

// ---------------------
// Basic RAM info
// ---------------------
double CompactMemory::get_total_memory() {
    MEMORYSTATUSEX mem = {};
    mem.dwLength = sizeof(mem);
    GlobalMemoryStatusEx(&mem);
    return static_cast<double>(mem.ullTotalPhys) / (1024.0 * 1024.0 * 1024.0);
}

double CompactMemory::get_free_memory() {
    MEMORYSTATUSEX mem = {};
    mem.dwLength = sizeof(mem);
    GlobalMemoryStatusEx(&mem);
    return static_cast<double>(mem.ullAvailPhys) / (1024.0 * 1024.0 * 1024.0);
}

double CompactMemory::get_used_memory_percent() {
    MEMORYSTATUSEX mem = {};
    mem.dwLength = sizeof(mem);
    GlobalMemoryStatusEx(&mem);
    return static_cast<double>(mem.dwMemoryLoad);
}

// ---------------------
// WMI Helper: Initialize COM & get IWbemServices*
// ---------------------
static IWbemServices* init_wmi() {
    HRESULT hres;
    IWbemLocator* pLoc = nullptr;
    IWbemServices* pSvc = nullptr;

    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) return nullptr;

    hres = CoInitializeSecurity(NULL, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE, NULL);
    if (FAILED(hres)) { CoUninitialize(); return nullptr; }

    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc);
    if (FAILED(hres)) { CoUninitialize(); return nullptr; }

    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL, NULL, 0, NULL, 0, 0, &pSvc
    );
    pLoc->Release();
    if (FAILED(hres)) { CoUninitialize(); return nullptr; }

    CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE);

    return pSvc;
}

// ---------------------
// RAM slots info
// ---------------------
int CompactMemory::memory_slot_used() {
    IWbemServices* pSvc = init_wmi();
    if (!pSvc) return 0;

    IEnumWbemClassObject* pEnum = nullptr;
    HRESULT hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_PhysicalMemory"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL, &pEnum);

    int count = 0;
    if (SUCCEEDED(hres) && pEnum) {
        IWbemClassObject* pObj = nullptr;
        ULONG uReturn = 0;
        while (pEnum->Next(WBEM_INFINITE, 1, &pObj, &uReturn) == S_OK) {
            count++;
            pObj->Release();
        }
        pEnum->Release();
    }

    pSvc->Release();
    CoUninitialize();
    return count;
}

int CompactMemory::memory_slot_available() {
    IWbemServices* pSvc = init_wmi();
    if (!pSvc) return 0;

    IEnumWbemClassObject* pEnum = nullptr;
    HRESULT hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT MemoryDevices FROM Win32_PhysicalMemoryArray"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL, &pEnum);

    int totalSlots = 0;
    if (SUCCEEDED(hres) && pEnum) {
        IWbemClassObject* pObj = nullptr;
        ULONG uReturn = 0;
        while (pEnum->Next(WBEM_INFINITE, 1, &pObj, &uReturn) == S_OK) {
            VARIANT vtProp;
            VariantInit(&vtProp);
            if (SUCCEEDED(pObj->Get(L"MemoryDevices", 0, &vtProp, 0, 0)) && vtProp.vt == VT_I4)
                totalSlots += vtProp.intVal;

            VariantClear(&vtProp);
            pObj->Release();
        }
        pEnum->Release();
    }

    pSvc->Release();
    CoUninitialize();
    return totalSlots;
}
