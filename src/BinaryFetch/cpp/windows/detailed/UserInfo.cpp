#include "UserInfo.h"
#include <Windows.h>
#include <lm.h>
#include <iostream>
#include <lmwksta.h> // for NetWkstaGetInfo
#pragma comment(lib, "netapi32.lib")
using namespace std;

// Get current username
string UserInfo::get_username()
{
    char username[UNLEN + 1];
    DWORD size = UNLEN + 1;
    if (GetUserNameA(username, &size))
        return string(username);
    return "Unknown User Name";
} 

string UserInfo::get_domain_name()
{
    // First try normal domain name
    char domainName[256];
    DWORD size = 256;
    if (GetComputerNameExA(ComputerNameDnsDomain, domainName, &size) && strlen(domainName) > 0)
        return string(domainName);

    // Fallback method using NetWkstaGetInfo
    LPWKSTA_INFO_100 pBuf = NULL;
    NET_API_STATUS nStatus;

    nStatus = NetWkstaGetInfo(NULL, 100, (LPBYTE*)&pBuf);

    if (nStatus == NERR_Success && pBuf != NULL)
    {
        if (pBuf->wki100_langroup != NULL)
        {
            wstring ws(pBuf->wki100_langroup);
            string domain(ws.begin(), ws.end());
            NetApiBufferFree(pBuf);
            return domain;
        }
        NetApiBufferFree(pBuf);
    }

    return "No Domain / Workgroup";
}

// Get user groups
string UserInfo::get_user_groups()
{
    LPLOCALGROUP_USERS_INFO_0 pBuf = NULL;
    DWORD entriesRead = 0;
    DWORD totalEntries = 0;
    NET_API_STATUS nStatus;

    string username = get_username();
    wstring wusername(username.begin(), username.end());

    nStatus = NetUserGetLocalGroups(
        NULL,
        wusername.c_str(),
        0,
        LG_INCLUDE_INDIRECT,
        (LPBYTE*)&pBuf,
        MAX_PREFERRED_LENGTH,
        &entriesRead,
        &totalEntries
    );

    if (nStatus == NERR_Success && pBuf != NULL)
    {
        string groups;
        for (DWORD i = 0; i < entriesRead; i++)
        {
            wstring ws(pBuf[i].lgrui0_name);
            string group(ws.begin(), ws.end());
            groups += group + (i < entriesRead - 1 ? ", " : "");
        }
        NetApiBufferFree(pBuf);
        return groups.empty() ? "No Groups Found" : groups;
    }                                                                  
    return "Failed to retrieve groups";
}

// Get computer name
string UserInfo::get_computer_name()
{
    char computerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
    if (GetComputerNameA(computerName, &size))
        return string(computerName);
    return "Unknown System";
}
