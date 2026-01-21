#include "../Platform.h"
#include "../../DetailedGPUInfo.h"
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <cstdio>

using namespace std;
using namespace Platform;

DetailedGPUInfo::DetailedGPUInfo() {}
DetailedGPUInfo::~DetailedGPUInfo() {}

static vector<GPUData> get_nvidia_gpus() {
    vector<GPUData> gpus;
    
    if (!commandExists("nvidia-smi")) {
        return gpus;
    }
    
    string output = exec("nvidia-smi --query-gpu=index,name,memory.total,clocks.gr --format=csv,noheader,nounits 2>/dev/null");
    if (output.empty()) return gpus;
    
    istringstream iss(output);
    string line;
    
    while (getline(iss, line)) {
        if (line.empty()) continue;
        
        vector<string> parts = split(line, ',');
        if (parts.size() < 3) continue;
        
        GPUData gpu;
        
        try {
            gpu.index = stoi(trim(parts[0]));
        } catch (...) {
            gpu.index = static_cast<int>(gpus.size());
        }
        
        gpu.name = trim(parts[1]);
        
        try {
            float vramMiB = stof(trim(parts[2]));
            gpu.vram_gb = vramMiB / 1024.0f;
        } catch (...) {
            gpu.vram_gb = 0.0f;
        }
        
        if (parts.size() >= 4) {
            try {
                float clockMHz = stof(trim(parts[3]));
                gpu.frequency_ghz = clockMHz / 1000.0f;
            } catch (...) {
                gpu.frequency_ghz = 0.0f;
            }
        } else {
            gpu.frequency_ghz = 0.0f;
        }
        
        gpus.push_back(gpu);
    }
    
    return gpus;
}

static vector<GPUData> get_lspci_gpus() {
    vector<GPUData> gpus;
    
    if (!commandExists("lspci")) {
        return gpus;
    }
    
    string output = exec("lspci -nn 2>/dev/null | grep -iE 'VGA|3D|Display' 2>/dev/null");
    if (output.empty()) return gpus;
    
    istringstream iss(output);
    string line;
    int index = 0;
    
    while (getline(iss, line)) {
        if (line.empty()) continue;
        
        GPUData gpu;
        gpu.index = index++;
        
        size_t colonPos = line.find("]: ");
        size_t bracketPos = line.rfind(" [");
        
        if (colonPos != string::npos && bracketPos != string::npos && bracketPos > colonPos) {
            gpu.name = trim(line.substr(colonPos + 3, bracketPos - colonPos - 3));
        } else if (colonPos != string::npos) {
            gpu.name = trim(line.substr(colonPos + 3));
        } else {
            gpu.name = trim(line);
        }
        
        gpu.vram_gb = 0.0f;
        gpu.frequency_ghz = 0.0f;
        
        gpus.push_back(gpu);
    }
    
    return gpus;
}

static void enrich_from_sysfs(vector<GPUData>& gpus) {
    const string drmPath = "/sys/class/drm/";
    
    DIR* dir = opendir(drmPath.c_str());
    if (!dir) return;
    
    struct dirent* entry;
    int cardIndex = 0;
    
    while ((entry = readdir(dir)) != nullptr) {
        string name = entry->d_name;
        
        if (name.find("card") != 0 || name.find('-') != string::npos) continue;
        
        string cardPath = drmPath + name + "/device/";
        
        string vramStr = trim(readFile(cardPath + "mem_info_vram_total"));
        if (!vramStr.empty() && cardIndex < static_cast<int>(gpus.size())) {
            try {
                unsigned long long vramBytes = stoull(vramStr);
                gpus[cardIndex].vram_gb = static_cast<float>(vramBytes) / (1024.0f * 1024.0f * 1024.0f);
            } catch (...) {}
        }
        
        string ppDpmSclk = trim(readFile(cardPath + "pp_dpm_sclk"));
        if (!ppDpmSclk.empty() && cardIndex < static_cast<int>(gpus.size())) {
            istringstream iss(ppDpmSclk);
            string line;
            while (getline(iss, line)) {
                if (line.find('*') != string::npos) {
                    size_t mhzPos = line.find("Mhz");
                    if (mhzPos == string::npos) mhzPos = line.find("MHz");
                    if (mhzPos != string::npos) {
                        size_t colonPos = line.find(':');
                        if (colonPos != string::npos) {
                            string clockStr = trim(line.substr(colonPos + 1, mhzPos - colonPos - 1));
                            try {
                                float clockMHz = stof(clockStr);
                                gpus[cardIndex].frequency_ghz = clockMHz / 1000.0f;
                            } catch (...) {}
                        }
                    }
                    break;
                }
            }
        }
        
        cardIndex++;
    }
    
    closedir(dir);
}

vector<GPUData> DetailedGPUInfo::get_all_gpus() {
    vector<GPUData> gpus = get_nvidia_gpus();
    
    if (gpus.empty()) {
        gpus = get_lspci_gpus();
    }
    
    enrich_from_sysfs(gpus);
    
    if (gpus.empty()) {
        GPUData gpu;
        gpu.index = 0;
        gpu.name = "Unknown GPU";
        gpu.vram_gb = 0.0f;
        gpu.frequency_ghz = 0.0f;
        gpus.push_back(gpu);
    }
    
    return gpus;
}

GPUData DetailedGPUInfo::primary_gpu_info() {
    auto gpus = get_all_gpus();
    if (!gpus.empty()) {
        return gpus[0];
    }
    
    return GPUData{ -1, "No GPU Found", 0.0f, 0.0f };
}
