#include "CompactSystem.h"
#include <fstream>
#include <sstream>
#include <string>

// Helper to read DMI properties
static std::string readDmiValue(const std::string& field) {
    std::string path = "/sys/class/dmi/id/" + field;
    std::ifstream file(path);
    std::string value;
    if (file && std::getline(file, value)) {
        // Strip trailing/leading spaces
        size_t first = value.find_first_not_of(" \t\r\n");
        size_t last = value.find_last_not_of(" \t\r\n");
        if (first != std::string::npos && last != std::string::npos) {
            return value.substr(first, (last - first + 1));
        }
        return value;
    }
    return "Unknown";
}

// Get BIOS information
std::string CompactSystem::getBIOSInfo() {
    std::string biosVendor = readDmiValue("bios_vendor");
    std::string biosVersion = readDmiValue("bios_version");
    std::string biosDate = readDmiValue("bios_date");

    return biosVendor + " " + biosVersion + " (" + biosDate + ")";
}

// Get Motherboard manufacturer and model name
std::string CompactSystem::getMotherboardInfo() {
    std::string boardVendor = readDmiValue("board_vendor");
    std::string boardName = readDmiValue("board_name");

    return boardVendor + " " + boardName;
}
