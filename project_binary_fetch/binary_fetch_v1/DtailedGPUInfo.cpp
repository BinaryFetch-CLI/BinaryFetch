#include "DetailedGPUInfo.h"
#include <windows.h> // The base library for Windows development. It provides the core API for memory management and hardware interaction.
#include <dxgi.h> // DirectX Graphics Infrastructure. Used to enumerate physical adapters (GPUs), check video memory capacity, and identify monitor outputs.
#include <vector> // Standard C++ library for using the vector container.
#include <string>
#include <algorithm>
#include <comdef.h>
#include "nvapi.h"

#pragma comment(lib,"dxgi.lib")
#pragma comment(lib, "nvapi64.lib")

DetailedGPUInfo::DetailedGPUInfo() {}
DetailedGPUInfo::~DetailedGPUInfo() {}

// Helper: Check if NVAPI is available
static bool is_nvapi_available()
{
    HMODULE nv = LoadLibraryA("nvapi64.dll");
    if (!nv) return false;
    FreeLibrary(nv);
    return true;
}

// Helper: Check if GPU is NVIDIA
static bool is_nvidia_gpu(UINT vendorId)
{
    return (vendorId == 0x10DE); // NVIDIA vendor ID
}

// Helper: Get GPU frequency using NVAPI
static float get_nvapi_gpu_frequency(NvPhysicalGpuHandle handle)
{
    NvU32 frequency = 0;

    // Method 1: Try current clock frequencies (most reliable)
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
            {
                // NVAPI returns frequency in kHz, convert to GHz
                // 2535000 kHz = 2535000 / 1000000 = 2.535 GHz
                return static_cast<float>(frequency) / 1000.0f / 1000.0f;
            }
        }
    }

    // Method 2: Try all clocks info as fallback
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
                // NVAPI returns frequency in kHz, convert to GHz
                return static_cast<float>(frequency) / 1000.0f / 1000.0f;
            }
        }
    }

    return 0.0f; // Failed to get frequency
}

// Helper: Estimate frequency for non-NVIDIA GPUs (basic method)
static float estimate_gpu_frequency_basic(const wstring& gpuName)
{
    // Basic estimation based on common GPU models
    // This is a fallback and won't be very accurate

    wstring name = gpuName;
    // Convert to lowercase for comparison
    transform(name.begin(), name.end(), name.begin(), ::towlower);

    // AMD GPUs - rough estimates
    if (name.find(L"rx 7900") != wstring::npos) return 2.5f;
    if (name.find(L"rx 7800") != wstring::npos) return 2.4f;
    if (name.find(L"rx 7700") != wstring::npos) return 2.3f;
    if (name.find(L"rx 6900") != wstring::npos) return 2.25f;
    if (name.find(L"rx 6800") != wstring::npos) return 2.1f;
    if (name.find(L"rx 6700") != wstring::npos) return 2.4f;

    // Intel Arc GPUs
    if (name.find(L"arc a770") != wstring::npos) return 2.4f;
    if (name.find(L"arc a750") != wstring::npos) return 2.35f;
    if (name.find(L"arc a580") != wstring::npos) return 2.0f;

    // Intel Integrated GPUs
    if (name.find(L"intel") != wstring::npos && name.find(L"iris") != wstring::npos)
        return 1.3f;
    if (name.find(L"intel") != wstring::npos && name.find(L"uhd") != wstring::npos)
        return 1.15f;

    // Default fallback
    return 0.0f; // Unknown - cannot estimate
}

vector<GPUData> DetailedGPUInfo::get_all_gpus()
{
    vector<GPUData> gpus;

    IDXGIFactory* pFactory = nullptr;
    if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory))) {
        return gpus;
    }

    // Initialize NVAPI if available
    bool nvapiInitialized = false;
    NvPhysicalGpuHandle nvapiHandles[NVAPI_MAX_PHYSICAL_GPUS] = { 0 };
    NvU32 nvapiGpuCount = 0;

    if (is_nvapi_available())
    {
        NvAPI_Status initStatus = NvAPI_Initialize();
        if (initStatus == NVAPI_OK)
        {
            nvapiInitialized = true;
            NvAPI_Status enumStatus = NvAPI_EnumPhysicalGPUs(nvapiHandles, &nvapiGpuCount);
            if (enumStatus != NVAPI_OK)
            {
                nvapiGpuCount = 0;
            }
        }
    }

    UINT i = 0;
    UINT nvidiaAdapterIndex = 0; // Separate counter for NVIDIA GPUs
    IDXGIAdapter* pAdapter = nullptr;

    while (pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC desc;
        pAdapter->GetDesc(&desc);

        GPUData gpu;
        gpu.index = i;

        // Convert wide string to string
        wstring ws(desc.Description);
        gpu.name = string(ws.begin(), ws.end());
        gpu.vram_gb = static_cast<float>(desc.DedicatedVideoMemory) / (1024.0f * 1024.0f * 1024.0f);

        // Get frequency based on GPU vendor
        gpu.frequency_ghz = 0.0f;

        if (is_nvidia_gpu(desc.VendorId) && nvapiInitialized && nvidiaAdapterIndex < nvapiGpuCount)
        {
            // NVIDIA GPU - use NVAPI
            NvPhysicalGpuHandle handle = nvapiHandles[nvidiaAdapterIndex];
            gpu.frequency_ghz = get_nvapi_gpu_frequency(handle);
            nvidiaAdapterIndex++;
        }
        else
        {
            // Non-NVIDIA GPU or NVAPI not available - use basic estimation
            gpu.frequency_ghz = estimate_gpu_frequency_basic(ws);
        }

        gpus.push_back(gpu);
        pAdapter->Release();
        i++;
    }

    // Clean up NVAPI
    if (nvapiInitialized)
    {
        NvAPI_Unload();
    }

    if (pFactory) pFactory->Release();
    return gpus;
}

GPUData DetailedGPUInfo::primary_gpu_info()
{
    auto gpus = get_all_gpus();
    if (!gpus.empty()) return gpus[0];
    return GPUData{ -1, "No GPU Found", 0.0f, 0.0f };
}