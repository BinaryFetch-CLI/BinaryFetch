#pragma once

#include "PlatformConfig.h"
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <fstream>
#include <sstream>

#if PLATFORM_POSIX
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#endif

#if PLATFORM_FREEBSD
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

namespace Platform {

inline std::string exec(const std::string& cmd) {
    std::string result;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return result;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

inline std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

inline std::string readFileLine(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return "";
    std::string line;
    std::getline(file, line);
    return line;
}

inline bool fileExists(const std::string& path) {
#if PLATFORM_WINDOWS
    return false;
#else
    struct stat st;
    return stat(path.c_str(), &st) == 0;
#endif
}

inline bool commandExists(const std::string& cmd) {
#if PLATFORM_WINDOWS
    return false;
#else
    std::string check = "command -v " + cmd + " >/dev/null 2>&1";
    return system(check.c_str()) == 0;
#endif
}

inline std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

inline std::vector<std::string> split(const std::string& str, char delim) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, delim)) {
        tokens.push_back(token);
    }
    return tokens;
}

inline std::string parseValue(const std::string& content, const std::string& key, char delim = ':') {
    std::istringstream iss(content);
    std::string line;
    while (std::getline(iss, line)) {
        size_t pos = line.find(key);
        if (pos == 0 || (pos != std::string::npos && (pos == 0 || !std::isalnum(line[pos-1])))) {
            size_t delimPos = line.find(delim, pos);
            if (delimPos != std::string::npos) {
                std::string value = line.substr(delimPos + 1);
                value.erase(0, value.find_first_not_of(" \t\""));
                value.erase(value.find_last_not_of(" \t\n\r\"") + 1);
                return value;
            }
        }
    }
    return "";
}

inline std::string getEnv(const std::string& name) {
    const char* val = std::getenv(name.c_str());
    return val ? std::string(val) : "";
}

inline std::string getHomeDir() {
#if PLATFORM_POSIX
    const char* home = std::getenv("HOME");
    if (home) return std::string(home);
    struct passwd* pw = getpwuid(getuid());
    if (pw) return std::string(pw->pw_dir);
#endif
    return "";
}

inline std::string getConfigDir() {
#if PLATFORM_WINDOWS
    return "C:\\Users\\Public\\BinaryFetch";
#else
    std::string xdg = getEnv("XDG_CONFIG_HOME");
    if (!xdg.empty()) return xdg + "/binaryfetch";
    return getHomeDir() + "/.config/binaryfetch";
#endif
}

#if PLATFORM_FREEBSD
inline std::string sysctlString(const std::string& name) {
    char buf[256] = {0};
    size_t len = sizeof(buf);
    if (sysctlbyname(name.c_str(), buf, &len, nullptr, 0) == 0) {
        return std::string(buf);
    }
    return "";
}

inline long sysctlLong(const std::string& name) {
    long val = 0;
    size_t len = sizeof(val);
    if (sysctlbyname(name.c_str(), &val, &len, nullptr, 0) == 0) {
        return val;
    }
    return 0;
}

inline unsigned long sysctlULong(const std::string& name) {
    unsigned long val = 0;
    size_t len = sizeof(val);
    if (sysctlbyname(name.c_str(), &val, &len, nullptr, 0) == 0) {
        return val;
    }
    return 0;
}
#endif

}
