#include "../Platform.h"
#include "../../SystemInfo.h"
#include <fstream>

static std::string readKenv(const std::string& key) {
    std::string cmd = "kenv " + key + " 2>/dev/null";
    std::string value = Platform::exec(cmd);
    value = Platform::trim(value);
    
    if (value.empty() || value == "To Be Filled By O.E.M." || 
        value == "Default string" || value == "Not Specified" ||
        value == "None" || value.find("unknown") != std::string::npos) {
        return "N/A";
    }
    return value;
}

std::string SystemInfo::get_bios_vendor() {
    return readKenv("smbios.bios.vendor");
}

std::string SystemInfo::get_bios_version() {
    return readKenv("smbios.bios.version");
}

std::string SystemInfo::get_bios_release_date() {
    return readKenv("smbios.bios.reldate");
}

std::string SystemInfo::get_motherboard_manufacturer() {
    return readKenv("smbios.planar.maker");
}

std::string SystemInfo::get_motherboard_model() {
    return readKenv("smbios.planar.product");
}

std::vector<std::pair<std::string, std::pair<std::string, std::string>>> SystemInfo::get_bios_info() {
    std::vector<std::pair<std::string, std::pair<std::string, std::string>>> info;
    
    info.push_back({"BIOS Vendor", {"", get_bios_vendor()}});
    info.push_back({"BIOS Version", {"", get_bios_version()}});
    info.push_back({"BIOS Date", {"", get_bios_release_date()}});
    info.push_back({"Board Manufacturer", {"", get_motherboard_manufacturer()}});
    info.push_back({"Board Model", {"", get_motherboard_model()}});
    
    std::string product = readKenv("smbios.system.product");
    if (product != "N/A") {
        info.push_back({"System Product", {"", product}});
    }
    
    std::string sys_vendor = readKenv("smbios.system.maker");
    if (sys_vendor != "N/A") {
        info.push_back({"System Vendor", {"", sys_vendor}});
    }
    
    return info;
}
