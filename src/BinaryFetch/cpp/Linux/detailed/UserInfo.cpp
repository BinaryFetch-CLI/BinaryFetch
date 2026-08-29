#include "UserInfo.h"
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <cstring>
#include <sstream>
#include <vector>
#include <algorithm>

string UserInfo::get_username()
{
    const char* username = getlogin();
    if (username != nullptr) {
        return string(username);
    }
    
    // Fallback: get username from UID
    uid_t uid = geteuid();
    struct passwd* pw = getpwuid(uid);
    if (pw != nullptr) {
        return string(pw->pw_name);
    }
    
    return "unknown";
}

string UserInfo::get_computer_name()
{
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        return string(hostname);
    }
    return "unknown";
}

string UserInfo::get_domain_name()
{
    char domainname[256];
    memset(domainname, 0, sizeof(domainname));
    
    if (getdomainname(domainname, sizeof(domainname)) == 0) {
        string domain(domainname);
        
        // Check for empty or default values
        if (domain.empty() || domain == "(none)") {
            return "Not configured";
        }
        return domain;
    }
    
    return "Error retrieving domain";
}

string UserInfo::get_user_groups()
{
    uid_t uid = geteuid();
    struct passwd* pw = getpwuid(uid);
    
    if (pw == nullptr) {
        return "unknown";
    }
    
    vector<string> groups;
    int ngroups = 10;
    gid_t grouplist[10];
    
    // Get all groups the user belongs to
    if (getgroups(ngroups, grouplist) > 0) {
        for (int i = 0; i < ngroups && grouplist[i] != 0; i++) {
            struct group* gr = getgrgid(grouplist[i]);
            if (gr != nullptr) {
                groups.push_back(string(gr->gr_name));
            }
        }
    }
    
    // Add primary group
    struct group* gr = getgrgid(pw->pw_gid);
    if (gr != nullptr) {
        string primary_group(gr->gr_name);
        // Avoid duplicates
        auto it = find(groups.begin(), groups.end(), primary_group);
        if (it == groups.end()) {
            groups.insert(groups.begin(), primary_group);
        }
    }
    
    // Format groups as comma-separated string
    stringstream ss;
    for (size_t i = 0; i < groups.size(); i++) {
        ss << groups[i];
        if (i < groups.size() - 1) {
            ss << ", ";
        }
    }
    
    return ss.str();
}