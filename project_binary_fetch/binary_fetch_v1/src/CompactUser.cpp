#include "CompactUser.h"
#include <Windows.h>
#include <lmcons.h>
#include <iostream>
#include <shellapi.h>
#include <sddl.h>
using namespace std;

string CompactUser::getUsername()
{
	char username[UNLEN + 1];
	DWORD size = UNLEN + 1;
	if (GetUserNameA(username, &size))
	{
		return string(username);
	}else { return "Uknown User"; }
}

std::string CompactUser::getDomain() {
    char computerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
    if (GetComputerNameA(computerName, &size))
        return std::string(computerName);
    return "UnknownDomain";
}

std::string CompactUser::isAdmin() {
    BOOL isAdmin = FALSE;
    HANDLE hToken = NULL;
    DWORD size = 0;

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        GetTokenInformation(hToken, TokenGroups, NULL, 0, &size);
        BYTE* buffer = new BYTE[size];
        if (GetTokenInformation(hToken, TokenGroups, buffer, size, &size)) {
            PTOKEN_GROUPS groups = (PTOKEN_GROUPS)buffer;
            SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
            PSID AdministratorsGroup;
            if (AllocateAndInitializeSid(&NtAuthority, 2,
                SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
                0, 0, 0, 0, 0, 0, &AdministratorsGroup))
            {
                for (DWORD i = 0; i < groups->GroupCount; i++) {
                    if (EqualSid(groups->Groups[i].Sid, AdministratorsGroup)) {
                        isAdmin = (groups->Groups[i].Attributes & SE_GROUP_ENABLED) != 0;
                        break;
                    }
                }
                FreeSid(AdministratorsGroup);
            }
        }
        delete[] buffer;
        CloseHandle(hToken);
    }

    return isAdmin ? "Admin" : "Non-Admin";
}