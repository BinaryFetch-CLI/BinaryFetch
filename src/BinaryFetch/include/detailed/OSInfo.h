#pragma once
#include <string>
using namespace std;
class OSInfo {
public:
    OSInfo() = default;

    string GetOSVersion();          // Windows version + build
    string GetOSArchitecture();     // 32-bit or 64-bit
    string GetOSName();             // Home / Pro / Enterprise
    string get_os_install_date();   // installation date of os 
    string get_os_serial_number();  // get the serial number of the os
    string get_os_uptime();         // get uptime of the os
    string get_os_kernel_info();    // get kernel info of the os
};

