#pragma once
#include <string>

class OSInfo {
public:
    OSInfo() = default;

    std::string GetOSVersion();          // Windows version + build
    std::string GetOSArchitecture();     // 32-bit or 64-bit
    std::string GetOSName();             // Home / Pro / Enterprise
    std::string get_os_install_date();   // installation date of os 
    std::string get_os_serial_number();  // get the serial number of the os
    std::string get_os_uptime();         // get uptime of the os
    std::string get_os_kernel_info();    // get kernel info of the os
};

