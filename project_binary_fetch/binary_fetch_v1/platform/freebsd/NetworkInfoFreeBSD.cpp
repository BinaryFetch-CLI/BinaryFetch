#include "../Platform.h"
#include "../HttpClient.h"
#include "../../NetworkInfo.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>

static std::string getPrimaryInterface() {
    struct ifaddrs* ifaddr;
    if (getifaddrs(&ifaddr) == -1) return "";
    
    std::string primary;
    for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;
        if (ifa->ifa_addr->sa_family != AF_INET) continue;
        
        std::string name = ifa->ifa_name;
        if (name == "lo0") continue;
        if (name.find("bridge") == 0) continue;
        if (name.find("tap") == 0) continue;
        if (name.find("tun") == 0) continue;
        
        if (ifa->ifa_flags & IFF_UP) {
            primary = name;
            break;
        }
    }
    
    freeifaddrs(ifaddr);
    return primary;
}

std::string NetworkInfo::get_local_ip() {
    struct ifaddrs* ifaddr;
    if (getifaddrs(&ifaddr) == -1) return "Unknown";
    
    std::string result = "Unknown";
    
    for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;
        if (ifa->ifa_addr->sa_family != AF_INET) continue;
        
        std::string name = ifa->ifa_name;
        if (name == "lo0") continue;
        
        if (!(ifa->ifa_flags & IFF_UP)) continue;
        
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr, ip, sizeof(ip));
        
        int prefix = 24;
        if (ifa->ifa_netmask) {
            uint32_t mask = ntohl(((struct sockaddr_in*)ifa->ifa_netmask)->sin_addr.s_addr);
            prefix = 0;
            while (mask) {
                prefix += (mask & 1);
                mask >>= 1;
            }
        }
        
        std::ostringstream oss;
        oss << ip << "/" << prefix;
        result = oss.str();
        break;
    }
    
    freeifaddrs(ifaddr);
    return result;
}

std::string NetworkInfo::get_mac_address() {
    struct ifaddrs* ifaddr;
    if (getifaddrs(&ifaddr) == -1) return "Unknown";
    
    std::string primary = getPrimaryInterface();
    std::string result = "Unknown";
    
    for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;
        if (ifa->ifa_addr->sa_family != AF_LINK) continue;
        
        std::string name = ifa->ifa_name;
        if (!primary.empty() && name != primary) continue;
        if (name == "lo0") continue;
        
        struct sockaddr_dl* sdl = (struct sockaddr_dl*)ifa->ifa_addr;
        if (sdl->sdl_alen == 6) {
            unsigned char* mac = (unsigned char*)LLADDR(sdl);
            char buf[18];
            snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
                     mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            result = buf;
            break;
        }
    }
    
    freeifaddrs(ifaddr);
    return result;
}

std::string NetworkInfo::get_locale() {
    std::string locale = Platform::getEnv("LANG");
    if (!locale.empty()) {
        size_t dot = locale.find('.');
        if (dot != std::string::npos) {
            locale = locale.substr(0, dot);
        }
        std::replace(locale.begin(), locale.end(), '_', '-');
        return locale;
    }
    
    return "en-US";
}

std::string NetworkInfo::get_network_name() {
    if (Platform::commandExists("ifconfig")) {
        std::string result = Platform::exec("ifconfig wlan0 2>/dev/null | grep 'ssid' | awk '{print $2}'");
        result = Platform::trim(result);
        if (!result.empty()) return result;
    }
    
    std::string iface = getPrimaryInterface();
    return iface.empty() ? "Unknown" : iface;
}

std::string NetworkInfo::get_public_ip() {
    Platform::HttpClient::Response resp = Platform::HttpClient::get("api.ipify.org", "/", 80, 5000);
    if (resp.success) {
        return Platform::trim(resp.body);
    }
    
    resp = Platform::HttpClient::get("ifconfig.me", "/ip", 80, 5000);
    if (resp.success) {
        return Platform::trim(resp.body);
    }
    
    return "Unknown";
}

std::string NetworkInfo::get_network_download_speed() {
    return Platform::HttpClient::downloadSpeed("speed.cloudflare.com", "/__down", 1000000, 5000);
}

std::string NetworkInfo::get_network_upload_speed() {
    return Platform::HttpClient::uploadSpeed("speed.cloudflare.com", "/__up", 500000, 5000);
}
