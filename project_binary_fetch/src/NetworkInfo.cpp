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
#include <algorithm>
#include <vector>
#include <chrono>

#define WINVER 0x0600
#define _WIN32_WINNT 0x0600

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wlanapi.lib")
#pragma comment(lib, "winhttp.lib")

using namespace std;
using namespace std::chrono;

//-----------------------------------------get_local_ip--------------------------------//
string NetworkInfo::get_local_ip()
{
	string result = "Unknown";

	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
		return result;

	ULONG out_buff_len = 15000;
	PIP_ADAPTER_ADDRESSES adapter_addresses = (IP_ADAPTER_ADDRESSES*)malloc(out_buff_len);

	if (GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, adapter_addresses, &out_buff_len) == NO_ERROR)
	{
		for (PIP_ADAPTER_ADDRESSES adapter = adapter_addresses; adapter != NULL; adapter = adapter->Next)
		{
			if (adapter->OperStatus != IfOperStatusUp || adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK)
				continue;

			for (PIP_ADAPTER_UNICAST_ADDRESS ua = adapter->FirstUnicastAddress; ua != NULL; ua = ua->Next)
			{
				SOCKADDR_IN* sa_in = (SOCKADDR_IN*)ua->Address.lpSockaddr;
				if (sa_in->sin_family == AF_INET)
				{
					char ip_str[INET_ADDRSTRLEN];
					inet_ntop(AF_INET, &(sa_in->sin_addr), ip_str, sizeof(ip_str));

					int cidr = ua->OnLinkPrefixLength;
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
string NetworkInfo::get_mac_address()
{
	string mac = "Unknown";
	ULONG out_buf_len = 15000;
	PIP_ADAPTER_ADDRESSES adapter_addresses = (IP_ADAPTER_ADDRESSES*)malloc(out_buf_len);

	if (GetAdaptersAddresses(AF_UNSPEC, 0, NULL, adapter_addresses, &out_buf_len) == NO_ERROR)
	{
		for (PIP_ADAPTER_ADDRESSES adapter = adapter_addresses; adapter != NULL; adapter = adapter->Next)
		{
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

//-----------------------------------------get_network_name--------------------------------//
string NetworkInfo::get_network_name()
{
	string ssid_str = "Unknown";
	HANDLE hClient = NULL;
	DWORD dwMaxClient = 2;
	DWORD dwCurVersion = 0;

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
string NetworkInfo::get_public_ip()
{
	string public_ip = "Unknown";

	HINTERNET hSession = WinHttpOpen(L"NetworkInfo/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (!hSession) return public_ip;

	HINTERNET hConnect = WinHttpConnect(hSession, L"api.ipify.org", INTERNET_DEFAULT_HTTP_PORT, 0);
	if (!hConnect)
	{
		WinHttpCloseHandle(hSession);
		return public_ip;
	}

	HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", NULL, NULL, WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
	if (!hRequest)
	{
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return public_ip;
	}

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

	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);
	return public_ip;
}

//-----------------------------------------HELPER: Format Speed--------------------------------//
static string format_speed(double mbps)
{
	ostringstream oss;
	if (mbps >= 1000.0)
	{
		oss << fixed << setprecision(1) << (mbps / 1000.0) << " Gbps";
	}
	else if (mbps >= 1.0)
	{
		oss << fixed << setprecision(1) << mbps << " Mbps";
	}
	else
	{
		oss << fixed << setprecision(0) << (mbps * 1000.0) << " Kbps";
	}
	return oss.str();
}

//-----------------------------------------get_network_download_speed--------------------------------//
/**
 * Measures actual download speed by downloading test data
 * Uses a fast, lightweight test (downloads ~1-2MB)
 * @return Formatted speed string (e.g., "45.3 Mbps", "1.2 Gbps")
 */
string NetworkInfo::get_network_download_speed()
{
	string speed_str = "Unknown";

	// Fast test: Download ~1MB file from cloudflare CDN (very fast servers)
	HINTERNET hSession = WinHttpOpen(L"SpeedTest/1.0",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		0);

	if (!hSession) return speed_str;

	// Set timeout to 5 seconds for fast response
	DWORD timeout = 5000;
	WinHttpSetOption(hSession, WINHTTP_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
	WinHttpSetOption(hSession, WINHTTP_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));

	// Use cloudflare's speed test file (1MB)
	HINTERNET hConnect = WinHttpConnect(hSession, L"speed.cloudflare.com", INTERNET_DEFAULT_HTTP_PORT, 0);

	if (!hConnect)
	{
		WinHttpCloseHandle(hSession);
		return speed_str;
	}

	HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", L"/__down?bytes=1000000",
		NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);

	if (!hRequest)
	{
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return speed_str;
	}

	// Start timing
	auto start_time = high_resolution_clock::now();
	DWORD total_bytes = 0;

	if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
		WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
		WinHttpReceiveResponse(hRequest, NULL))
	{
		DWORD bytes_available = 0;
		char buffer[8192];

		// Read all data
		while (WinHttpQueryDataAvailable(hRequest, &bytes_available) && bytes_available > 0)
		{
			DWORD bytes_read = 0;
			DWORD to_read = min(bytes_available, sizeof(buffer));

			if (WinHttpReadData(hRequest, buffer, to_read, &bytes_read))
			{
				total_bytes += bytes_read;
			}
			else
			{
				break;
			}
		}

		// End timing
		auto end_time = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(end_time - start_time).count();

		if (duration > 0 && total_bytes > 0)
		{
			// Calculate speed in Mbps
			double seconds = duration / 1000.0;
			double megabits = (total_bytes * 8.0) / 1000000.0;
			double mbps = megabits / seconds;

			speed_str = format_speed(mbps);
		}
	}

	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);

	return speed_str;
}

//-----------------------------------------get_network_upload_speed--------------------------------//
/**
 * Measures actual upload speed by uploading test data
 * Uses a fast, lightweight test (uploads ~500KB)
 * @return Formatted speed string (e.g., "23.5 Mbps", "890 Kbps")
 */
string NetworkInfo::get_network_upload_speed()
{
	string speed_str = "Unknown";

	HINTERNET hSession = WinHttpOpen(L"SpeedTest/1.0",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		0);

	if (!hSession) return speed_str;

	// Set timeout to 5 seconds
	DWORD timeout = 5000;
	WinHttpSetOption(hSession, WINHTTP_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
	WinHttpSetOption(hSession, WINHTTP_OPTION_SEND_TIMEOUT, &timeout, sizeof(timeout));

	// Use cloudflare's speed test endpoint
	HINTERNET hConnect = WinHttpConnect(hSession, L"speed.cloudflare.com", INTERNET_DEFAULT_HTTP_PORT, 0);

	if (!hConnect)
	{
		WinHttpCloseHandle(hSession);
		return speed_str;
	}

	HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", L"/__up",
		NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);

	if (!hRequest)
	{
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return speed_str;
	}

	// Create test data (500KB of random data)
	const DWORD test_size = 500000;
	char* test_data = new char[test_size];

	// Fill with pattern (faster than random)
	for (DWORD i = 0; i < test_size; i++)
	{
		test_data[i] = (char)(i % 256);
	}

	// Start timing
	auto start_time = high_resolution_clock::now();

	// Send request with data
	BOOL success = WinHttpSendRequest(hRequest,
		L"Content-Type: application/octet-stream\r\n",
		-1,
		test_data,
		test_size,
		test_size,
		0);

	if (success && WinHttpReceiveResponse(hRequest, NULL))
	{
		// End timing
		auto end_time = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(end_time - start_time).count();

		if (duration > 0)
		{
			// Calculate speed in Mbps
			double seconds = duration / 1000.0;
			double megabits = (test_size * 8.0) / 1000000.0;
			double mbps = megabits / seconds;

			speed_str = format_speed(mbps);
		}
	}

	delete[] test_data;
	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);

	return speed_str;
}

/*
================================================================================
				NETWORK SPEED FUNCTIONS DOCUMENTATION
================================================================================

NEW FUNCTIONS:

1. get_network_download_speed()
   - Downloads ~1MB test file from Cloudflare CDN
   - Measures time taken to download
   - Calculates actual download speed in Mbps/Gbps
   - Fast execution (~1-2 seconds)
   - Returns: "45.3 Mbps", "1.2 Gbps", "850 Kbps"

2. get_network_upload_speed()
   - Uploads ~500KB test data to Cloudflare endpoint
   - Measures time taken to upload
   - Calculates actual upload speed in Mbps/Gbps
   - Fast execution (~1-2 seconds)
   - Returns: "23.5 Mbps", "890 Kbps", "1.1 Gbps"

FEATURES:
- Uses Cloudflare's speed test infrastructure (fast, reliable)
- 5-second timeout to prevent hanging
- Automatic unit formatting (Kbps/Mbps/Gbps)
- Returns "Unknown" if test fails

EXAMPLE OUTPUTS:
- Download: "85.3 Mbps"
- Upload: "23.7 Mbps"
- High-speed: "1.2 Gbps"
- Low-speed: "450 Kbps"

================================================================================
*/