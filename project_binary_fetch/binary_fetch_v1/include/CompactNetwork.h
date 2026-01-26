#pragma once
#include <string>

class CompactNetwork {
public:
    std::string get_network_name();  // Adapter name or WiFi SSID
    std::string get_network_type();  // "WiFi" or "Ethernet"
    std::string get_network_ip();    // Local IPv4 address

private:
    std::string get_wifi_ssid();     // Helper: returns WiFi SSID
    std::string get_ethernet_name(); // Helper: returns Ethernet adapter name
};
