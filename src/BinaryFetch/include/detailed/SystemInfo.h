#pragma once
#include <string>
#include <vector>
using namespace std;
class SystemInfo {
public:
    SystemInfo();
    ~SystemInfo();

    string get_bios_vendor();
    string get_bios_version();
    string get_bios_date();
    string get_motherboard_model();
    string get_motherboard_manufacturer();

private:
    string read_registry_value(const std::string& subkey, const std::string& valueName);
};
