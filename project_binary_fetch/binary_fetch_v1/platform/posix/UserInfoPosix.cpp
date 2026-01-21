#include "../Platform.h"
#include "../../UserInfo.h"
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <cstring>
#include <vector>

std::string UserInfo::get_username() {
    const char* user = std::getenv("USER");
    if (user) return std::string(user);
    
    struct passwd* pw = getpwuid(getuid());
    if (pw) return std::string(pw->pw_name);
    
    return "Unknown User Name";
}

std::string UserInfo::get_domain_name() {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        char* dot = strchr(hostname, '.');
        if (dot && *(dot + 1)) {
            return std::string(dot + 1);
        }
    }
    
    std::string result = Platform::exec("hostname -d 2>/dev/null");
    result = Platform::trim(result);
    if (!result.empty() && result != "(none)") {
        return result;
    }
    
    std::string resolv = Platform::readFile("/etc/resolv.conf");
    std::string search = Platform::parseValue(resolv, "search", ' ');
    if (!search.empty()) {
        auto parts = Platform::split(search, ' ');
        if (!parts.empty()) return parts[0];
    }
    
    std::string domain = Platform::parseValue(resolv, "domain", ' ');
    if (!domain.empty()) return domain;
    
    return "No Domain / Workgroup";
}

std::string UserInfo::get_user_groups() {
    gid_t groups[64];
    int ngroups = 64;
    
    struct passwd* pw = getpwuid(getuid());
    if (!pw) return "Failed to retrieve groups";
    
    if (getgrouplist(pw->pw_name, pw->pw_gid, groups, &ngroups) == -1) {
        return "Failed to retrieve groups";
    }
    
    std::string result;
    for (int i = 0; i < ngroups; i++) {
        struct group* gr = getgrgid(groups[i]);
        if (gr) {
            if (!result.empty()) result += ", ";
            result += gr->gr_name;
        }
    }
    
    return result.empty() ? "No Groups Found" : result;
}

std::string UserInfo::get_computer_name() {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        char* dot = strchr(hostname, '.');
        if (dot) *dot = '\0';
        return std::string(hostname);
    }
    return "Unknown System";
}
