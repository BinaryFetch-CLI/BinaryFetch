#pragma once
#include <string>

class CompactSystem {
public:
    std::string getBIOSInfo();        // BIOS Name + Version
    std::string getMotherboardInfo(); // Motherboard Model
};
