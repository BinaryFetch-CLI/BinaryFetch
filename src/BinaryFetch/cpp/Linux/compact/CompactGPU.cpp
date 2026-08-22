#include "CompactGPU.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <glob.h>
#include <dlfcn.h>
#include <unistd.h>     // readlink()
#include <dirent.h>
#include <nvml.h>


// These are the NVML functions BinaryFetch needs.
// NVML is loaded at runtime so the program can still run on
// systems that do not have an NVIDIA driver installed.
using nvmlInit_t = nvmlReturn_t (*)();
using nvmlShutdown_t = nvmlReturn_t (*)();

using nvmlDeviceGetHandleByIndex_t =
    nvmlReturn_t (*)(unsigned int, nvmlDevice_t*);

using nvmlDeviceGetName_t =
    nvmlReturn_t (*)(nvmlDevice_t, char*, unsigned int);

using nvmlDeviceGetMemoryInfo_t =
    nvmlReturn_t (*)(nvmlDevice_t, nvmlMemory_t*);

using nvmlDeviceGetUtilizationRates_t =
    nvmlReturn_t (*)(nvmlDevice_t, nvmlUtilization_t*);

using nvmlDeviceGetClockInfo_t =
    nvmlReturn_t (*)(nvmlDevice_t, nvmlClockType_t, unsigned int*);

using nvmlDeviceGetTemperature_t =
    nvmlReturn_t (*)(nvmlDevice_t, unsigned int, unsigned int*);

using nvmlDeviceGetPciInfo_t =
    nvmlReturn_t (*)(nvmlDevice_t, nvmlPciInfo_t*);


// Keeps all dynamically loaded NVIDIA functions together.
struct NVMLFunctions {

    void* handle = nullptr;

    nvmlInit_t init = nullptr;
    nvmlShutdown_t shutdown = nullptr;

    nvmlDeviceGetHandleByIndex_t getHandle = nullptr;
    nvmlDeviceGetName_t getName = nullptr;
    nvmlDeviceGetMemoryInfo_t getMemoryInfo = nullptr;
    nvmlDeviceGetUtilizationRates_t getUtilizationRates = nullptr;
    nvmlDeviceGetClockInfo_t getClockInfo = nullptr;
    nvmlDeviceGetTemperature_t getTemperature = nullptr;
    nvmlDeviceGetPciInfo_t getPciInfo = nullptr;


    // Loads NVML and resolves the functions we need.
    bool load() {

        if (handle)
            return true;

        handle = dlopen(
            "libnvidia-ml.so.1",
            RTLD_LAZY
        );

        if (!handle) {

            handle = dlopen(
                "libnvidia-ml.so",
                RTLD_LAZY
            );
        }

        if (!handle)
            return false;


        init = reinterpret_cast<nvmlInit_t>(
            dlsym(handle, "nvmlInit_v2")
        );

        if (!init) {

            init = reinterpret_cast<nvmlInit_t>(
                dlsym(handle, "nvmlInit")
            );
        }


        shutdown = reinterpret_cast<nvmlShutdown_t>(
            dlsym(handle, "nvmlShutdown")
        );


        getHandle =
            reinterpret_cast<nvmlDeviceGetHandleByIndex_t>(
                dlsym(
                    handle,
                    "nvmlDeviceGetHandleByIndex_v2"
                )
            );

        if (!getHandle) {

            getHandle =
                reinterpret_cast<nvmlDeviceGetHandleByIndex_t>(
                    dlsym(
                        handle,
                        "nvmlDeviceGetHandleByIndex"
                    )
                );
        }


        getName =
            reinterpret_cast<nvmlDeviceGetName_t>(
                dlsym(
                    handle,
                    "nvmlDeviceGetName"
                )
            );


        getMemoryInfo =
            reinterpret_cast<nvmlDeviceGetMemoryInfo_t>(
                dlsym(
                    handle,
                    "nvmlDeviceGetMemoryInfo"
                )
            );


        getUtilizationRates =
            reinterpret_cast<nvmlDeviceGetUtilizationRates_t>(
                dlsym(
                    handle,
                    "nvmlDeviceGetUtilizationRates"
                )
            );


        getClockInfo =
            reinterpret_cast<nvmlDeviceGetClockInfo_t>(
                dlsym(
                    handle,
                    "nvmlDeviceGetClockInfo"
                )
            );


        getTemperature =
            reinterpret_cast<nvmlDeviceGetTemperature_t>(
                dlsym(
                    handle,
                    "nvmlDeviceGetTemperature"
                )
            );


        getPciInfo =
            reinterpret_cast<nvmlDeviceGetPciInfo_t>(
                dlsym(
                    handle,
                    "nvmlDeviceGetPciInfo_v3"
                )
            );

        if (!getPciInfo) {

            getPciInfo =
                reinterpret_cast<nvmlDeviceGetPciInfo_t>(
                    dlsym(
                        handle,
                        "nvmlDeviceGetPciInfo_v2"
                    )
                );
        }

        if (!getPciInfo) {

            getPciInfo =
                reinterpret_cast<nvmlDeviceGetPciInfo_t>(
                    dlsym(
                        handle,
                        "nvmlDeviceGetPciInfo"
                    )
                );
        }


        // These three functions are enough to identify and use
        // an NVIDIA GPU. The other functions are optional.
        if (!init || !shutdown || !getHandle) {

            dlclose(handle);
            handle = nullptr;

            return false;
        }

        return true;
    }


    ~NVMLFunctions() {

        if (handle)
            dlclose(handle);
    }
};


static NVMLFunctions g_nvml;


// Represents the GPU that Linux is currently using for a display.
struct ActiveGPU {

    std::string cardPath;
    std::string vendor;
    std::string device;
    std::string driver;

    bool valid = false;
};


// Reads a single value from sysfs.
static std::string readFile(
    const std::string& path
) {

    std::ifstream file(path);

    std::string value;

    if (file && std::getline(file, value))
        return value;

    return "";
}


// Linux gives each GPU a DRM card such as card1, card2, etc.
// We inspect the connectors belonging to each card and look for
// one that is actually connected to a display.
static ActiveGPU findActiveDisplayGPU() {

    ActiveGPU result;

    glob_t connectors{};

    if (glob(
            "/sys/class/drm/card*-*/status",
            0,
            nullptr,
            &connectors
        ) != 0) {

        return result;
    }


    for (size_t i = 0; i < connectors.gl_pathc; ++i) {

        const std::string statusPath =
            connectors.gl_pathv[i];

        const std::string status =
            readFile(statusPath);

        if (status != "connected")
            continue;


        // Example:
        //
        // /sys/class/drm/card2-HDMI-A-2/status
        //
        // We need:
        //
        // /sys/class/drm/card2/device
        //
        const std::string marker = "/status";

        std::string connectorPath = statusPath;

        if (connectorPath.size() <= marker.size())
            continue;

        connectorPath.erase(
            connectorPath.size() - marker.size()
        );


        const size_t slash =
            connectorPath.find_last_of('/');

        if (slash == std::string::npos)
            continue;


        const std::string cardConnector =
            connectorPath.substr(
                slash + 1
            );


        const size_t dash =
            cardConnector.find('-');

        if (dash == std::string::npos)
            continue;


        const std::string cardName =
            cardConnector.substr(
                0,
                dash
            );


        const std::string cardPath =
            "/sys/class/drm/" +
            cardName +
            "/device";


        const std::string vendor =
            readFile(
                cardPath + "/vendor"
            );


        if (vendor.empty())
            continue;


        result.cardPath = cardPath;
        result.vendor = vendor;
        result.device = readFile(
            cardPath + "/device"
        );


        const std::string driverLink =
            cardPath + "/driver";


        const std::string driverPath =
            [](
                const std::string& path
            ) -> std::string {

                char buffer[4096];

                const ssize_t length =
                    readlink(
                        path.c_str(),
                        buffer,
                        sizeof(buffer) - 1
                    );

                if (length <= 0)
                    return "";

                buffer[length] = '\0';

                std::string fullPath(buffer);

                const size_t slash =
                    fullPath.find_last_of('/');

                if (slash == std::string::npos)
                    return fullPath;

                return fullPath.substr(
                    slash + 1
                );

            }(driverLink);


        result.driver = driverPath;
        result.valid = true;

        break;
    }


    globfree(&connectors);

    return result;
}


// Finds the NVIDIA NVML device that corresponds to the PCI
// address of the DRM card we discovered above.
static bool getMatchingNvidiaDevice(
    const ActiveGPU& gpu,
    nvmlDevice_t& device
) {

    if (!g_nvml.load())
        return false;

    if (g_nvml.init() != NVML_SUCCESS)
        return false;


    // If PCI matching is unavailable, the system can still use
    // NVIDIA device 0 as a fallback.
    if (!g_nvml.getPciInfo) {

        if (g_nvml.getHandle(
                0,
                &device
            ) == NVML_SUCCESS) {

            return true;
        }

        g_nvml.shutdown();

        return false;
    }


    // The Linux DRM path contains the PCI address, for example:
    //
    // 0000:01:00.0
    //
    // NVML exposes the same address through nvmlPciInfo_t.
    std::string drmRealPath;

    char buffer[4096];

    const std::string deviceLink =
        gpu.cardPath;


    const ssize_t length =
        readlink(
            deviceLink.c_str(),
            buffer,
            sizeof(buffer) - 1
        );


    if (length > 0) {

        buffer[length] = '\0';

        drmRealPath = buffer;
    }


    // Extract the PCI address from the DRM device path.
    std::string pciAddress;

    const std::string pciMarker =
        "/0000:";


    const size_t pciStart =
        drmRealPath.find(pciMarker);


    if (pciStart != std::string::npos) {

        const size_t start =
            pciStart + 1;

        const size_t end =
            drmRealPath.find(
                '/',
                start
            );

        if (end != std::string::npos) {

            pciAddress =
                drmRealPath.substr(
                    start,
                    end - start
                );
        }
    }


    unsigned int count = 0;

    // If we cannot determine the PCI address, use the first
    // NVIDIA device rather than failing completely.
    if (pciAddress.empty()) {

        if (g_nvml.getHandle(
                0,
                &device
            ) == NVML_SUCCESS) {

            return true;
        }

        g_nvml.shutdown();

        return false;
    }


    // Search every NVIDIA device until its PCI address matches
    // the DRM GPU that owns the active display.
    for (unsigned int index = 0;
         index < 32;
         ++index) {

        nvmlDevice_t candidate{};

        if (g_nvml.getHandle(
                index,
                &candidate
            ) != NVML_SUCCESS) {

            break;
        }


        nvmlPciInfo_t pciInfo{};

        if (g_nvml.getPciInfo(
                candidate,
                &pciInfo
            ) != NVML_SUCCESS) {

            continue;
        }


        std::string nvmlBusId =
            pciInfo.busId;


        if (nvmlBusId == pciAddress) {

            device = candidate;

            return true;
        }


        // Some NVML versions format the address differently.
        if (nvmlBusId.find(pciAddress) !=
            std::string::npos) {

            device = candidate;

            return true;
        }


        ++count;
    }


    // The display GPU was NVIDIA, but PCI matching failed.
    // Device 0 is used only as a final fallback.
    if (g_nvml.getHandle(
            0,
            &device
        ) == NVML_SUCCESS) {

        return true;
    }


    g_nvml.shutdown();

    return false;
}


// Returns the GPU name of the display-driving GPU.
std::string CompactGPU::getGPUName() {

    const ActiveGPU gpu =
        findActiveDisplayGPU();


    if (!gpu.valid)
        return "Unknown GPU";


    if (gpu.vendor.find("0x10de") !=
        std::string::npos) {

        nvmlDevice_t device{};

        if (getMatchingNvidiaDevice(
                gpu,
                device
            )) {

            if (g_nvml.getName) {

                char name[128]{};

                if (g_nvml.getName(
                        device,
                        name,
                        sizeof(name)
                    ) == NVML_SUCCESS) {

                    g_nvml.shutdown();

                    return std::string(name);
                }
            }

            g_nvml.shutdown();
        }


        return "NVIDIA GPU";
    }


    if (gpu.vendor.find("0x1002") !=
        std::string::npos) {

        return "AMD Graphics";
    }


    if (gpu.vendor.find("0x8086") !=
        std::string::npos) {

        return "Intel Graphics";
    }


    return "Unknown GPU";
}


// Gets VRAM from the GPU that actually owns the display.
double CompactGPU::getVRAMGB() {

    const ActiveGPU gpu =
        findActiveDisplayGPU();


    if (!gpu.valid)
        return 0.0;


    if (gpu.vendor.find("0x10de") !=
        std::string::npos) {

        nvmlDevice_t device{};

        if (getMatchingNvidiaDevice(
                gpu,
                device
            )) {

            if (g_nvml.getMemoryInfo) {

                nvmlMemory_t memory{};

                if (g_nvml.getMemoryInfo(
                        device,
                        &memory
                    ) == NVML_SUCCESS) {

                    g_nvml.shutdown();

                    return static_cast<double>(
                        memory.total
                    ) /
                    (1024.0 * 1024.0 * 1024.0);
                }
            }

            g_nvml.shutdown();
        }
    }


    // AMD exposes dedicated VRAM through this file.
    const std::string vramPath =
        gpu.cardPath +
        "/mem_info_vram_total";


    std::ifstream file(vramPath);

    double bytes = 0;

    if (file && file >> bytes) {

        return bytes /
               (1024.0 * 1024.0 * 1024.0);
    }


    return 0.0;
}


// Gets GPU utilization from the display-driving GPU.
int CompactGPU::getGPUUsagePercent() {

    const ActiveGPU gpu =
        findActiveDisplayGPU();


    if (!gpu.valid)
        return -1;


    if (gpu.vendor.find("0x10de") !=
        std::string::npos) {

        nvmlDevice_t device{};

        if (getMatchingNvidiaDevice(
                gpu,
                device
            )) {

            if (g_nvml.getUtilizationRates) {

                nvmlUtilization_t utilization{};

                if (g_nvml.getUtilizationRates(
                        device,
                        &utilization
                    ) == NVML_SUCCESS) {

                    g_nvml.shutdown();

                    return static_cast<int>(
                        utilization.gpu
                    );
                }
            }

            g_nvml.shutdown();
        }
    }


    const std::string usagePath =
        gpu.cardPath +
        "/gpu_busy_percent";


    std::ifstream file(usagePath);

    int usage = 0;

    if (file && file >> usage)
        return usage;


    return -1;
}


// Gets the current graphics clock of the display GPU.
std::string CompactGPU::getGPUFrequency() {

    const ActiveGPU gpu =
        findActiveDisplayGPU();


    if (!gpu.valid)
        return "Unknown MHz";


    if (gpu.vendor.find("0x10de") !=
        std::string::npos) {

        nvmlDevice_t device{};

        if (getMatchingNvidiaDevice(
                gpu,
                device
            )) {

            if (g_nvml.getClockInfo) {

                unsigned int clockMHz = 0;

                if (g_nvml.getClockInfo(
                        device,
                        NVML_CLOCK_GRAPHICS,
                        &clockMHz
                    ) == NVML_SUCCESS) {

                    g_nvml.shutdown();

                    return std::to_string(
                        clockMHz
                    ) + " MHz";
                }
            }

            g_nvml.shutdown();
        }
    }


    // AMD exposes the current graphics clock through pp_dpm_sclk.
    const std::string amdClockPath =
        gpu.cardPath +
        "/pp_dpm_sclk";


    std::ifstream amdFile(amdClockPath);

    if (amdFile) {

        std::string line;
        std::string lastLine;

        while (std::getline(
            amdFile,
            line
        )) {

            if (line.empty())
                continue;

            if (line.find('*') !=
                std::string::npos) {

                return line;
            }

            lastLine = line;
        }

        if (!lastLine.empty())
            return lastLine;
    }


    // Intel exposes its current frequency through sysfs.
    const std::string intelClockPath =
        gpu.cardPath +
        "/gt_cur_freq_mhz";


    std::ifstream intelFile(
        intelClockPath
    );

    int frequency = 0;

    if (intelFile &&
        intelFile >> frequency) {

        return std::to_string(
            frequency
        ) + " MHz";
    }


    return "Unknown MHz";
}


// Gets the temperature from the display-driving GPU.
double CompactGPU::getGPUTemperature() {

    const ActiveGPU gpu =
        findActiveDisplayGPU();


    if (!gpu.valid)
        return 0.0;


    if (gpu.vendor.find("0x10de") !=
        std::string::npos) {

        nvmlDevice_t device{};

        if (getMatchingNvidiaDevice(
                gpu,
                device
            )) {

            if (g_nvml.getTemperature) {

                unsigned int temperature = 0;

                if (g_nvml.getTemperature(
                        device,
                        NVML_TEMPERATURE_GPU,
                        &temperature
                    ) == NVML_SUCCESS) {

                    g_nvml.shutdown();

                    return static_cast<double>(
                        temperature
                    );
                }
            }

            g_nvml.shutdown();
        }
    }


    // AMD and other Linux GPU drivers usually expose temperature
    // through their hwmon entries.
    const std::string hwmonPattern =
        gpu.cardPath +
        "/hwmon/hwmon*/temp*_input";


    glob_t files{};

    if (glob(
            hwmonPattern.c_str(),
            0,
            nullptr,
            &files
        ) == 0) {

        for (size_t i = 0;
             i < files.gl_pathc;
             ++i) {

            std::ifstream file(
                files.gl_pathv[i]
            );

            double millidegrees = 0;

            if (file &&
                file >> millidegrees) {

                globfree(&files);

                return millidegrees / 1000.0;
            }
        }
    }

    globfree(&files);


    return 0.0;
}