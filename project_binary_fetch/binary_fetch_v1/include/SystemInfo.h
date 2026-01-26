#pragma once
#include <string>
#include <vector>

class SystemInfo {
public:
    SystemInfo();
    ~SystemInfo();

    std::string get_bios_vendor();
    std::string get_bios_version();
    std::string get_bios_date();
    std::string get_motherboard_model();
    std::string get_motherboard_manufacturer();

private:
    std::string read_registry_value(const std::string& subkey, const std::string& valueName);
};
