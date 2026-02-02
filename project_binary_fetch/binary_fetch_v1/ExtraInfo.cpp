#include "include\ExtraInfo.h"
#include <windows.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <powrprof.h>
#include <sstream>
using namespace std;

/**
 * Retrieves all audio OUTPUT devices (speakers/headphones) on the system
 * @return Vector of AudioDevice structs containing device info
 */
vector<AudioDevice> ExtraInfo::get_output_devices()
{
    vector<AudioDevice> devices;
    HRESULT hr;

    // Initialize COM library for current thread
    CoInitialize(nullptr);

    // Pointers for device enumeration
    IMMDeviceEnumerator* pEnum = nullptr;
    IMMDeviceCollection* pDevices = nullptr;

    // Create an instance of the device enumerator
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator), (void**)&pEnum);
    if (FAILED(hr))
    {
        CoUninitialize();
        return devices;
    }

    // Enumerate all rendering (output) audio devices, both active and disabled
    hr = pEnum->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE | DEVICE_STATE_DISABLED, &pDevices);
    if (SUCCEEDED(hr))
    {
        UINT count = 0;
        pDevices->GetCount(&count);  // Get total number of output devices

        // Get the default output device to compare later
        IMMDevice* pDefaultOut = nullptr;
        pEnum->GetDefaultAudioEndpoint(eRender, eConsole, &pDefaultOut);

        // Iterate through all output devices
        for (UINT i = 0; i < count; i++)
        {
            IMMDevice* pDevice = nullptr;
            IPropertyStore* pProps = nullptr;
            LPWSTR deviceId = nullptr;

            // Get device at index i
            pDevices->Item(i, &pDevice);

            // Open property store to read device properties
            pDevice->OpenPropertyStore(STGM_READ, &pProps);

            // Initialize and get the friendly name of the device
            PROPVARIANT name;
            PropVariantInit(&name);
            pProps->GetValue(PKEY_Device_FriendlyName, &name);

            // Get unique device ID
            pDevice->GetId(&deviceId);

            // Convert wide string to standard string
            wstring wname(name.pwszVal);
            string deviceName(wname.begin(), wname.end());

            // Check if this device is the active (default) output device
            bool isActive = false;
            if (pDefaultOut)
            {
                LPWSTR defId = nullptr;
                pDefaultOut->GetId(&defId);

                // Compare device IDs
                if (wcscmp(deviceId, defId) == 0)
                    isActive = true;

                CoTaskMemFree(defId);  // Free allocated memory
            }

            // Store device information
            AudioDevice device;
            device.name = deviceName;
            device.isActive = isActive;
            device.isOutput = true;
            devices.push_back(device);

            // Clean up resources for this device
            PropVariantClear(&name);
            if (pProps) pProps->Release();
            if (pDevice) pDevice->Release();
            CoTaskMemFree(deviceId);
        }

        // Release default output device and collection
        if (pDefaultOut) pDefaultOut->Release();
        pDevices->Release();
    }

    // Release enumerator and uninitialize COM
    if (pEnum) pEnum->Release();
    CoUninitialize();

    return devices;
}

/**
 * Retrieves all audio INPUT devices (microphones) on the system
 * @return Vector of AudioDevice structs containing device info
 */
vector<AudioDevice> ExtraInfo::get_input_devices()
{
    vector<AudioDevice> devices;
    HRESULT hr;

    // Initialize COM library for current thread
    CoInitialize(nullptr);

    // Pointers for device enumeration
    IMMDeviceEnumerator* pEnum = nullptr;
    IMMDeviceCollection* pDevices = nullptr;

    // Create an instance of the device enumerator
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator), (void**)&pEnum);
    if (FAILED(hr))
    {
        CoUninitialize();
        return devices;
    }

    // Enumerate all capture (input) audio devices, both active and disabled
    hr = pEnum->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE | DEVICE_STATE_DISABLED, &pDevices);
    if (SUCCEEDED(hr))
    {
        UINT count = 0;
        pDevices->GetCount(&count);  // Get total number of input devices

        // Get the default input device to compare later
        IMMDevice* pDefaultIn = nullptr;
        pEnum->GetDefaultAudioEndpoint(eCapture, eConsole, &pDefaultIn);

        // Iterate through all input devices
        for (UINT i = 0; i < count; i++)
        {
            IMMDevice* pDevice = nullptr;
            IPropertyStore* pProps = nullptr;
            LPWSTR deviceId = nullptr;

            // Get device at index i
            pDevices->Item(i, &pDevice);

            // Open property store to read device properties
            pDevice->OpenPropertyStore(STGM_READ, &pProps);

            // Initialize and get the friendly name of the device
            PROPVARIANT name;
            PropVariantInit(&name);
            pProps->GetValue(PKEY_Device_FriendlyName, &name);

            // Get unique device ID
            pDevice->GetId(&deviceId);

            // Convert wide string to standard string
            wstring wname(name.pwszVal);
            string deviceName(wname.begin(), wname.end());

            // Check if this device is the active (default) input device
            bool isActive = false;
            if (pDefaultIn)
            {
                LPWSTR defId = nullptr;
                pDefaultIn->GetId(&defId);

                // Compare device IDs
                if (wcscmp(deviceId, defId) == 0)
                    isActive = true;

                CoTaskMemFree(defId);  // Free allocated memory
            }

            // Store device information
            AudioDevice device;
            device.name = deviceName;
            device.isActive = isActive;
            device.isOutput = false;
            devices.push_back(device);

            // Clean up resources for this device
            PropVariantClear(&name);
            if (pProps) pProps->Release();
            if (pDevice) pDevice->Release();
            CoTaskMemFree(deviceId);
        }

        // Release default input device and collection
        if (pDefaultIn) pDefaultIn->Release();
        pDevices->Release();
    }

    // Release enumerator and uninitialize COM
    if (pEnum) pEnum->Release();
    CoUninitialize();

    return devices;
}

/**
 * Retrieves system power status information
 * @return PowerStatus struct containing power information
 */
PowerStatus ExtraInfo::get_power_status()
{
    PowerStatus status;
    SYSTEM_POWER_STATUS sps;

    // Get system power status information
    if (!GetSystemPowerStatus(&sps))
    {
        status.hasBattery = false;
        status.isCharging = false;
        status.batteryPercent = 0;
        status.isACOnline = false;
        return status;
    }

    // Check if system has a battery (BatteryFlag == 128 means no battery)
    status.hasBattery = (sps.BatteryFlag != 128);
    status.batteryPercent = (int)sps.BatteryLifePercent;
    status.isACOnline = (sps.ACLineStatus == 1);
    status.isCharging = status.isACOnline;

    return status;
}

/*
================================================================================
                        END-OF-FILE DOCUMENTATION
================================================================================

FUNCTION REFERENCE:

1. SetConsoleTextAttribute(HANDLE, WORD)
   - Sets text color/attributes for console output
   - Parameters: Console handle, color/attribute code
   - Returns: TRUE on success

2. GetStdHandle(DWORD)
   - Retrieves handle to standard input/output/error device
   - STD_OUTPUT_HANDLE = standard output (console)
   - Returns: Handle to specified device

3. CoInitialize(LPVOID)
   - Initializes COM library for current thread
   - Must be called before using COM objects
   - Parameter: Reserved (use nullptr)
   - Returns: S_OK on success

4. CoCreateInstance(REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID*)
   - Creates single object of specified class
   - Used to create MMDeviceEnumerator instance
   - Returns: HRESULT indicating success/failure

5. EnumAudioEndpoints(EDataFlow, DWORD, IMMDeviceCollection**)
   - Enumerates audio endpoint devices
   - EDataFlow: eRender (output) or eCapture (input)
   - DWORD: Device state filter (ACTIVE, DISABLED, etc.)
   - Returns: Collection of devices

6. GetDefaultAudioEndpoint(EDataFlow, ERole, IMMDevice**)
   - Gets default audio endpoint for specified data flow
   - ERole: eConsole, eMultimedia, or eCommunications
   - Returns: Default device interface

7. GetCount(UINT*)
   - Gets number of devices in collection
   - Parameter: Pointer to receive count
   - Returns: S_OK on success

8. Item(UINT, IMMDevice**)
   - Gets device at specified index from collection
   - Returns: Device interface pointer

9. OpenPropertyStore(DWORD, IPropertyStore**)
   - Opens property store for device
   - STGM_READ: Read-only access
   - Returns: Property store interface

10. PropVariantInit(PROPVARIANT*)
    - Initializes PROPVARIANT structure
    - Must be called before using PROPVARIANT

11. GetValue(REFPROPERTYKEY, PROPVARIANT*)
    - Retrieves property value from store
    - PKEY_Device_FriendlyName: Device's display name
    - Returns: S_OK on success

12. GetId(LPWSTR*)
    - Gets unique device identifier string
    - Returns: Wide string device ID

13. wcscmp(const wchar_t*, const wchar_t*)
    - Compares two wide-character strings
    - Returns: 0 if equal, <0 if first < second, >0 if first > second

14. PropVariantClear(PROPVARIANT*)
    - Frees memory used by PROPVARIANT structure
    - Must be called after finishing with PROPVARIANT

15. Release()
    - Decrements COM object reference count
    - Frees object when count reaches zero
    - Called on all COM interfaces when done

16. CoTaskMemFree(LPVOID)
    - Frees memory allocated by COM task memory allocator
    - Used for strings allocated by GetId()

17. CoUninitialize()
    - Closes COM library for current thread
    - Must be called after CoInitialize()

18. GetSystemPowerStatus(LPSYSTEM_POWER_STATUS)
    - Retrieves system power status
    - Returns: TRUE on success, FALSE on failure

STRUCTURES USED:

- SYSTEM_POWER_STATUS: Contains power status information
  * ACLineStatus: 0=offline, 1=online, 255=unknown
  * BatteryFlag: Battery status (128=no battery)
  * BatteryLifePercent: 0-100 or 255 if unknown
  * BatteryLifeTime: Seconds of battery life remaining
  * BatteryFullLifeTime: Seconds when fully charged

- PROPVARIANT: Variant data type for property values
  * pwszVal: Wide string value for device names

- AudioDevice: Custom struct for storing device information
  * name: Device friendly name
  * isActive: Whether device is the default/active device
  * isOutput: True for output devices, false for input devices

- PowerStatus: Custom struct for storing power information
  * hasBattery: Whether system has a battery
  * batteryPercent: Battery charge percentage (0-100)
  * isACOnline: Whether AC power is connected
  * isCharging: Whether battery is charging

COLOR CODES USED (for main.cpp reference):
- 7  = Light Gray (Default)
- 10 = Light Green (Active status)
- 11 = Light Cyan/Sky Blue (Brackets)
- 12 = Light Red (Not charging)
- 14 = Yellow/Orange (Battery percentage)

================================================================================
*/