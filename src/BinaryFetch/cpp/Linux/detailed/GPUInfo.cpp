#include "GPUInfo.h"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <glob.h>
#include <unistd.h>
#include <dirent.h>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <algorithm>

using namespace std;

// Reads the first line from a file.
static string readFile(const string& path) {
    ifstream file(path);
    string value;

    if (file && getline(file, value)) {
        return value;
    }

    return "";
}

// Runs a shell command and captures stdout.
// stderr is discarded so optional tools never spam BinaryFetch output.
static string runCommand(const string& cmd) {
    string result;
    char buffer[256];

    string fullCmd = cmd + " 2>/dev/null";

    FILE* pipe = popen(fullCmd.c_str(), "r");
    if (!pipe) {
        return "";
    }

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    pclose(pipe);

    return result;
}

// Removes whitespace from the beginning and end of a string.
static string trim(const string& value) {
    size_t start = value.find_first_not_of(" \t\r\n");

    if (start == string::npos) {
        return "";
    }

    size_t end = value.find_last_not_of(" \t\r\n");

    return value.substr(start, end - start + 1);
}

// Formats bytes as MB.
static string formatMemoryMB(unsigned long long bytes) {
    if (bytes == 0) {
        return "Unknown";
    }

    ostringstream ss;

    ss << fixed
       << setprecision(0)
       << (static_cast<double>(bytes) / (1024.0 * 1024.0))
       << " MB";

    return ss.str();
}

// Formats KiB values as MB.
static string formatMemoryKiB(unsigned long long kib) {
    if (kib == 0) {
        return "Unknown";
    }

    return formatMemoryMB(kib * 1024ULL);
}

namespace {

// DRM card enumeration, one entry per physical GPU adapter.
struct DrmCard {
    string cardPath;
    string vendor;
    string pciAddress;
};

}

// Gets the PCI address from the DRM device symlink.
static string getPciAddress(const string& devicePath) {
    char buffer[4096];

    ssize_t length = readlink(
        devicePath.c_str(),
        buffer,
        sizeof(buffer) - 1
    );

    if (length <= 0) {
        return "";
    }

    buffer[length] = '\0';

    string fullPath(buffer);

    const string marker = "/0000:";

    size_t pos = fullPath.find(marker);

    if (pos == string::npos) {
        return "";
    }

    size_t start = pos + 1;
    size_t end = fullPath.find('/', start);

    if (end == string::npos) {
        return "";
    }

    return fullPath.substr(start, end - start);
}

// Finds all physical DRM cards.
// This does not depend on NVIDIA, AMD, or Intel-specific APIs.
static vector<DrmCard> enumerateCards() {
    vector<DrmCard> cards;

    glob_t g{};

    if (glob("/sys/class/drm/card[0-9]*", 0, nullptr, &g) != 0) {
        return cards;
    }

    for (size_t i = 0; i < g.gl_pathc; ++i) {
        string path = g.gl_pathv[i];

        size_t lastSlash = path.find_last_of('/');

        string dname =
            (lastSlash != string::npos)
                ? path.substr(lastSlash + 1)
                : path;

        // Ignore connector entries such as card0-HDMI-A-1.
        if (dname.size() <= 4) {
            continue;
        }

        bool pureCard = true;

        for (size_t k = 4; k < dname.size(); ++k) {
            if (!isdigit(static_cast<unsigned char>(dname[k]))) {
                pureCard = false;
                break;
            }
        }

        if (!pureCard) {
            continue;
        }

        string devicePath = path + "/device";

        string vendor = trim(
            readFile(devicePath + "/vendor")
        );

        if (vendor.empty()) {
            continue;
        }

        DrmCard card;

        card.cardPath = devicePath;
        card.vendor = vendor;
        card.pciAddress = getPciAddress(devicePath);

        cards.push_back(card);
    }

    globfree(&g);

    return cards;
}

// Gets a human-readable GPU name using lspci when available.
static string getNameViaLspci(const string& pciAddress) {
    if (pciAddress.empty()) {
        return "";
    }

    string output = runCommand(
        "lspci -s " + pciAddress
    );

    if (output.empty()) {
        return "";
    }

    size_t colon = output.find(": ");

    if (colon == string::npos) {
        return "";
    }

    string name = output.substr(colon + 2);

    size_t newline = name.find_first_of("\r\n");

    if (newline != string::npos) {
        name = name.substr(0, newline);
    }

    size_t rev = name.rfind(" (rev");

    if (rev != string::npos) {
        name = name.substr(0, rev);
    }

    return trim(name);
}

// Reads the Linux kernel module version.
static string getKernelModuleVersion(const string& moduleName) {
    string version = trim(
        readFile("/sys/module/" + moduleName + "/version")
    );

    if (!version.empty()) {
        return version;
    }

    string output = runCommand(
        "modinfo -F version " + moduleName
    );

    return trim(output);
}

// Reads NVIDIA driver version without touching NVML.
static string getNvidiaDriverVersion() {
    string version = trim(
        readFile("/proc/driver/nvidia/version")
    );

    if (!version.empty()) {
        // Example:
        // NVRM version: NVIDIA UNIX x86_64 Kernel Module  570.xx.xx
        size_t marker = version.find("Kernel Module");

        if (marker != string::npos) {
            string result = trim(
                version.substr(marker + strlen("Kernel Module"))
            );

            if (!result.empty()) {
                return result;
            }
        }

        return version;
    }

    version = getKernelModuleVersion("nvidia");

    if (!version.empty()) {
        return version;
    }

    return "Unknown";
}

// Reads a numeric value from a sysfs file.
static bool readUnsignedLongLong(
    const string& path,
    unsigned long long& value
) {
    ifstream file(path);

    if (!file) {
        return false;
    }

    file >> value;

    return !file.fail();
}

// Reads an integer from a sysfs file.
static bool readInt(
    const string& path,
    int& value
) {
    ifstream file(path);

    if (!file) {
        return false;
    }

    file >> value;

    return !file.fail();
}

// Gets AMD VRAM size from amdgpu sysfs.
static string getAMDVram(
    const string& cardPath
) {
    unsigned long long bytes = 0;

    if (readUnsignedLongLong(
            cardPath + "/mem_info_vram_total",
            bytes)) {
        return formatMemoryMB(bytes);
    }

    return "Unknown";
}

// Gets AMD GPU usage.
static float getAMDUsage(
    const string& cardPath
) {
    int usage = -1;

    if (readInt(
            cardPath + "/gpu_busy_percent",
            usage)) {
        return static_cast<float>(usage);
    }

    return -1.0f;
}

// Gets AMD GPU temperature.
static float getAMDTemperature(
    const string& cardPath
) {
    string pattern =
        cardPath +
        "/hwmon/hwmon*/temp1_input";

    glob_t files{};

    if (glob(
            pattern.c_str(),
            0,
            nullptr,
            &files) != 0) {
        return 0.0f;
    }

    float result = 0.0f;

    for (size_t i = 0; i < files.gl_pathc; ++i) {
        unsigned long long milli = 0;

        ifstream file(files.gl_pathv[i]);

        if (file && (file >> milli)) {
            result =
                static_cast<float>(
                    milli / 1000.0
                );

            break;
        }
    }

    globfree(&files);

    return result;
}

// Gets the current AMD graphics clock.
static float getAMDFrequency(
    const string& cardPath
) {
    ifstream file(
        cardPath + "/pp_dpm_sclk"
    );

    if (!file) {
        return 0.0f;
    }

    string line;

    while (getline(file, line)) {
        if (line.find('*') == string::npos) {
            continue;
        }

        size_t mhzPos = line.find("Mhz");

        if (mhzPos == string::npos) {
            mhzPos = line.find("MHz");
        }

        if (mhzPos == string::npos) {
            continue;
        }

        size_t end = mhzPos;
        size_t start = end;

        while (
            start > 0 &&
            isdigit(
                static_cast<unsigned char>(
                    line[start - 1]
                )
            )
        ) {
            --start;
        }

        if (start == end) {
            continue;
        }

        try {
            return static_cast<float>(
                stoi(
                    line.substr(
                        start,
                        end - start
                    )
                )
            );
        } catch (...) {
            return 0.0f;
        }
    }

    return 0.0f;
}

// Gets Intel GPU frequency.
static float getIntelFrequency(
    const string& cardPath
) {
    int mhz = 0;

    if (readInt(
            cardPath + "/gt_cur_freq_mhz",
            mhz)) {
        return static_cast<float>(mhz);
    }

    return 0.0f;
}

// CUDA core count is a fixed hardware property. Neither NVML nor
// nvidia-smi report it directly (it isn't a runtime metric), so the
// common approach - used by tools like nvtop/gpustat - is a static
// lookup table keyed off the model name reported by nvidia-smi/lspci.
// Matched by substring so minor name variations (e.g. trailing
// "/PCIe/SSE2" from lspci, or "Laptop GPU" suffixes) still resolve.
struct NvidiaCoreEntry {
    const char* match;
    int cudaCores;
};






// Nvidia GPUs and their CUDA core counts. This is not exhaustive, but covers the most common models.
// CUDA didn't exist before the GeForce 8800 (G80, Nov 2006), so nothing earlier is included.
// 2006-2026
static const NvidiaCoreEntry kNvidiaCoreTable[] = {
    //  Blackwell (RTX 50 series, 2025) 
    { "RTX 5090", 21760 },
    { "RTX 5080", 10752 },
    { "RTX 5070 Ti Super", 10240 },
    { "RTX 5070 Ti", 8960 },
    { "RTX 5070", 6144 },
    { "RTX 5060 Ti", 4608 },
    { "RTX 5060", 3840 },
    { "RTX 5050", 2560 },
    { "RTX PRO 6000 Blackwell", 24064 },

    //  Blackwell (datacenter, 2024-) 
    { "GB200", 20480 },
    { "B200", 20480 },
    { "B100", 14592 },

    //  Ada Lovelace (RTX 40 series, 2022-2023) 
    { "RTX 4090", 16384 },
    { "RTX 4080 SUPER", 10240 },
    { "RTX 4080", 9728 },
    { "RTX 4070 Ti SUPER", 8448 },
    { "RTX 4070 Ti", 7680 },
    { "RTX 4070 SUPER", 7168 },
    { "RTX 4070", 5888 },
    { "RTX 4060 Ti", 4352 },
    { "RTX 4060", 3072 },
    { "RTX 4050", 2560 },

    //  Hopper (datacenter, 2022-2024) 
    { "H200", 16896 },
    { "H100", 16896 },

    //  Ampere (RTX 30 series, 2020-2022) 
    { "RTX 3090 Ti", 10752 },
    { "RTX 3090", 10496 },
    { "RTX 3080 Ti", 10240 },
    { "RTX 3080 12GB", 8960 },
    { "RTX 3080", 8704 },
    { "RTX 3070 Ti", 6144 },
    { "RTX 3070", 5888 },
    { "RTX 3060 Ti", 4864 },
    { "RTX 3060", 3584 },
    { "RTX 3050 Ti", 2560 },
    { "RTX 3050", 2560 },

    //  Ampere (datacenter, 2020-) 
    { "A100", 6912 },
    { "A40", 10752 },
    { "A30", 3804 },
    { "A10", 9216 },

    //  Turing (RTX 20 / GTX 16 series, 2018-2019) 
    { "TITAN RTX", 4608 },
    { "RTX 2080 Ti", 4352 },
    { "RTX 2080 SUPER", 3072 },
    { "RTX 2080", 2944 },
    { "RTX 2070 SUPER", 2560 },
    { "RTX 2070", 2304 },
    { "RTX 2060 SUPER", 2176 },
    { "RTX 2060", 1920 },
    { "GTX 1660 Ti", 1536 },
    { "GTX 1660 SUPER", 1408 },
    { "GTX 1660", 1408 },
    { "GTX 1650 SUPER", 1280 },
    { "GTX 1650", 896 },
    { "MX550", 1024 },
    { "MX450", 896 },

    //  Turing (datacenter, 2018-) 
    { "Tesla T4", 2560 },

    //  Volta (2017-2018) 
    { "TITAN V", 5120 },
    { "Tesla V100", 5120 },

    //  Pascal (GTX 10 series, 2016-2017) 
    { "TITAN Xp", 3840 },
    { "TITAN X (Pascal)", 3584 },
    { "GTX 1080 Ti", 3584 },
    { "GTX 1080", 2560 },
    { "GTX 1070 Ti", 2432 },
    { "GTX 1070", 1920 },
    { "GTX 1060", 1280 },
    { "GTX 1050 Ti", 768 },
    { "GTX 1050", 640 },
    { "MX350", 640 },
    { "MX250", 384 },
    { "MX150", 384 },

    //  Pascal (datacenter, 2016-) 
    { "Tesla P100", 3584 },
    { "Tesla P40", 3840 },
    { "Tesla P4", 2560 },

    //  Maxwell (GTX 900 series, 2014-2015) 
    { "GTX TITAN X", 3072 },
    { "GTX 980 Ti", 2816 },
    { "GTX 980", 2048 },
    { "GTX 970", 1664 },
    { "GTX 960", 1024 },
    { "GTX 950", 768 },

    //  Maxwell (datacenter, 2015-) 
    { "Tesla M60", 4096 },
    { "Tesla M40", 3072 },
    { "Tesla M4", 1024 },

    //  Kepler (GTX 600/700 series, 2012-2014) 
    { "GTX TITAN Black", 2880 },
    { "GTX TITAN Z", 5760 },
    { "GTX TITAN", 2688 },
    { "GTX 780 Ti", 2880 },
    { "GTX 780", 2304 },
    { "GTX 770", 1536 },
    { "GTX 760", 1152 },
    { "GTX 750 Ti", 640 },
    { "GTX 750", 512 },
    { "GTX 690", 3072 },
    { "GTX 680", 1536 },
    { "GTX 670", 1344 },
    { "GTX 660 Ti", 1344 },
    { "GTX 660", 960 },
    { "GTX 650 Ti Boost", 768 },
    { "GTX 650 Ti", 768 },
    { "GTX 650", 384 },

    //  Kepler (datacenter, 2012-) 
    { "Tesla K80", 4992 },
    { "Tesla K40", 2880 },
    { "Tesla K20X", 2688 },
    { "Tesla K20", 2496 },
    { "Tesla K10", 3072 },

    //  Blackwell (datacenter, 2024-) 
    { "GB200", 20480 },
    { "GB10", 6144 },   // DGX Spark's integrated Blackwell GPU (Grace Blackwell Superchip)
    { "B200", 20480 },
    { "B100", 14592 },

    //  Fermi (GTX 400/500 series, 2010-2012) 
    { "GTX 590", 1024 },
    { "GTX 580", 512 },
    { "GTX 570", 480 },
    { "GTX 560 Ti", 384 },
    { "GTX 560", 336 },
    { "GTX 550 Ti", 192 },
    { "GTX 480", 480 },
    { "GTX 470", 448 },
    { "GTX 460", 336 },
    { "GTX 450", 192 },

    //  Fermi (datacenter, 2010-) 
    { "Tesla M2090", 512 },
    { "Tesla C2075", 448 },
    { "Tesla C2050", 448 },

    //  Tesla architecture (GeForce 8/9/100/200/300 series, 2006-2010) 
    // The "Tesla" microarchitecture predates and is unrelated to the
    // "Tesla" datacenter product line above; these were the first
    // GPUs to expose CUDA cores at all.
    { "GTX 295", 480 },
    { "GTX 285", 240 },
    { "GTX 280", 240 },
    { "GTX 260", 216 },
    { "GTS 250", 128 },
    { "GT 240", 96 },
    { "GT 220", 48 },
    { "9800 GX2", 256 },
    { "9800 GTX+", 128 },
    { "9800 GTX", 128 },
    { "9800 GT", 112 },
    { "9600 GT", 64 },
    { "9600 GSO", 96 },
    { "9500 GT", 32 },
    { "8800 Ultra", 128 },
    { "8800 GTX", 128 },
    { "8800 GTS", 96 },
    { "8800 GT", 112 },
    { "8800 GS", 96 },
    { "8600 GTS", 32 },
    { "8600 GT", 32 },
    { "8500 GT", 16 },
    { "8400 GS", 16 },
};




// Looks up CUDA core count from the GPU model name.
// Returns 0 (Unknown) when the model isn't in the table.
static int lookupNvidiaCoreCount(const string& name) {
    for (const auto& entry : kNvidiaCoreTable) {
        if (name.find(entry.match) != string::npos) {
            return entry.cudaCores;
        }
    }

    return 0;
}

// Gets NVIDIA data through nvidia-smi.
// nvidia-smi is used only when NVIDIA hardware is detected.
static bool getNvidiaStats(
    unsigned int index,
    string& name,
    string& memory,
    float& usage,
    float& temperature,
    float& frequency
) {
    string base =
        "nvidia-smi -i " +
        to_string(index) +
        " --query-gpu=name,memory.total,utilization.gpu,temperature.gpu,clocks.gr "
        "--format=csv,noheader,nounits";

    string output = runCommand(base);

    output = trim(output);

    if (output.empty()) {
        return false;
    }

    stringstream ss(output);

    string nameValue;
    string memoryValue;
    string usageValue;
    string temperatureValue;
    string frequencyValue;

    if (!getline(ss, nameValue, ',')) {
        return false;
    }

    if (!getline(ss, memoryValue, ',')) {
        return false;
    }

    if (!getline(ss, usageValue, ',')) {
        return false;
    }

    if (!getline(ss, temperatureValue, ',')) {
        return false;
    }

    if (!getline(ss, frequencyValue, ',')) {
        return false;
    }

    nameValue = trim(nameValue);
    memoryValue = trim(memoryValue);
    usageValue = trim(usageValue);
    temperatureValue = trim(temperatureValue);
    frequencyValue = trim(frequencyValue);

    // nvidia-smi reports "[N/A]" (or "N/A") for fields a given GPU or
    // driver doesn't support instead of leaving them blank, so a plain
    // emptiness check above isn't enough - that string would otherwise
    // reach stof() and get silently treated as garbage.
    auto isUnavailable = [](const string& value) {
        return value.empty() ||
               value == "N/A" ||
               value == "[N/A]" ||
               value == "[Not Supported]";
    };

    if (!isUnavailable(nameValue)) {
        name = nameValue;
    }

    if (!isUnavailable(memoryValue)) {
        memory = memoryValue + " MB";
    } else {
        memory = "Unknown";
    }

    if (isUnavailable(usageValue)) {
        usage = -1.0f;
    } else {
        try {
            usage = stof(usageValue);
        } catch (...) {
            usage = -1.0f;
        }
    }

    if (isUnavailable(temperatureValue)) {
        temperature = 0.0f;
    } else {
        try {
            temperature = stof(temperatureValue);
        } catch (...) {
            temperature = 0.0f;
        }
    }

    if (isUnavailable(frequencyValue)) {
        frequency = 0.0f;
    } else {
        try {
            frequency = stof(frequencyValue);
        } catch (...) {
            frequency = 0.0f;
        }
    }

    return true;
}

// Fills one gpu_data entry from a DRM card.
static gpu_data buildGpuData(
    const DrmCard& card,
    unsigned int nvidiaIndex
) {
    gpu_data data{};

    data.gpu_name = "Unknown GPU";
    data.gpu_memory = "Unknown";
    data.gpu_driver_version = "Unknown";
    data.gpu_vendor = "Unknown";

    data.gpu_usage = -1.0f;
    data.gpu_temperature = 0.0f;
    data.gpu_core_count = 0;
    data.gpu_frequency = 0.0f;

    // NVIDIA
    if (card.vendor == "0x10de") {
        data.gpu_vendor = "NVIDIA";
        data.gpu_driver_version =
            getNvidiaDriverVersion();

        data.gpu_name =
            getNameViaLspci(
                card.pciAddress
            );

        if (data.gpu_name.empty()) {
            data.gpu_name = "NVIDIA GPU";
        }

        string nvidiaName =
            data.gpu_name;

        string nvidiaMemory =
            data.gpu_memory;

        float nvidiaUsage =
            data.gpu_usage;

        float nvidiaTemperature =
            data.gpu_temperature;

        float nvidiaFrequency =
            data.gpu_frequency;

        if (getNvidiaStats(
                nvidiaIndex,
                nvidiaName,
                nvidiaMemory,
                nvidiaUsage,
                nvidiaTemperature,
                nvidiaFrequency)) {

            data.gpu_name = nvidiaName;
            data.gpu_memory = nvidiaMemory;
            data.gpu_usage = nvidiaUsage;
            data.gpu_temperature = nvidiaTemperature;
            data.gpu_frequency = nvidiaFrequency;
        }

        // NVML/nvidia-smi don't expose CUDA core count as a runtime
        // metric (it's fixed per model), so it's resolved from the
        // model name instead. Falls back to 0 ("Unknown") for models
        // not in the table.
        data.gpu_core_count = lookupNvidiaCoreCount(data.gpu_name);
    }

    // AMD
    else if (card.vendor == "0x1002") {
        data.gpu_vendor = "AMD";

        data.gpu_name =
            getNameViaLspci(
                card.pciAddress
            );

        if (data.gpu_name.empty()) {
            data.gpu_name = "AMD Graphics";
        }

        data.gpu_memory =
            getAMDVram(
                card.cardPath
            );

        data.gpu_usage =
            getAMDUsage(
                card.cardPath
            );

        data.gpu_temperature =
            getAMDTemperature(
                card.cardPath
            );

        data.gpu_frequency =
            getAMDFrequency(
                card.cardPath
            );

        string driverVersion =
            getKernelModuleVersion(
                "amdgpu"
            );

        if (!driverVersion.empty()) {
            data.gpu_driver_version =
                driverVersion;
        }
    }

    // Intel
    else if (card.vendor == "0x8086") {
        data.gpu_vendor = "Intel";

        data.gpu_name =
            getNameViaLspci(
                card.pciAddress
            );

        if (data.gpu_name.empty()) {
            data.gpu_name = "Intel Graphics";
        }

        data.gpu_memory = "Shared";

        data.gpu_frequency =
            getIntelFrequency(
                card.cardPath
            );

        string driverVersion =
            getKernelModuleVersion(
                "i915"
            );

        if (!driverVersion.empty()) {
            data.gpu_driver_version =
                driverVersion;
        }

        // Intel integrated graphics generally use shared system memory.
        data.gpu_core_count = 0;
    }

    // Unknown GPU vendor
    else {
        data.gpu_name =
            getNameViaLspci(
                card.pciAddress
            );

        if (data.gpu_name.empty()) {
            data.gpu_name = "Unknown GPU";
        }

        data.gpu_vendor =
            trim(
                readFile(
                    card.cardPath + "/vendor"
                )
            );
    }

    return data;
}

// Returns information about all GPUs.
vector<gpu_data> GPUInfo::get_all_gpu_info() {
    vector<gpu_data> result;

    vector<DrmCard> cards =
        enumerateCards();

    if (cards.empty()) {
        return result;
    }

    unsigned int nvidiaIndex = 0;

    for (const auto& card : cards) {
        if (card.vendor == "0x10de") {
            result.push_back(
                buildGpuData(
                    card,
                    nvidiaIndex
                )
            );

            ++nvidiaIndex;
        } else {
            result.push_back(
                buildGpuData(
                    card,
                    0
                )
            );
        }
    }

    return result;
}

// Returns usage for the primary GPU.
float GPUInfo::get_gpu_usage() {
    vector<gpu_data> gpus =
        get_all_gpu_info();

    if (gpus.empty()) {
        return -1.0f;
    }

    return gpus[0].gpu_usage;
}

// Returns temperature for the primary GPU.
float GPUInfo::get_gpu_temperature() {
    vector<gpu_data> gpus =
        get_all_gpu_info();

    if (gpus.empty()) {
        return 0.0f;
    }

    return gpus[0].gpu_temperature;
}

// Returns core count for the primary GPU.
int GPUInfo::get_gpu_core_count() {
    vector<gpu_data> gpus =
        get_all_gpu_info();

    if (gpus.empty()) {
        return 0;
    }

    return gpus[0].gpu_core_count;
}