#include "../Platform.h"
#include "../../SystemInfo.h"
#include <fstream>

static std::string readDMI(const std::string& file) {
    std::string path = "/sys/class/dmi/id/" + file;
    std::string value = Platform::readFileLine(path);
    value = Platform::trim(value);
    
    if (value.empty() || value == "To Be Filled By O.E.M." || value == "Default string" || value == "Not Specified") {
        return "N/A";
    }
    return value;
}

SystemInfo::SystemInfo() {}
SystemInfo::~SystemInfo() {}

std::string SystemInfo::get_bios_vendor() {
    return readDMI("bios_vendor");
}

std::string SystemInfo::get_bios_version() {
    return readDMI("bios_version");
}

std::string SystemInfo::get_bios_date() {
    return readDMI("bios_date");
}

std::string SystemInfo::get_motherboard_manufacturer() {
    return readDMI("board_vendor");
}

std::string SystemInfo::get_motherboard_model() {
    return readDMI("board_name");
}

std::string SystemInfo::read_registry_value(const std::string& /*subkey*/, const std::string& /*valueName*/) {
    return "";
}
