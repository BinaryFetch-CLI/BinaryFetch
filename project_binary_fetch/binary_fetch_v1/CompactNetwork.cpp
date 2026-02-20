#include "include\CompactNetwork.h"  
// Custom project header file.
// Likely contains declarations for the CompactNetwork class,
// function prototypes, constants, and internal networking logic.

#include <string>  
// Provides std::string for safe and flexible text handling.
// Useful for storing IP addresses, SSIDs, adapter names, etc.

#include <vector>  
// Provides std::vector (dynamic array container).
// Useful for storing collections like adapter lists,
// available Wi-Fi networks, or socket results.

#include <winsock2.h>  
// Core Windows Sockets (Winsock) API.
// Enables TCP/UDP networking using socket(), bind(), connect(),
// send(), recv(), closesocket(), etc.
// Foundation for low-level network communication.

#include <ws2tcpip.h>  
// Winsock extensions for modern networking.
// Adds support for IPv6 and advanced utilities like:
// - getaddrinfo()
// - inet_pton()
// - inet_ntop()
// Provides protocol-independent address handling.

#include <iphlpapi.h>  
// IP Helper API (Windows).
// Used to retrieve detailed network adapter information such as:
// - IP addresses
// - MAC addresses
// - Gateway info
// - Network statistics
// Common functions: GetAdaptersInfo(), GetAdaptersAddresses().

#include <wlanapi.h>  
// Windows WLAN (Wi-Fi) API.
// Used for wireless-specific operations:
// - Scanning available Wi-Fi networks
// - Retrieving SSID and signal strength
// - Managing wireless profiles
// - Connecting/disconnecting Wi-Fi networks

#include <windows.h>  
// Core Windows operating system API.
// Provides access to system-level features such as:
// - Handles
// - Threads
// - Processes
// - Memory management
// - System services
// Required for many low-level Windows functions.

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "wlanapi.lib")
#pragma comment(lib, "ole32.lib")

// ------------------- Public Functions -------------------

// Returns the current network name.
// First tries WiFi SSID.
// If not connected to WiFi, tries Ethernet adapter name.
// If nothing is found, returns "Unknown".
std::string CompactNetwork::get_network_name() {
    std::string ssidName = get_wifi_ssid();  // Try getting WiFi network name
    if (!ssidName.empty()) return ssidName;  // If WiFi exists, return it

    std::string adapterName = get_ethernet_name();  // Otherwise check Ethernet
    return adapterName.empty() ? "Unknown" : adapterName;  // Fallback safety
}


// Determines whether the system is using WiFi or Ethernet.
// If WiFi SSID exists ? return "WiFi"
// Otherwise ? assume Ethernet.
std::string CompactNetwork::get_network_type() {
    return !get_wifi_ssid().empty() ? "WiFi" : "Ethernet";
}


// Retrieves the system's IPv4 address.
// Uses Winsock to resolve hostname into IP address.
std::string CompactNetwork::get_network_ip() {

    // Step 1: Initialize Winsock (required before using socket functions)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return "Unknown";  // If initialization fails

    // Step 2: Get local machine hostname
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        WSACleanup();  // Always cleanup before returning
        return "Unknown";
    }

    // Step 3: Prepare address lookup settings
    addrinfo hints = {}, * res = nullptr;
    hints.ai_family = AF_INET; // Restrict to IPv4 only

    // Step 4: Convert hostname into IP address info
    if (getaddrinfo(hostname, nullptr, &hints, &res) != 0) {
        WSACleanup();
        return "Unknown";
    }

    std::string ip = "Unknown";

    // Step 5: Loop through returned addresses (if multiple exist)
    for (addrinfo* p = res; p != nullptr; p = p->ai_next) {

        // Convert generic address structure into IPv4 structure
        sockaddr_in* ipv4 = reinterpret_cast<sockaddr_in*>(p->ai_addr);

        char ipStr[INET_ADDRSTRLEN];

        // Convert binary IP into readable string format (e.g., "192.168.0.1")
        if (InetNtopA(AF_INET, &ipv4->sin_addr, ipStr, INET_ADDRSTRLEN)) {
            ip = ipStr;
            break;  // Stop after first valid IP
        }
    }

    // Step 6: Free allocated memory
    freeaddrinfo(res);

    // Step 7: Shutdown Winsock properly
    WSACleanup();

    return ip;
}



// ------------------- Private Helpers -------------------


// Retrieves the currently connected WiFi SSID.
// Uses Windows WLAN API.
std::string CompactNetwork::get_wifi_ssid() {

    HANDLE hClient = NULL;
    DWORD dwMaxClient = 2;        // WLAN API version
    DWORD dwCurVersion = 0;

    // Step 1: Open connection to WLAN service
    if (WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient) != ERROR_SUCCESS)
        return "";

    PWLAN_INTERFACE_INFO_LIST pIfList = NULL;

    // Step 2: Get list of wireless interfaces (WiFi adapters)
    if (WlanEnumInterfaces(hClient, NULL, &pIfList) != ERROR_SUCCESS) {
        WlanCloseHandle(hClient, NULL);
        return "";
    }

    std::string ssid = "";

    // Step 3: Loop through all WiFi interfaces
    for (unsigned int i = 0; i < pIfList->dwNumberOfItems; ++i) {

        PWLAN_INTERFACE_INFO pIfInfo = &pIfList->InterfaceInfo[i];
        PWLAN_CONNECTION_ATTRIBUTES pConnectInfo = NULL;

        DWORD connectSize = sizeof(WLAN_CONNECTION_ATTRIBUTES);
        WLAN_OPCODE_VALUE_TYPE opCode;

        // Step 4: Query current connection info for this interface
        if (WlanQueryInterface(
            hClient,
            &pIfInfo->InterfaceGuid,
            wlan_intf_opcode_current_connection,
            NULL,
            &connectSize,
            (PVOID*)&pConnectInfo,
            &opCode) == ERROR_SUCCESS) {

            // Step 5: Check if interface is connected
            if (pConnectInfo->isState == wlan_interface_state_connected) {

                // Extract SSID from connection attributes
                ssid = std::string(
                    (char*)pConnectInfo->wlanAssociationAttributes.dot11Ssid.ucSSID,
                    pConnectInfo->wlanAssociationAttributes.dot11Ssid.uSSIDLength
                );

                WlanFreeMemory(pConnectInfo);
                break;  // Stop once connected WiFi is found
            }

            WlanFreeMemory(pConnectInfo);
        }
    }

    // Step 6: Clean up allocated resources
    WlanFreeMemory(pIfList);
    WlanCloseHandle(hClient, NULL);

    return ssid;
}



// Retrieves the Ethernet adapter name.
// Uses Windows IP Helper API.
std::string CompactNetwork::get_ethernet_name() {

    ULONG size = 0;

    // Step 1: First call to determine required buffer size
    if (GetAdaptersInfo(nullptr, &size) != ERROR_BUFFER_OVERFLOW)
        return "";

    // Step 2: Allocate buffer dynamically
    std::vector<BYTE> buffer(size);
    PIP_ADAPTER_INFO pAdapterInfo = (PIP_ADAPTER_INFO)buffer.data();

    // Step 3: Retrieve adapter information
    if (GetAdaptersInfo(pAdapterInfo, &size) != ERROR_SUCCESS)
        return "";

    // Step 4: Iterate through adapter linked list
    while (pAdapterInfo) {

        // Check if adapter type is Ethernet
        if (pAdapterInfo->Type == MIB_IF_TYPE_ETHERNET)
            return pAdapterInfo->Description;

        pAdapterInfo = pAdapterInfo->Next;  // Move to next adapter
    }

    return "";
}