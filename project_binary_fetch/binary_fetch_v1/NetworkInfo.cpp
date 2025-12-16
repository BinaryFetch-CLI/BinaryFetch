#include "NetworkInfo.h"
#include <WinSock2.h>
#include <iphlpapi.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <locale>
#include <iomanip>
#include <sstream>
#include <netioapi.h>
#include <wlanapi.h>
#include <winhttp.h>
#include <algorithm>  // For std::transform
#include <vector>     // For std::vector (if needed later)

// Add this before any includes to target Windows Vista or later
#define WINVER 0x0600
#define _WIN32_WINNT 0x0600

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wlanapi.lib")
#pragma comment(lib, "winhttp.lib")

using namespace std;

//-----------------------------------------get_local_ip--------------------------------//
//Get local ip address and subnet mask (e.g., "192.168.0.9/24")
string NetworkInfo::get_local_ip()
{
	string result = "Unknown";

	// initialize Winsock
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
		return result;

	ULONG out_buff_len = 15000;
	PIP_ADAPTER_ADDRESSES adapter_addresses = (IP_ADAPTER_ADDRESSES*)malloc(out_buff_len);

	if (GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, adapter_addresses, &out_buff_len) == NO_ERROR)
	{
		for (PIP_ADAPTER_ADDRESSES adapter = adapter_addresses; adapter != NULL; adapter = adapter->Next)
		{
			//skip disconnected or loopback adapters
			if (adapter->OperStatus != IfOperStatusUp || adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK)
				continue;

			// iterate through all unicast addresses (IPv4)
			for (PIP_ADAPTER_UNICAST_ADDRESS ua = adapter->FirstUnicastAddress; ua != NULL; ua = ua->Next)
			{
				SOCKADDR_IN* sa_in = (SOCKADDR_IN*)ua->Address.lpSockaddr;
				if (sa_in->sin_family == AF_INET)
				{
					char ip_str[INET_ADDRSTRLEN];
					inet_ntop(AF_INET, &(sa_in->sin_addr), ip_str, sizeof(ip_str));

					int cidr = ua->OnLinkPrefixLength; // prefix length = CIDR bits
					ostringstream oss;
					oss << ip_str << "/" << cidr;

					result = oss.str();
					free(adapter_addresses);
					WSACleanup();
					return result;
				}
			}
		}
	}
	free(adapter_addresses);
	WSACleanup();
	return result;
}

//-----------------------------------------get_mac_address--------------------------------//
//get Mac address of main (active) adapter
string NetworkInfo::get_mac_address()
{
	string mac = "Unknown";
	ULONG out_buf_len = 15000;
	PIP_ADAPTER_ADDRESSES adapter_addresses = (IP_ADAPTER_ADDRESSES*)malloc(out_buf_len);

	if (GetAdaptersAddresses(AF_UNSPEC, 0, NULL, adapter_addresses, &out_buf_len) == NO_ERROR)
	{
		for (PIP_ADAPTER_ADDRESSES adapter = adapter_addresses; adapter != NULL; adapter = adapter->Next)
		{
			// skip disconnected or loopback adapters
			if (adapter->OperStatus != IfOperStatusUp || adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK)
				continue;

			ostringstream oss;
			for (UINT i = 0; i < adapter->PhysicalAddressLength; i++)
			{
				if (i != 0) oss << ":";
				oss << hex << uppercase << setw(2) << setfill('0') << (int)adapter->PhysicalAddress[i];
			}
			mac = oss.str();
			break;
		}
	}
	free(adapter_addresses);
	return mac;
}

//-----------------------------------------get_locale--------------------------------//
// get system locale (e.g., "en-US")
string NetworkInfo::get_locale()
{
	WCHAR locale_name[LOCALE_NAME_MAX_LENGTH];
	if (GetUserDefaultLocaleName(locale_name, LOCALE_NAME_MAX_LENGTH))
	{
		char locale_str[LOCALE_NAME_MAX_LENGTH];
		WideCharToMultiByte(CP_UTF8, 0, locale_name, -1, locale_str, sizeof(locale_str), NULL, NULL);
		return string(locale_str);
	}
	return "Unknown";
}

//-----------------------------------------HELPER FUNCTIONS--------------------------------//

/**
 * Helper: Check if adapter name suggests it's virtual
 * @param desc - Adapter description string
 * @return true if adapter appears to be virtual, false otherwise
 */
static bool is_virtual_adapter(const string& desc)
{
	// Common virtual adapter keywords (case-insensitive check)
	const char* virtualKeywords[] = {
		"virtual", "vmware", "virtualbox", "hyper-v",
		"vethernet", "loopback", "tunnel", "vpn",
		"tap-windows", "tap-win", "bluetooth", "wan miniport",
		"microsoft wi-fi direct", "teredo", "isatap"
	};

	string lowerDesc = desc;
	transform(lowerDesc.begin(), lowerDesc.end(), lowerDesc.begin(), ::tolower);

	for (const auto& keyword : virtualKeywords)
	{
		if (lowerDesc.find(keyword) != string::npos)
			return true;
	}
	return false;
}

/**
 * Helper: Check if adapter type is physical (not virtual/loopback)
 * @param type - Interface type constant
 * @return true if physical adapter, false otherwise
 */
static bool is_physical_adapter(DWORD type)
{
	return (type == IF_TYPE_ETHERNET_CSMACD ||    // Ethernet
		type == IF_TYPE_IEEE80211 ||           // WiFi
		type == IF_TYPE_GIGABITETHERNET);      // Gigabit Ethernet
}

/**
 * Helper: Get default gateway interface index (most likely internet-connected)
 * @return Interface index of default gateway adapter, or 0 if not found
 */
static DWORD get_default_gateway_interface()
{
	DWORD dwSize = 0;
	DWORD dwRetVal = 0;
	PMIB_IPFORWARDTABLE pIpForwardTable = NULL;

	// Get the size needed
	dwRetVal = GetIpForwardTable(NULL, &dwSize, 0);
	if (dwRetVal == ERROR_INSUFFICIENT_BUFFER)
	{
		pIpForwardTable = (MIB_IPFORWARDTABLE*)malloc(dwSize);
		if (pIpForwardTable)
		{
			dwRetVal = GetIpForwardTable(pIpForwardTable, &dwSize, 0);
			if (dwRetVal == NO_ERROR)
			{
				// Find default route (destination 0.0.0.0)
				for (DWORD i = 0; i < pIpForwardTable->dwNumEntries; i++)
				{
					if (pIpForwardTable->table[i].dwForwardDest == 0)
					{
						DWORD ifIndex = pIpForwardTable->table[i].dwForwardIfIndex;
						free(pIpForwardTable);
						return ifIndex;
					}
				}
			}
			free(pIpForwardTable);
		}
	}
	return 0;
}

//-----------------------------------------get_network_speed--------------------------------//
/**
 * Get the speed of the primary/active network connection
 * Prioritizes the adapter with default gateway (actual internet connection)
 * Filters out virtual adapters (VirtualBox, VMware, VPN, etc.)
 * @return String with speed formatted as "1.0 Gbps (Ethernet)" or "300 Mbps (WiFi)"
 */
string NetworkInfo::get_network_speed()
{
	string net_speed_str = "Unknown";
	MIB_IFTABLE* ifTable = NULL;
	DWORD dwSize = 0;

	// Get the interface index of the default gateway (internet connection)
	DWORD defaultIfIndex = get_default_gateway_interface();

	// First call to get the buffer size
	if (GetIfTable(NULL, &dwSize, FALSE) == ERROR_INSUFFICIENT_BUFFER)
	{
		ifTable = (MIB_IFTABLE*)malloc(dwSize);
		if (ifTable && GetIfTable(ifTable, &dwSize, FALSE) == NO_ERROR)
		{
			// First pass: Try to find the default gateway adapter
			if (defaultIfIndex != 0)
			{
				for (DWORD i = 0; i < ifTable->dwNumEntries; ++i)
				{
					MIB_IFROW row = ifTable->table[i];

					// Check if this is the default gateway interface
					if (row.dwIndex == defaultIfIndex &&
						row.dwOperStatus == IF_OPER_STATUS_OPERATIONAL &&
						is_physical_adapter(row.dwType))
					{
						string adapterDesc((char*)row.bDescr, row.dwDescrLen);

						// Skip if virtual adapter
						if (is_virtual_adapter(adapterDesc))
							continue;

						// Speed is in bits per second, convert to Mbps
						double speed_mbps = row.dwSpeed / 1e6;

						ostringstream oss;
						// Format based on speed magnitude
						if (speed_mbps >= 1000.0)
						{
							double speedGbps = speed_mbps / 1000.0;
							oss << fixed << setprecision(1) << speedGbps << " Gbps";
						}
						else
						{
							oss << fixed << setprecision(0) << speed_mbps << " Mbps";
						}

						// Add connection type indicator
						if (row.dwType == IF_TYPE_IEEE80211)
							oss << " (WiFi)";
						else if (row.dwType == IF_TYPE_ETHERNET_CSMACD || row.dwType == IF_TYPE_GIGABITETHERNET)
							oss << " (Ethernet)";

						net_speed_str = oss.str();
						free(ifTable);
						return net_speed_str;
					}
				}
			}

			// Second pass: Fallback to first operational physical adapter
			for (DWORD i = 0; i < ifTable->dwNumEntries; ++i)
			{
				MIB_IFROW row = ifTable->table[i];

				// Check if adapter is operational and has a physical address
				if (row.dwOperStatus == IF_OPER_STATUS_OPERATIONAL &&
					row.dwPhysAddrLen != 0 &&
					is_physical_adapter(row.dwType))
				{
					string adapterDesc((char*)row.bDescr, row.dwDescrLen);

					// Skip if virtual adapter
					if (is_virtual_adapter(adapterDesc))
						continue;

					// Speed is in bits per second, convert to Mbps
					double speed_mbps = row.dwSpeed / 1e6;

					ostringstream oss;
					// Format based on speed magnitude
					if (speed_mbps >= 1000.0)
					{
						double speedGbps = speed_mbps / 1000.0;
						oss << fixed << setprecision(1) << speedGbps << " Gbps";
					}
					else
					{
						oss << fixed << setprecision(0) << speed_mbps << " Mbps";
					}

					// Add connection type indicator
					if (row.dwType == IF_TYPE_IEEE80211)
						oss << " (WiFi)";
					else if (row.dwType == IF_TYPE_ETHERNET_CSMACD || row.dwType == IF_TYPE_GIGABITETHERNET)
						oss << " (Ethernet)";

					net_speed_str = oss.str();
					break;
				}
			}
		}
		if (ifTable) free(ifTable);
	}
	return net_speed_str;
}

//-----------------------------------------get_network_name--------------------------------//
// get connected network public name (SSID)
string NetworkInfo::get_network_name()
{
	string ssid_str = "Unknown";
	HANDLE hClient = NULL;
	DWORD dwMaxClient = 2;
	DWORD dwCurVersion = 0;

	// open wlan handle
	if (WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient) != ERROR_SUCCESS)
		return ssid_str;

	PWLAN_INTERFACE_INFO_LIST pIfList = NULL;
	if (WlanEnumInterfaces(hClient, NULL, &pIfList) != ERROR_SUCCESS)
	{
		WlanCloseHandle(hClient, NULL);
		return ssid_str;
	}

	for (DWORD i = 0; i < pIfList->dwNumberOfItems; ++i)
	{
		PWLAN_INTERFACE_INFO pIfInfo = &pIfList->InterfaceInfo[i];
		PWLAN_AVAILABLE_NETWORK_LIST pBssList = NULL;

		if (WlanGetAvailableNetworkList(hClient, &pIfInfo->InterfaceGuid, 0, NULL, &pBssList) == ERROR_SUCCESS)
		{
			for (DWORD j = 0; j < pBssList->dwNumberOfItems; ++j)
			{
				const auto& net = pBssList->Network[j];
				if (net.dwFlags & WLAN_AVAILABLE_NETWORK_CONNECTED)
				{
					// extract SSID name
					ssid_str = string((char*)net.dot11Ssid.ucSSID, net.dot11Ssid.uSSIDLength);
					break;
				}
			}
			if (pBssList) WlanFreeMemory(pBssList);
		}
	}

	if (pIfList) WlanFreeMemory(pIfList);
	WlanCloseHandle(hClient, NULL);
	return ssid_str;
}

//-----------------------------------------get_public_ip--------------------------------//
// get public ip of the connected network
string NetworkInfo::get_public_ip()
{
	string public_ip = "Unknown";

	// open HTTP session
	HINTERNET hSession = WinHttpOpen(L"NetworkInfo/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (!hSession) return public_ip;

	// connect to ipify.org
	HINTERNET hConnect = WinHttpConnect(hSession, L"api.ipify.org", INTERNET_DEFAULT_HTTP_PORT, 0);
	if (!hConnect)
	{
		WinHttpCloseHandle(hSession);
		return public_ip;
	}

	// create HTTP request
	HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", NULL, NULL, WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
	if (!hRequest)
	{
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return public_ip;
	}

	// send and receive HTTP data
	if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
		WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
		WinHttpReceiveResponse(hRequest, NULL))
	{
		DWORD size = 0;
		WinHttpQueryDataAvailable(hRequest, &size);
		if (size > 0)
		{
			char* buffer = new char[size + 1];
			ZeroMemory(buffer, size + 1);
			DWORD read = 0;
			WinHttpReadData(hRequest, buffer, size, &read);
			public_ip = string(buffer, read);
			delete[] buffer;
		}
	}

	// cleanup handles
	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);
	return public_ip;
}

/*
================================================================================
					 NETWORK SPEED FUNCTION DOCUMENTATION
================================================================================

IMPROVEMENTS MADE TO get_network_speed():

1. **Finds Default Gateway Interface**
   - Uses GetIpForwardTable() to identify the adapter routing to the internet
   - Prioritizes this adapter over others

2. **Filters Virtual Adapters**
   - Excludes VirtualBox, VMware, Hyper-V, VPN, Bluetooth adapters
   - Only returns physical Ethernet/WiFi adapters

3. **Better Speed Formatting**
   - Shows "1.0 Gbps" for gigabit+ connections (?1000 Mbps)
   - Shows "100 Mbps" or "300 Mbps" for sub-gigabit speeds
   - Adds connection type: "(Ethernet)" or "(WiFi)"

4. **Two-Pass Algorithm**
   - First pass: Find default gateway adapter (most accurate)
   - Second pass: Fallback to first operational physical adapter

HELPER FUNCTIONS:

- get_default_gateway_interface(): Returns interface index of default route
- is_physical_adapter(): Checks if adapter is physical (not virtual)
- is_virtual_adapter(): Detects virtual adapters by description keywords

INTERFACE TYPES DETECTED:
- IF_TYPE_ETHERNET_CSMACD (6): Standard Ethernet
- IF_TYPE_IEEE80211 (71): WiFi/Wireless LAN
- IF_TYPE_GIGABITETHERNET: Gigabit Ethernet

EXAMPLE OUTPUTS:
- "1.0 Gbps (Ethernet)" - Gigabit Ethernet connection
- "2.5 Gbps (Ethernet)" - 2.5 Gigabit connection
- "300 Mbps (WiFi)" - WiFi connection
- "100 Mbps (Ethernet)" - Fast Ethernet

================================================================================
*/