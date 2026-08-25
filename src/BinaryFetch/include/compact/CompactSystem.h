#pragma once
#include <string>
using namespace std;
class CompactSystem {
public:
    string getBIOSInfo();        // BIOS Name + Version
    string getMotherboardInfo(); // Motherboard Model
};
