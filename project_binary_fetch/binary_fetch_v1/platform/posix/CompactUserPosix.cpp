#include "CompactUser.h"
#include "platform/Platform.h"
#include <unistd.h>
#include <pwd.h>
#include <climits>

#if PLATFORM_POSIX

std::string CompactUser::getUsername() {
    struct passwd* pw = getpwuid(getuid());
    if (pw && pw->pw_name) {
        return std::string(pw->pw_name);
    }
    
    const char* user = getenv("USER");
    if (user) return std::string(user);
    
    return "Unknown";
}

std::string CompactUser::getDomain() {
    char hostname[HOST_NAME_MAX + 1] = {0};
    if (gethostname(hostname, sizeof(hostname) - 1) == 0) {
        return std::string(hostname);
    }
    return "localhost";
}

std::string CompactUser::isAdmin() {
    if (getuid() == 0 || geteuid() == 0) {
        return "Root";
    }
    
    if (Platform::commandExists("sudo")) {
        std::string groups = Platform::exec("groups 2>/dev/null");
        if (groups.find("sudo") != std::string::npos ||
            groups.find("wheel") != std::string::npos ||
            groups.find("admin") != std::string::npos) {
            return "Sudoer";
        }
    }
    
    return "User";
}

#endif
