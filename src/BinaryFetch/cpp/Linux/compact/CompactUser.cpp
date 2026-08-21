#include "CompactUser.h"
#include <unistd.h>
#include <pwd.h>
#include <climits>

// Get current logged-in username
std::string CompactUser::getUsername() {
    uid_t uid = geteuid();
    struct passwd* pw = getpwuid(uid);
    if (pw) {
        return std::string(pw->pw_name);
    }
    // Fallback to environment variable if getpwuid fails
    char* user = getenv("USER");
    if (user) {
        return std::string(user);
    }
    return "Unknown User";
}

// Get the domain/hostname on Linux
std::string CompactUser::getDomain() {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        return std::string(hostname);
    }
    return "UnknownDomain";
}

// Check if user has administrative privileges (root)
std::string CompactUser::isAdmin() {
    if (geteuid() == 0) {
        return "Admin";
    }
    return "Non-Admin";
}
