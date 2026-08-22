#include "core/config_management.h"
#include <windows.h>
#include <direct.h>
#include <fstream>
#include <iostream>
#include <sstream>

ConfigManager::ConfigManager(bool devMode) {
    // Initialize color map for ANSI escape codes
    m_colors = {
        {"red", "\033[31m"}, {"green", "\033[32m"}, {"yellow", "\033[33m"},
        {"blue", "\033[34m"}, {"magenta", "\033[35m"}, {"cyan", "\033[36m"},
        {"white", "\033[37m"}, {"bright_red", "\033[91m"}, {"bright_green", "\033[92m"},
        {"bright_yellow", "\033[93m"}, {"bright_blue", "\033[94m"},
        {"bright_magenta", "\033[95m"}, {"bright_cyan", "\033[96m"},
        {"bright_white", "\033[97m"}, {"reset", "\033[0m"}
    };

    loadPlatformConfig(devMode);
}

void ConfigManager::loadPlatformConfig(bool devMode) {
    std::string configDir = "C:\\Users\\Public\\BinaryFetch";
    std::string userConfigPath = configDir + "\\BinaryFetch_Config.json";
    std::string configPath;

    if (devMode) {
        // DEV MODE: Load directly from project folder
        configPath = "resources\\Default_BinaryFetch_Config.json";
    }
    else {
        // PRODUCTION MODE: Use constant public folder
        configPath = userConfigPath;

        // 1. Create directory if it doesn't exist
        if (GetFileAttributesA(configDir.c_str()) == INVALID_FILE_ATTRIBUTES) {
            _mkdir(configDir.c_str());
        }

        // 2. Self-Healing: Check if user config exists, if not, extract from EXE memory (Resource 101)
        std::ifstream checkConfig(userConfigPath);
        bool userConfigExists = checkConfig.good();
        checkConfig.close();

        if (!userConfigExists) {
            // IDR_DEFAULT_CONFIG is 101
            HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(101), RT_RCDATA);
            if (hRes) {
                HGLOBAL hData = LoadResource(NULL, hRes);
                DWORD size = SizeofResource(NULL, hRes);
                const char* data = static_cast<const char*>(LockResource(hData));

                std::ofstream userConfig(userConfigPath, std::ios::binary);
                if (userConfig.is_open()) {
                    userConfig.write(data, size);
                    userConfig.close();
                }
            }
            else {
                std::cout << "Warning: Internal resource IDR_DEFAULT_CONFIG not found." << std::endl;
            }
        }
    }

    m_loaded = false;
    std::ifstream config_file(configPath);
    if (config_file.is_open()) {
        try {
            m_config = nlohmann::json::parse(config_file);
            m_loaded = true;
        }
        catch (const std::exception& e) {
            std::cout << "Warning: Failed to parse config file. Using hardcoded defaults." << std::endl;
        }
        config_file.close();
    }
    else {
        std::cout << "Warning: Could not open config file: " << configPath << std::endl;
    }
}

bool ConfigManager::isLoaded() const {
    return m_loaded;
}

const nlohmann::json& ConfigManager::getJson() const {
    return m_config;
}

std::string ConfigManager::getResetColor() const {
    auto it = m_colors.find("reset");
    return (it != m_colors.end()) ? it->second : "\033[0m";
}

std::string ConfigManager::resolveColor(const std::string& colorName, const std::string& defaultColor) const {
    auto it = m_colors.find(colorName);
    if (it != m_colors.end()) {
        return it->second;
    }
    auto defIt = m_colors.find(defaultColor);
    if (defIt != m_colors.end()) {
        return defIt->second;
    }
    auto whiteIt = m_colors.find("white");
    return (whiteIt != m_colors.end()) ? whiteIt->second : "\033[37m";
}

bool ConfigManager::isEnabled(const std::string& section) const {
    if (!m_loaded || !m_config.contains(section)) return true;
    return m_config[section].value("enabled", true);
}

bool ConfigManager::isSubEnabled(const std::string& section, const std::string& key) const {
    if (!m_loaded || !m_config.contains(section)) return true;
    return m_config[section].value(key, true);
}

bool ConfigManager::isSectionEnabled(const std::string& module, const std::string& section) const {
    if (!m_loaded || !m_config.contains(module)) return true;
    if (!m_config[module].contains("sections")) return true;
    return m_config[module]["sections"].value(section, true);
}

bool ConfigManager::isNestedEnabled(const std::string& module, const std::string& section, const std::string& key) const {
    if (!m_loaded || !m_config.contains(module)) return true;
    if (!m_config[module].contains(section)) return true;
    return m_config[module][section].value(key, true);
}

bool ConfigManager::getNestedBool(const std::string& module, const std::string& path, bool defaultValue) const {
    if (!m_loaded || !m_config.contains(module)) return defaultValue;

    std::vector<std::string> keys;
    std::stringstream ss(path);
    std::string key;
    while (std::getline(ss, key, '.')) {
        keys.push_back(key);
    }

    nlohmann::json current = m_config[module];
    for (const auto& k : keys) {
        if (!current.contains(k)) return defaultValue;
        current = current[k];
    }

    if (current.is_boolean()) {
        return current.get<bool>();
    }
    return defaultValue;
}

bool ConfigManager::getNestedBool(const std::string& path, bool defaultValue) const {
    if (!m_loaded) return defaultValue;

    std::vector<std::string> keys;
    std::stringstream ss(path);
    std::string key;
    while (std::getline(ss, key, '.')) {
        keys.push_back(key);
    }

    nlohmann::json current = m_config;
    for (const auto& k : keys) {
        if (!current.contains(k)) return defaultValue;
        current = current[k];
    }

    if (current.is_boolean()) {
        return current.get<bool>();
    }
    return defaultValue;
}

std::string ConfigManager::getColor(const std::string& section, const std::string& key, const std::string& defaultColor) const {
    if (!m_loaded || !m_config.contains(section)) return resolveColor(defaultColor, defaultColor);

    if (m_config[section].contains("colors") && m_config[section]["colors"].contains(key)) {
        if (m_config[section]["colors"][key].is_string()) {
            std::string colorName = m_config[section]["colors"][key].get<std::string>();
            return resolveColor(colorName, defaultColor);
        }
    }

    if (m_config[section].contains(key) && m_config[section][key].is_string()) {
        std::string colorName = m_config[section][key].get<std::string>();
        return resolveColor(colorName, defaultColor);
    }

    return resolveColor(defaultColor, defaultColor);
}

std::string ConfigManager::getNestedColor(const std::string& module, const std::string& subsection, const std::string& key, const std::string& defaultColor) const {
    if (!m_loaded || !m_config.contains(module)) return resolveColor(defaultColor, defaultColor);
    if (!m_config[module].contains(subsection)) return resolveColor(defaultColor, defaultColor);

    if (m_config[module][subsection].contains("colors") && m_config[module][subsection]["colors"].contains(key)) {
        if (m_config[module][subsection]["colors"][key].is_string()) {
            std::string colorName = m_config[module][subsection]["colors"][key].get<std::string>();
            return resolveColor(colorName, defaultColor);
        }
    }

    if (m_config[module][subsection].contains(key) && m_config[module][subsection][key].is_string()) {
        std::string colorName = m_config[module][subsection][key].get<std::string>();
        return resolveColor(colorName, defaultColor);
    }

    return resolveColor(defaultColor, defaultColor);
}

std::string ConfigManager::getNestedColor(const std::string& module, const std::string& path, const std::string& defaultColor) const {
    if (!m_loaded || !m_config.contains(module)) return resolveColor(defaultColor, defaultColor);

    std::vector<std::string> keys;
    std::stringstream ss(path);
    std::string key;
    while (std::getline(ss, key, '.')) {
        keys.push_back(key);
    }

    nlohmann::json current = m_config[module];
    for (const auto& k : keys) {
        if (!current.contains(k)) return resolveColor(defaultColor, defaultColor);
        current = current[k];
    }

    if (current.is_string()) {
        std::string colorName = current.get<std::string>();
        return resolveColor(colorName, defaultColor);
    }
    return resolveColor(defaultColor, defaultColor);
}

std::string ConfigManager::getNestedColor(const std::string& path, const std::string& defaultColor) const {
    if (!m_loaded) return resolveColor(defaultColor, defaultColor);

    std::vector<std::string> keys;
    std::stringstream ss(path);
    std::string key;
    while (std::getline(ss, key, '.')) {
        keys.push_back(key);
    }

    nlohmann::json current = m_config;
    for (const auto& k : keys) {
        if (!current.contains(k)) return resolveColor(defaultColor, defaultColor);
        current = current[k];
    }

    if (current.is_string()) {
        std::string colorName = current.get<std::string>();
        return resolveColor(colorName, defaultColor);
    }
    return resolveColor(defaultColor, defaultColor);
}

std::string ConfigManager::getLabel(const std::string& section, const std::string& key, const std::string& defaultLabel) const {
    if (!m_loaded || !m_config.contains(section)) return defaultLabel;

    if (m_config[section].contains("labels") && m_config[section]["labels"].contains(key)) {
        if (m_config[section]["labels"][key].is_string()) {
            return m_config[section]["labels"][key].get<std::string>();
        }
    }

    if (m_config[section].contains(key) && m_config[section][key].is_string()) {
        return m_config[section][key].get<std::string>();
    }

    return defaultLabel;
}

std::string ConfigManager::getNestedLabel(const std::string& module, const std::string& section, const std::string& key, const std::string& defaultLabel) const {
    if (!m_loaded || !m_config.contains(module)) return defaultLabel;
    if (!m_config[module].contains(section)) return defaultLabel;

    if (m_config[module][section].contains("labels") && m_config[module][section]["labels"].contains(key)) {
        if (m_config[module][section]["labels"][key].is_string()) {
            return m_config[module][section]["labels"][key].get<std::string>();
        }
    }

    if (m_config[module][section].contains(key) && m_config[module][section][key].is_string()) {
        return m_config[module][section][key].get<std::string>();
    }

    return defaultLabel;
}
