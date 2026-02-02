#pragma once
#include <string>
using namespace std;

class NetworkInfo {
public:
	string get_local_ip();      //returns local IPv4 with subnet mask (e.g., "192.168.0.9/24")
	string get_mac_address();   //Returns MAC address (e.g., "A4:B1:C1:23:8F:99")
	string get_locale();        //Returns system locale (e.g., "en-us"
	string get_network_name();  //Returns connected network name
	string get_network_upload_speed(); //Rturns connected network's upload speed
	string get_network_download_speed();//Returns connected network's download speed
	string get_public_ip();     //Returns public ip (if it's available)
};  