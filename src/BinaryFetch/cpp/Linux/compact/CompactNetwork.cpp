#include "CompactNetwork.h"
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <cstring>

// Helper to find the active network interface and return its name and IP
static bool getActiveInterface(std::string& outInterfaceName, std::string& outIP) {
    struct ifaddrs* ifaddr = nullptr;
    if (getifaddrs(&ifaddr) == -1) {
        return false;
    }

    bool found = false;
    // Iterate through list of network interfaces
    for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;

        // We only care about IPv4 addresses
        if (ifa->ifa_addr->sa_family == AF_INET) {
            // Must be UP, RUNNING and not Loopback
            if ((ifa->ifa_flags & IFF_UP) && 
                (ifa->ifa_flags & IFF_RUNNING) && 
                !(ifa->ifa_flags & IFF_LOOPBACK)) {
                
                char ipStr[INET_ADDRSTRLEN];
                struct sockaddr_in* ipv4 = reinterpret_cast<struct sockaddr_in*>(ifa->ifa_addr);
                if (inet_ntop(AF_INET, &(ipv4->sin_addr), ipStr, INET_ADDRSTRLEN)) {
                    outInterfaceName = ifa->ifa_name;
                    outIP = ipStr;
                    found = true;
                    break;
                }
            }
        }
    }

    freeifaddrs(ifaddr);
    return found;
}

// Get the network name: either WiFi SSID or Ethernet adapter name
std::string CompactNetwork::get_network_name() {
    std::string ssid = get_wifi_ssid();
    if (!ssid.empty()) return ssid;

    std::string eth = get_ethernet_name();
    if (!eth.empty()) return eth;

    return "Unknown";
}

// Determine network type ("WiFi" or "Ethernet")
std::string CompactNetwork::get_network_type() {
    std::string interfaceName, ip;
    if (getActiveInterface(interfaceName, ip)) {
        if (interfaceName.rfind("wl", 0) == 0) {
            return "WiFi";
        }
    }
    return "Ethernet";
}

// Get local IPv4 address
std::string CompactNetwork::get_network_ip() {
    std::string interfaceName, ip;
    if (getActiveInterface(interfaceName, ip)) {
        return ip;
    }
    return "Unknown";
}

// Helper to query Wi-Fi SSID
std::string CompactNetwork::get_wifi_ssid() {
    std::string interfaceName, ip;
    if (getActiveInterface(interfaceName, ip)) {
        if (interfaceName.rfind("wl", 0) == 0) {
            // Check /proc/net/wireless first to see if active
            std::ifstream wfile("/proc/net/wireless");
            if (wfile.is_open()) {
                std::string line;
                while (std::getline(wfile, line)) {
                    if (line.find(interfaceName) != std::string::npos) {
                        // Attempt to extract SSID via iwgetid (if available) or return interface name
                        // Running shell command is a backup, let's keep it safe.
                        FILE* pipe = popen(("iwgetid " + interfaceName + " -r 2>/dev/null").c_str(), "r");
                        if (pipe) {
                            char buffer[128];
                            std::string result = "";
                            if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                                result = buffer;
                                // Remove trailing newline
                                if (!result.empty() && result.back() == '\n') result.pop_back();
                                pclose(pipe);
                                return result;
                            }
                            pclose(pipe);
                        }
                        return interfaceName; // Fallback to interface name
                    }
                }
            }
        }
    }
    return "";
}

// Helper to query active Ethernet adapter name
std::string CompactNetwork::get_ethernet_name() {
    std::string interfaceName, ip;
    if (getActiveInterface(interfaceName, ip)) {
        if (interfaceName.rfind("wl", 0) != 0) {
            return interfaceName; // e.g. eth0, enp3s0
        }
    }
    return "";
}
