#pragma once
#include <string>
using namespace std;
class CompactNetwork {
public:
    string get_network_name();  // Adapter name or WiFi SSID
    string get_network_type();  // "WiFi" or "Ethernet"
    string get_network_ip();    // Local IPv4 address

private:
    string get_wifi_ssid();     // Helper: returns WiFi SSID
    string get_ethernet_name(); // Helper: returns Ethernet adapter name
};
