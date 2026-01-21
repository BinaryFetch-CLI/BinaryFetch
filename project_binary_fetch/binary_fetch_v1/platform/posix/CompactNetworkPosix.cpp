#include "CompactNetwork.h"
#include "platform/Platform.h"
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <cstring>

#if PLATFORM_POSIX

std::string CompactNetwork::get_wifi_ssid() {
    if (Platform::commandExists("iwgetid")) {
        std::string ssid = Platform::exec("iwgetid -r 2>/dev/null");
        ssid = Platform::trim(ssid);
        if (!ssid.empty()) return ssid;
    }
    
    if (Platform::commandExists("nmcli")) {
        std::string result = Platform::exec("nmcli -t -f active,ssid dev wifi 2>/dev/null | grep '^yes'");
        if (!result.empty()) {
            size_t colonPos = result.find(':');
            if (colonPos != std::string::npos) {
                return Platform::trim(result.substr(colonPos + 1));
            }
        }
    }
    
    return "";
}

std::string CompactNetwork::get_ethernet_name() {
    struct ifaddrs* ifaddr = nullptr;
    if (getifaddrs(&ifaddr) == -1) return "Ethernet";
    
    std::string result = "Ethernet";
    for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;
        if (ifa->ifa_addr->sa_family != AF_INET) continue;
        if (!(ifa->ifa_flags & IFF_UP)) continue;
        if (ifa->ifa_flags & IFF_LOOPBACK) continue;
        
        std::string name = ifa->ifa_name;
        if (name.find("eth") == 0 || name.find("en") == 0) {
            result = name;
            break;
        }
    }
    
    freeifaddrs(ifaddr);
    return result;
}

std::string CompactNetwork::get_network_name() {
    std::string ssid = get_wifi_ssid();
    if (!ssid.empty()) return ssid;
    return get_ethernet_name();
}

std::string CompactNetwork::get_network_type() {
    struct ifaddrs* ifaddr = nullptr;
    if (getifaddrs(&ifaddr) == -1) return "Unknown";
    
    std::string type = "Unknown";
    for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;
        if (ifa->ifa_addr->sa_family != AF_INET) continue;
        if (!(ifa->ifa_flags & IFF_UP)) continue;
        if (ifa->ifa_flags & IFF_LOOPBACK) continue;
        
        std::string name = ifa->ifa_name;
        if (name.find("wl") == 0 || name.find("wlan") == 0) {
            type = "WiFi";
            break;
        }
        if (name.find("eth") == 0 || name.find("en") == 0) {
            type = "Ethernet";
        }
    }
    
    freeifaddrs(ifaddr);
    return type;
}

std::string CompactNetwork::get_network_ip() {
    struct ifaddrs* ifaddr = nullptr;
    if (getifaddrs(&ifaddr) == -1) return "Unknown";
    
    std::string ip = "Unknown";
    for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;
        if (ifa->ifa_addr->sa_family != AF_INET) continue;
        if (!(ifa->ifa_flags & IFF_UP)) continue;
        if (ifa->ifa_flags & IFF_LOOPBACK) continue;
        
        char addrBuf[INET_ADDRSTRLEN];
        struct sockaddr_in* addr = reinterpret_cast<struct sockaddr_in*>(ifa->ifa_addr);
        if (inet_ntop(AF_INET, &addr->sin_addr, addrBuf, sizeof(addrBuf))) {
            ip = addrBuf;
            break;
        }
    }
    
    freeifaddrs(ifaddr);
    return ip;
}

#endif
