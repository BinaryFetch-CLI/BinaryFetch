#include "include\CompactAudio.h"
#include <windows.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <comdef.h>
#include <atlbase.h>
#include <string>
using namespace std;

string get_audio_device_name(EDataFlow flow) {
    CoInitialize(NULL);
    CComPtr<IMMDeviceEnumerator> pEnum;
    pEnum.CoCreateInstance(__uuidof(MMDeviceEnumerator));

    CComPtr<IMMDevice> pDevice;
    if (FAILED(pEnum->GetDefaultAudioEndpoint(flow, eConsole, &pDevice))) {
        CoUninitialize();
        return flow == eRender ? "No speaker found" : "No microphone found";
    }

    CComPtr<IPropertyStore> pProps;
    pDevice->OpenPropertyStore(STGM_READ, &pProps);

    PROPVARIANT varName;
    PropVariantInit(&varName);
    pProps->GetValue(PKEY_Device_FriendlyName, &varName);

    // Convert wide string to string
    wstring ws(varName.pwszVal);
    string result(ws.begin(), ws.end());  // simple conversion

    PropVariantClear(&varName);
    CoUninitialize();
    return result;
}


string CompactAudio::active_audio_output() {
    return get_audio_device_name(eRender);
}

string CompactAudio::active_audio_output_status() {
    return "Active"; // Currently only fetching default active device
}

string CompactAudio::active_audio_input() {
    return get_audio_device_name(eCapture);
}

string CompactAudio::active_audio_input_status() {
    return "Active"; // Currently only fetching default active device
}
