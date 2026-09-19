// NetworkInfo.cpp

#include "NetworkInfo.h"

#include <arpa/inet.h>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <ifaddrs.h>
#include <memory>
#include <net/if.h>
#include <netdb.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>
#include <cstdlib>
#include <vector>

using namespace std;
using namespace std::chrono;


// ============================================================
// Helper: Execute shell command and capture output
// ============================================================

static string exec_command(const string& command)
{
    string result;

    FILE* pipe = popen(command.c_str(), "r");

    if (!pipe)
        return result;

    char buffer[256];

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
        result += buffer;
    }

    pclose(pipe);

    // Remove trailing newline
    while (!result.empty() &&
           (result.back() == '\n' || result.back() == '\r'))
    {
        result.pop_back();
    }

    return result;
}


// ============================================================
// Helper: Format network speed
// ============================================================

static string format_speed(double mbps)
{
    ostringstream oss;

    if (mbps >= 1000.0)
    {
        oss << fixed << setprecision(1)
            << (mbps / 1000.0)
            << " Gbps";
    }
    else if (mbps >= 1.0)
    {
        oss << fixed << setprecision(1)
            << mbps
            << " Mbps";
    }
    else
    {
        oss << fixed << setprecision(0)
            << (mbps * 1000.0)
            << " Kbps";
    }

    return oss.str();
}


// ============================================================
// get_local_ip
//
// Returns local IPv4 address with CIDR prefix.
//
// Example:
// 192.168.0.9/24
// ============================================================

string NetworkInfo::get_local_ip()
{
    string result = "Unknown";

    struct ifaddrs* interfaces = nullptr;

    if (getifaddrs(&interfaces) != 0)
        return result;

    for (struct ifaddrs* interface = interfaces;
         interface != nullptr;
         interface = interface->ifa_next)
    {
        if (!interface->ifa_addr)
            continue;

        // Ignore loopback
        if (interface->ifa_flags & IFF_LOOPBACK)
            continue;

        // Only IPv4
        if (interface->ifa_addr->sa_family != AF_INET)
            continue;

        sockaddr_in* addr =
            reinterpret_cast<sockaddr_in*>(interface->ifa_addr);

        char ip[INET_ADDRSTRLEN] = {};

        if (!inet_ntop(AF_INET,
                       &(addr->sin_addr),
                       ip,
                       sizeof(ip)))
        {
            continue;
        }

        // Ignore 0.0.0.0
        if (strcmp(ip, "0.0.0.0") == 0)
            continue;

        // Calculate CIDR from netmask
        int cidr = 0;

        if (interface->ifa_netmask)
        {
            sockaddr_in* netmask =
                reinterpret_cast<sockaddr_in*>(interface->ifa_netmask);

            uint32_t mask = ntohl(netmask->sin_addr.s_addr);

            while (mask)
            {
                cidr += mask & 1;
                mask >>= 1;
            }
        }

        ostringstream oss;

        oss << ip;

        if (cidr > 0)
            oss << "/" << cidr;

        result = oss.str();

        break;
    }

    freeifaddrs(interfaces);

    return result;
}


// ============================================================
// get_mac_address
//
// Returns MAC address of first active non-loopback interface.
//
// Example:
// A4:B1:C1:23:8F:99
// ============================================================

string NetworkInfo::get_mac_address()
{
    string mac = "Unknown";

    struct ifaddrs* interfaces = nullptr;

    if (getifaddrs(&interfaces) != 0)
        return mac;

    for (struct ifaddrs* interface = interfaces;
         interface != nullptr;
         interface = interface->ifa_next)
    {
        if (!interface->ifa_addr)
            continue;

        if (interface->ifa_flags & IFF_LOOPBACK)
            continue;

        if (!(interface->ifa_flags & IFF_UP))
            continue;

        string interface_name = interface->ifa_name;

        // Linux exposes MAC addresses through sysfs.
        string path =
            "/sys/class/net/" +
            interface_name +
            "/address";

        ifstream file(path);

        if (!file)
            continue;

        string address;

        getline(file, address);

        if (address.empty())
            continue;

        // Convert lowercase to uppercase
        transform(
            address.begin(),
            address.end(),
            address.begin(),
            [](unsigned char c)
            {
                return static_cast<char>(toupper(c));
            }
        );

        mac = address;

        break;
    }

    freeifaddrs(interfaces);

    return mac;
}


// ============================================================
// get_locale
//
// Returns system/user locale.
//
// Examples:
// en_US.UTF-8
// en_GB.UTF-8
// bn_BD.UTF-8
// ============================================================

string NetworkInfo::get_locale()
{
    const char* locale = getenv("LC_ALL");

    if (!locale || strlen(locale) == 0)
        locale = getenv("LC_MESSAGES");

    if (!locale || strlen(locale) == 0)
        locale = getenv("LANG");

    if (!locale || strlen(locale) == 0)
        return "Unknown";

    string result(locale);

    return result;
}


// ============================================================
// get_network_name
//
// Attempts to retrieve the connected Wi-Fi/network name.
//
// Uses NetworkManager's nmcli when available.
//
// Example:
// Home_WiFi
// ===========================================================

string NetworkInfo::get_network_name()
{
    // Try Linux Wi-Fi interface first.
    string result =
        exec_command(
            "iwgetid -r 2>/dev/null"
        );

    if (!result.empty())
        return result;

    // Fallback for systems using NetworkManager.
    result =
        exec_command(
            "nmcli -t -f active,ssid dev wifi 2>/dev/null "
            "| grep '^yes:' | head -n 1 | cut -d: -f2-"
        );

    if (!result.empty())
        return result;

    return "Unknown";
}
// ============================================================
// get_public_ip
//
// Uses api.ipify.org.
//
// Example:
// 103.xxx.xxx.xxx
// ============================================================

string NetworkInfo::get_public_ip()
{
    string public_ip = "Unknown";

    string result =
        exec_command(
            "curl -4 -s --max-time 5 "
            "https://api.ipify.org"
        );

    if (!result.empty())
        public_ip = result;

    return public_ip;
}


// ============================================================
// get_network_download_speed
//
// Downloads approximately 1 MB from Cloudflare.
//
// Measures the time required to download the data.
//
// Example:
// 85.3 Mbps
// ============================================================

string NetworkInfo::get_network_download_speed()
{
    string speed_str = "Unknown";

    /*
        Cloudflare test endpoint:

        https://speed.cloudflare.com/__down?bytes=1000000

        We intentionally discard the downloaded data.
    */

    const long long test_size = 1000000;

    string url =
        "https://speed.cloudflare.com/__down?bytes=" +
        to_string(test_size);

    auto start_time =
        high_resolution_clock::now();

    /*
        curl options:

        -4              IPv4
        -s              silent
        --max-time 5    maximum 5 seconds
        -o /dev/null    discard downloaded data
    */

    string command =
        "curl -4 -s "
        "--max-time 5 "
        "-o /dev/null "
        "\"" + url + "\"";

    int exit_code = system(command.c_str());

    auto end_time =
        high_resolution_clock::now();

    if (exit_code != 0)
        return speed_str;

    auto duration =
        duration_cast<milliseconds>(
            end_time - start_time
        ).count();

    if (duration <= 0)
        return speed_str;

    double seconds =
        duration / 1000.0;

    double megabits =
        (test_size * 8.0) / 1000000.0;

    double mbps =
        megabits / seconds;

    speed_str = format_speed(mbps);

    return speed_str;
}


// ============================================================
// get_network_upload_speed
//
// Uploads approximately 500 KB to Cloudflare.
//
// Example:
// 23.5 Mbps
// ============================================================

string NetworkInfo::get_network_upload_speed()
{
    string speed_str = "Unknown";

    const long long test_size = 500000;

    /*
        Create temporary test data.

        /dev/zero is extremely fast and avoids
        generating random data.
    */

    string command =
        "dd if=/dev/zero "
        "bs=500000 count=1 "
        "2>/dev/null "
        "| curl -4 -s "
        "--max-time 5 "
        "-X POST "
        "--data-binary @- "
        "-o /dev/null "
        "https://speed.cloudflare.com/__up";

    auto start_time =
        high_resolution_clock::now();

    int exit_code =
        system(command.c_str());

    auto end_time =
        high_resolution_clock::now();

    if (exit_code != 0)
        return speed_str;

    auto duration =
        duration_cast<milliseconds>(
            end_time - start_time
        ).count();

    if (duration <= 0)
        return speed_str;

    double seconds =
        duration / 1000.0;

    double megabits =
        (test_size * 8.0) / 1000000.0;

    double mbps =
        megabits / seconds;

    speed_str = format_speed(mbps);

    return speed_str;
}