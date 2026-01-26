#include "CompactNetwork.h"
#include <string>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <wlanapi.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "wlanapi.lib")
#pragma comment(lib, "ole32.lib")

// ------------------- Public Functions -------------------

std::string CompactNetwork::get_network_name() {
    std::string ssidName = get_wifi_ssid();
    if (!ssidName.empty()) return ssidName;

    std::string adapterName = get_ethernet_name();
    return adapterName.empty() ? "Unknown" : adapterName;
}

std::string CompactNetwork::get_network_type() {
    return !get_wifi_ssid().empty() ? "WiFi" : "Ethernet";
}

std::string CompactNetwork::get_network_ip() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return "Unknown";

    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        WSACleanup();
        return "Unknown";
    }

    addrinfo hints = {}, * res = nullptr;
    hints.ai_family = AF_INET; // IPv4 only

    if (getaddrinfo(hostname, nullptr, &hints, &res) != 0) {
        WSACleanup();
        return "Unknown";
    }

    std::string ip = "Unknown";
    for (addrinfo* p = res; p != nullptr; p = p->ai_next) {
        sockaddr_in* ipv4 = reinterpret_cast<sockaddr_in*>(p->ai_addr);
        char ipStr[INET_ADDRSTRLEN];
        if (InetNtopA(AF_INET, &ipv4->sin_addr, ipStr, INET_ADDRSTRLEN)) {
            ip = ipStr;
            break;
        }
    }

    freeaddrinfo(res);
    WSACleanup();
    return ip;
}

// ------------------- Private Helpers -------------------

std::string CompactNetwork::get_wifi_ssid() {
    HANDLE hClient = NULL;
    DWORD dwMaxClient = 2;
    DWORD dwCurVersion = 0;
    if (WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient) != ERROR_SUCCESS)
        return "";

    PWLAN_INTERFACE_INFO_LIST pIfList = NULL;
    if (WlanEnumInterfaces(hClient, NULL, &pIfList) != ERROR_SUCCESS) {
        WlanCloseHandle(hClient, NULL);
        return "";
    }

    std::string ssid = "";
    for (unsigned int i = 0; i < pIfList->dwNumberOfItems; ++i) {
        PWLAN_INTERFACE_INFO pIfInfo = &pIfList->InterfaceInfo[i];
        PWLAN_CONNECTION_ATTRIBUTES pConnectInfo = NULL;
        DWORD connectSize = sizeof(WLAN_CONNECTION_ATTRIBUTES);
        WLAN_OPCODE_VALUE_TYPE opCode;
        if (WlanQueryInterface(hClient, &pIfInfo->InterfaceGuid, wlan_intf_opcode_current_connection,
            NULL, &connectSize, (PVOID*)&pConnectInfo, &opCode) == ERROR_SUCCESS) {
            if (pConnectInfo->isState == wlan_interface_state_connected) {
                ssid = std::string((char*)pConnectInfo->wlanAssociationAttributes.dot11Ssid.ucSSID,
                    pConnectInfo->wlanAssociationAttributes.dot11Ssid.uSSIDLength);
                WlanFreeMemory(pConnectInfo);
                break;
            }
            WlanFreeMemory(pConnectInfo);
        }
    }

    WlanFreeMemory(pIfList);
    WlanCloseHandle(hClient, NULL);
    return ssid;
}

std::string CompactNetwork::get_ethernet_name() {
    ULONG size = 0;
    if (GetAdaptersInfo(nullptr, &size) != ERROR_BUFFER_OVERFLOW) return "";
    std::vector<BYTE> buffer(size);
    PIP_ADAPTER_INFO pAdapterInfo = (PIP_ADAPTER_INFO)buffer.data();
    if (GetAdaptersInfo(pAdapterInfo, &size) != ERROR_SUCCESS) return "";

    while (pAdapterInfo) {
        if (pAdapterInfo->Type == MIB_IF_TYPE_ETHERNET)
            return pAdapterInfo->Description;
        pAdapterInfo = pAdapterInfo->Next;
    }
    return "";
}
