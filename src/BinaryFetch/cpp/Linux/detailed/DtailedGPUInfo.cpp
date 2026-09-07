#include "DetailedGPUInfo.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <glob.h>
#include <dlfcn.h>
#include <unistd.h>
#include <dirent.h>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cmath>
#include <memory>
#include <array>
#include <algorithm>
#include <nvml.h>

using namespace std;

// NVML is loaded at runtime (dlopen) so this still runs fine on systems
// with no NVIDIA driver installed at all — same approach as CompactGPU.
using nvmlInit_t = nvmlReturn_t (*)();
using nvmlShutdown_t = nvmlReturn_t (*)();
using nvmlDeviceGetHandleByIndex_t = nvmlReturn_t (*)(unsigned int, nvmlDevice_t*);
using nvmlDeviceGetName_t = nvmlReturn_t (*)(nvmlDevice_t, char*, unsigned int);
using nvmlDeviceGetMemoryInfo_t = nvmlReturn_t (*)(nvmlDevice_t, nvmlMemory_t*);
using nvmlDeviceGetClockInfo_t = nvmlReturn_t (*)(nvmlDevice_t, nvmlClockType_t, unsigned int*);
using nvmlDeviceGetPciInfo_t = nvmlReturn_t (*)(nvmlDevice_t, nvmlPciInfo_t*);

struct NVMLFunctions {
    void* handle = nullptr;
    nvmlInit_t init = nullptr;
    nvmlShutdown_t shutdown = nullptr;
    nvmlDeviceGetHandleByIndex_t getHandle = nullptr;
    nvmlDeviceGetName_t getName = nullptr;
    nvmlDeviceGetMemoryInfo_t getMemoryInfo = nullptr;
    nvmlDeviceGetClockInfo_t getClockInfo = nullptr;
    nvmlDeviceGetPciInfo_t getPciInfo = nullptr;
    bool initialized = false;

    bool load() {
        if (handle) return true;
        handle = dlopen("libnvidia-ml.so.1", RTLD_LAZY);
        if (!handle) handle = dlopen("libnvidia-ml.so", RTLD_LAZY);
        if (!handle) return false;

        init = reinterpret_cast<nvmlInit_t>(dlsym(handle, "nvmlInit_v2"));
        if (!init) init = reinterpret_cast<nvmlInit_t>(dlsym(handle, "nvmlInit"));

        shutdown = reinterpret_cast<nvmlShutdown_t>(dlsym(handle, "nvmlShutdown"));

        getHandle = reinterpret_cast<nvmlDeviceGetHandleByIndex_t>(
            dlsym(handle, "nvmlDeviceGetHandleByIndex_v2"));
        if (!getHandle) getHandle = reinterpret_cast<nvmlDeviceGetHandleByIndex_t>(
            dlsym(handle, "nvmlDeviceGetHandleByIndex"));

        getName = reinterpret_cast<nvmlDeviceGetName_t>(dlsym(handle, "nvmlDeviceGetName"));
        getMemoryInfo = reinterpret_cast<nvmlDeviceGetMemoryInfo_t>(dlsym(handle, "nvmlDeviceGetMemoryInfo"));
        getClockInfo = reinterpret_cast<nvmlDeviceGetClockInfo_t>(dlsym(handle, "nvmlDeviceGetClockInfo"));

        getPciInfo = reinterpret_cast<nvmlDeviceGetPciInfo_t>(dlsym(handle, "nvmlDeviceGetPciInfo_v3"));
        if (!getPciInfo) getPciInfo = reinterpret_cast<nvmlDeviceGetPciInfo_t>(dlsym(handle, "nvmlDeviceGetPciInfo_v2"));
        if (!getPciInfo) getPciInfo = reinterpret_cast<nvmlDeviceGetPciInfo_t>(dlsym(handle, "nvmlDeviceGetPciInfo"));

        if (!init || !shutdown || !getHandle) {
            dlclose(handle);
            handle = nullptr;
            return false;
        }
        return true;
    }

    bool ensureInit() {
        if (initialized) return true;
        if (!load()) return false;
        if (init() != NVML_SUCCESS) return false;
        initialized = true;
        return true;
    }

    void teardown() {
        if (initialized && shutdown) shutdown();
        initialized = false;
    }

    ~NVMLFunctions() {
        teardown();
        if (handle) dlclose(handle);
    }
};

static NVMLFunctions g_nvml;

// Small helpers
static string readFile(const string& path) {
    ifstream file(path);
    string value;
    if (file && getline(file, value)) return value;
    return "";
}

static string runCommand(const string& cmd) {
    string result;
    array<char, 256> buffer;
    string fullCmd = cmd + " 2>/dev/null";
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(fullCmd.c_str(), "r"), pclose);
    if (!pipe) return "";
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

static string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static string toUpperCopy(const string& s) {
    string out = s;
    transform(out.begin(), out.end(), out.begin(), ::toupper);
    return out;
}

// Estimate fallback frequency for GPUs where no live clock readout is
// available (ported directly from the Windows implementation).
static float estimate_gpu_frequency_basic(const wstring& gpuName) {
    wstring name = gpuName;
    transform(name.begin(), name.end(), name.begin(), ::towlower);

    if (name.find(L"rx 7900") != wstring::npos) return 2.5f;
    if (name.find(L"rx 7800") != wstring::npos) return 2.4f;
    if (name.find(L"rx 7700") != wstring::npos) return 2.3f;
    if (name.find(L"rx 6900") != wstring::npos) return 2.25f;
    if (name.find(L"rx 6800") != wstring::npos) return 2.1f;
    if (name.find(L"rx 6700") != wstring::npos) return 2.4f;

    if (name.find(L"arc a770") != wstring::npos) return 2.4f;
    if (name.find(L"arc a750") != wstring::npos) return 2.35f;
    if (name.find(L"arc a580") != wstring::npos) return 2.0f;

    if (name.find(L"intel") != wstring::npos && name.find(L"iris") != wstring::npos) return 1.3f;
    if (name.find(L"intel") != wstring::npos && name.find(L"uhd") != wstring::npos) return 1.15f;

    return 0.0f;
}

// DRM card enumeration — every physical GPU on the system, not just the
// one currently driving a display. "/sys/class/drm/cardN" (no dash) is a
// physical adapter; "cardN-<connector>" entries are outputs, not GPUs,
// and are skipped.
struct DrmCard {
    string cardPath;     // .../cardN/device
    string vendor;       // e.g. "0x10de"
    string device;       // PCI device id
    string pciAddress;   // e.g. "0000:01:00.0"
};

static string getPciAddress(const string& devicePath) {
    char buffer[4096];
    ssize_t length = readlink(devicePath.c_str(), buffer, sizeof(buffer) - 1);
    if (length <= 0) return "";
    buffer[length] = '\0';
    string fullPath(buffer);

    const string marker = "/0000:";
    size_t pos = fullPath.find(marker);
    if (pos == string::npos) return "";
    size_t start = pos + 1;
    size_t end = fullPath.find('/', start);
    if (end == string::npos) return "";
    return fullPath.substr(start, end - start);
}

static vector<DrmCard> enumerateCards() {
    vector<DrmCard> cards;
    glob_t g{};
    if (glob("/sys/class/drm/card[0-9]*", 0, nullptr, &g) != 0) return cards;

    for (size_t i = 0; i < g.gl_pathc; ++i) {
        string path = g.gl_pathv[i];
        size_t lastSlash = path.find_last_of('/');
        string dname = (lastSlash != string::npos) ? path.substr(lastSlash + 1) : path;

        // Skip connector entries like "card0-HDMI-A-1": everything after
        // "card" must be digits only for this to be a real adapter node.
        if (dname.size() <= 4) continue;
        bool pureCard = true;
        for (size_t k = 4; k < dname.size(); ++k) {
            if (!isdigit((unsigned char)dname[k])) { pureCard = false; break; }
        }
        if (!pureCard) continue;

        string devicePath = path + "/device";
        string vendor = readFile(devicePath + "/vendor");
        if (vendor.empty()) continue; // no backing device, not a real GPU

        DrmCard card;
        card.cardPath = devicePath;
        card.vendor = vendor;
        card.device = readFile(devicePath + "/device");
        card.pciAddress = getPciAddress(devicePath);
        cards.push_back(card);
    }
    globfree(&g);
    return cards;
}

// NVIDIA (NVML) — match an NVML device to a specific DRM card by PCI
// bus address so multi-GPU (e.g. laptop hybrid) systems report correctly
// instead of always grabbing device 0.
static bool matchNvmlDeviceByPci(const string& pciAddress, nvmlDevice_t& device) {
    if (!g_nvml.getPciInfo || pciAddress.empty()) {
        return g_nvml.getHandle && g_nvml.getHandle(0, &device) == NVML_SUCCESS;
    }

    // NVML's busId includes a domain that doesn't always match sysfs's
    // formatting exactly, so compare on the "bus:dev.func" tail only.
    size_t colon = pciAddress.find(':');
    string tail = (colon != string::npos) ? pciAddress.substr(colon + 1) : pciAddress;
    string tailUpper = toUpperCopy(tail);

    for (unsigned int index = 0; index < 32; ++index) {
        nvmlDevice_t candidate{};
        if (g_nvml.getHandle(index, &candidate) != NVML_SUCCESS) break;

        nvmlPciInfo_t pciInfo{};
        if (g_nvml.getPciInfo(candidate, &pciInfo) != NVML_SUCCESS) continue;

        string nvmlBusId = toUpperCopy(pciInfo.busId);
        if (nvmlBusId.find(tailUpper) != string::npos) {
            device = candidate;
            return true;
        }
    }

    // Fall back to device 0 rather than failing outright.
    return g_nvml.getHandle(0, &device) == NVML_SUCCESS;
}

// AMD / Intel sysfs readouts
static string getNameViaLspci(const string& pciAddress) {
    if (pciAddress.empty()) return "";
    string output = runCommand("lspci -s " + pciAddress);
    if (output.empty()) return "";
    // Typical line: "01:00.0 VGA compatible controller: AMD/ATI ... (rev c1)"
    size_t colon = output.find(": ");
    if (colon == string::npos) return "";
    string name = output.substr(colon + 2);
    size_t nl = name.find_first_of("\r\n");
    if (nl != string::npos) name = name.substr(0, nl);
    // Strip trailing "(rev xx)"
    size_t rev = name.rfind(" (rev");
    if (rev != string::npos) name = name.substr(0, rev);
    return trim(name);
}

static float getAMDVramGB(const string& cardPath) {
    ifstream file(cardPath + "/mem_info_vram_total");
    double bytes = 0;
    if (file && (file >> bytes)) {
        return static_cast<float>(bytes / (1024.0 * 1024.0 * 1024.0));
    }
    return 0.0f;
}

// Parses a line like "7: 2500Mhz *" and returns the frequency in GHz.
static float parseMhzLine(const string& line) {
    size_t mhzPos = line.find("Mhz");
    if (mhzPos == string::npos) mhzPos = line.find("MHz");
    if (mhzPos == string::npos) return 0.0f;
    size_t digitsEnd = mhzPos;
    size_t digitsStart = digitsEnd;
    while (digitsStart > 0 && isdigit((unsigned char)line[digitsStart - 1])) --digitsStart;
    if (digitsStart == digitsEnd) return 0.0f;
    try {
        int mhz = stoi(line.substr(digitsStart, digitsEnd - digitsStart));
        return mhz / 1000.0f;
    } catch (...) {
        return 0.0f;
    }
}

static float getAMDFrequencyGHz(const string& cardPath) {
    ifstream file(cardPath + "/pp_dpm_sclk");
    if (!file) return 0.0f;
    string line, lastLine;
    while (getline(file, line)) {
        if (line.empty()) continue;
        if (line.find('*') != string::npos) return parseMhzLine(line);
        lastLine = line;
    }
    if (!lastLine.empty()) return parseMhzLine(lastLine);
    return 0.0f;
}

static float getIntelFrequencyGHz(const string& cardPath) {
    ifstream file(cardPath + "/gt_cur_freq_mhz");
    int mhz = 0;
    if (file && (file >> mhz)) {
        return mhz / 1000.0f;
    }
    return 0.0f;
}

// DetailedGPUInfo
DetailedGPUInfo::DetailedGPUInfo() {}
DetailedGPUInfo::~DetailedGPUInfo() {}

vector<GPUData> DetailedGPUInfo::get_all_gpus() {
    vector<GPUData> gpus;

    vector<DrmCard> cards = enumerateCards();
    if (cards.empty()) return gpus;

    bool nvmlReady = g_nvml.ensureInit();

    int index = 0;
    for (const auto& card : cards) {
        GPUData gpu;
        gpu.index = index;
        gpu.name = "Unknown GPU";
        gpu.vram_gb = 0.0f;
        gpu.frequency_ghz = 0.0f;

        if (card.vendor == "0x10de") {
            // NVIDIA — use NVML
            gpu.name = "NVIDIA GPU";
            if (nvmlReady) {
                nvmlDevice_t device{};
                if (matchNvmlDeviceByPci(card.pciAddress, device)) {
                    if (g_nvml.getName) {
                        char name[128] = {0};
                        if (g_nvml.getName(device, name, sizeof(name)) == NVML_SUCCESS) {
                            gpu.name = string(name);
                        }
                    }
                    if (g_nvml.getMemoryInfo) {
                        nvmlMemory_t mem{};
                        if (g_nvml.getMemoryInfo(device, &mem) == NVML_SUCCESS) {
                            gpu.vram_gb = static_cast<float>(mem.total) / (1024.0f * 1024.0f * 1024.0f);
                        }
                    }
                    if (g_nvml.getClockInfo) {
                        unsigned int clockMHz = 0;
                        if (g_nvml.getClockInfo(device, NVML_CLOCK_GRAPHICS, &clockMHz) == NVML_SUCCESS) {
                            gpu.frequency_ghz = clockMHz / 1000.0f;
                        }
                    }
                }
            }
        }
        else if (card.vendor == "0x1002") {
            // AMD — sysfs
            string name = getNameViaLspci(card.pciAddress);
            gpu.name = name.empty() ? "AMD Graphics" : name;
            gpu.vram_gb = getAMDVramGB(card.cardPath);
            gpu.frequency_ghz = getAMDFrequencyGHz(card.cardPath);
            if (gpu.frequency_ghz <= 0.0f) {
                wstring wname(gpu.name.begin(), gpu.name.end());
                gpu.frequency_ghz = estimate_gpu_frequency_basic(wname);
            }
        }
        else if (card.vendor == "0x8086") {
            // Intel — integrated, no dedicated VRAM to report
            string name = getNameViaLspci(card.pciAddress);
            gpu.name = name.empty() ? "Intel Graphics" : name;
            gpu.vram_gb = 0.0f;
            gpu.frequency_ghz = getIntelFrequencyGHz(card.cardPath);
            if (gpu.frequency_ghz <= 0.0f) {
                wstring wname(gpu.name.begin(), gpu.name.end());
                gpu.frequency_ghz = estimate_gpu_frequency_basic(wname);
            }
        }
        else {
            // Unknown vendor — best-effort name only
            string name = getNameViaLspci(card.pciAddress);
            if (!name.empty()) gpu.name = name;
        }

        gpus.push_back(gpu);
        ++index;
    }

    if (nvmlReady) g_nvml.teardown();

    return gpus;
}

GPUData DetailedGPUInfo::primary_gpu_info() {
    auto gpus = get_all_gpus();
    if (!gpus.empty()) return gpus[0];
    return GPUData{ -1, "No GPU Found", 0.0f, 0.0f };
}