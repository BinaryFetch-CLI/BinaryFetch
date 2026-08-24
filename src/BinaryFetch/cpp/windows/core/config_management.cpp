#include "core/config_management.h"
#include <windows.h>
#include <direct.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>

ConfigManager::ConfigManager(bool devMode) {
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
        configPath = "src\\BinaryFetch\\resources\\Default_JSON_theme_windows_RC\\Default_BinaryFetch_Config.json";
        std::ifstream devCheck(configPath);
        if (!devCheck.good()) {
            std::cerr << "Warning: Could not find development configuration JSON file at: " << configPath << std::endl;
            m_loaded = false;
            return;
        }
    } else {
        configPath = userConfigPath;
        if (GetFileAttributesA(configDir.c_str()) == INVALID_FILE_ATTRIBUTES) {
            _mkdir(configDir.c_str());
        }
        std::ifstream checkConfig(userConfigPath);
        bool userConfigExists = checkConfig.good();
        checkConfig.close();

        if (!userConfigExists) {
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
        }
    }

    std::ifstream configFile(configPath);
    if (!configFile.is_open()) {
        std::cerr << "Warning: Cannot open configuration file: " << configPath << std::endl;
        m_loaded = false;
        return;
    }

    try {
        configFile >> m_config;
        if (m_config.is_object() && m_config.empty()) {
            m_loaded = false;
        } else {
            m_loaded = true;
        }
    } catch (...) {
        m_loaded = false;
    }
}

bool ConfigManager::isLoaded() const { return m_loaded; }

// ===================== RESOLVE SECTION KEY =====================
std::string ConfigManager::resolveSectionKey(const std::string& section) const {
    if (m_config.contains(section)) return section;

    static const std::unordered_map<std::string, std::string> aliases = {
        {"compact_os", "compact_operating_system"},
        {"compact_cpu", "compact_processor"},
        {"compact_gpu", "compact_graphics_card"},
        {"compact_screen", "compact_display_monitor"},
        {"compact_memory", "compact_system_memory"},
        {"compact_audio", "compact_audio_devices"},
        {"compact_performance", "compact_resource_usage"},
        {"compact_user", "compact_user_account"},
        {"compact_network", "compact_network_connection"},
        {"compact_disk", "compact_disk_storage"},
        {"compact_time", "compact_date_and_time"},
        {"detailed_memory", "detailed_system_memory"},
        {"detailed_storage", "detailed_disk_storage"},
        {"network_info", "detailed_network_connection"},
        {"dummy_network_info", "detailed_dummy_network_connection"},
        {"os_info", "detailed_operating_system"},
        {"cpu_info", "detailed_processor"},
        {"gpu_info", "detailed_graphics_card"},
        {"display_info", "detailed_display_monitor"},
        {"bios_mb_info", "detailed_bios_and_motherboard"},
        {"user_info", "detailed_user_account"},
        {"performance_info", "detailed_resource_usage"},
        {"audio_power_info", "detailed_audio_and_power"},
        {"header", "header_settings"},
        {"date_and_time", "compact_date_and_time"},
        {"operating_system", "compact_operating_system"},
        {"processor", "compact_processor"},
        {"graphics_card", "compact_graphics_card"},
        {"display_monitor", "compact_display_monitor"},
        {"system_memory", "compact_system_memory"},
        {"audio_devices", "compact_audio_devices"},
        {"resource_usage", "compact_resource_usage"},
        {"user_account", "compact_user_account"},
        {"network_connection", "compact_network_connection"},
        {"disk_storage", "compact_disk_storage"},
        {"memory_details", "detailed_system_memory"},
        {"storage_details", "detailed_disk_storage"},
        {"network_details", "detailed_network_connection"},
        {"operating_system_details", "detailed_operating_system"},
        {"processor_details", "detailed_processor"},
        {"graphics_details", "detailed_graphics_card"},
        {"display_details", "detailed_display_monitor"},
        {"bios_and_motherboard", "detailed_bios_and_motherboard"},
        {"user_details", "detailed_user_account"},
        {"performance_monitor", "detailed_resource_usage"},
        {"audio_and_power", "detailed_audio_and_power"}
    };

    auto it = aliases.find(section);
    if (it != aliases.end() && m_config.contains(it->second)) {
        return it->second;
    }
    return section;
}

std::string ConfigManager::resolveSubsectionKey(const std::string& module, const std::string& rawSubsection) const {
    if (!m_config.contains(module)) return rawSubsection;
    if (m_config[module].contains(rawSubsection)) return rawSubsection;

    static const std::unordered_map<std::string, std::string> subAliases = {
        {"time_section", "time"}, {"time", "time_section"},
        {"date_section", "date"}, {"date", "date_section"},
        {"week_section", "week"}, {"week", "week_section"},
        {"leap_section", "leap_year"}, {"leap_year", "leap_section"}
    };

    auto it = subAliases.find(rawSubsection);
    if (it != subAliases.end() && m_config[module].contains(it->second)) {
        return it->second;
    }
    return rawSubsection;
}

std::string ConfigManager::resolveColor(const std::string& colorName, const std::string& defaultColor) const {
    auto it = m_colors.find(colorName);
    if (it != m_colors.end()) return it->second;
    auto defIt = m_colors.find(defaultColor);
    if (defIt != m_colors.end()) return defIt->second;
    auto whiteIt = m_colors.find("white");
    return (whiteIt != m_colors.end()) ? whiteIt->second : "\033[37m";
}

// ===================== ENABLED CHECKS =====================
bool ConfigManager::isEnabled(const std::string& rawSection) const {
    if (!m_loaded) return false;
    std::string section = resolveSectionKey(rawSection);
    if (!m_config.contains(section)) return true;
    return m_config[section].value("enabled", true);
}

bool ConfigManager::isFieldEnabled(const std::string& rawSection, const std::string& fieldPath) const {
    if (!isEnabled(rawSection)) return false;
    return getNestedBool(rawSection, fieldPath, true);
}

bool ConfigManager::isSubEnabled(const std::string& rawSection, const std::string& key) const {
    std::string section = resolveSectionKey(rawSection);
    if (!m_loaded || !m_config.contains(section)) return true;
    return m_config[section].value(key, true);
}

bool ConfigManager::isSectionEnabled(const std::string& rawModule, const std::string& section) const {
    std::string module = resolveSectionKey(rawModule);
    if (!m_loaded || !m_config.contains(module)) return true;
    if (!m_config[module].contains("sections")) return true;
    return m_config[module]["sections"].value(section, true);
}

bool ConfigManager::isNestedEnabled(const std::string& rawModule, const std::string& rawSection, const std::string& key) const {
    std::string module = resolveSectionKey(rawModule);
    if (!m_loaded || !m_config.contains(module)) return true;
    std::string section = resolveSubsectionKey(module, rawSection);
    if (!m_config[module].contains(section)) return true;
    return m_config[module][section].value(key, true);
}

bool ConfigManager::getNestedBool(const std::string& rawModule, const std::string& path, bool defaultValue) const {
    std::string module = resolveSectionKey(rawModule);
    if (!m_loaded || !m_config.contains(module)) return defaultValue;

    std::vector<std::string> keys;
    std::stringstream ss(path);
    std::string key;
    while (std::getline(ss, key, '.')) {
        keys.push_back(key);
    }
    if (!keys.empty()) {
        keys[0] = resolveSubsectionKey(module, keys[0]);
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

// ===================== COLOR RESOLUTION =====================
std::string ConfigManager::getColor(const std::string& rawSection, const std::string& key, const std::string& defaultColor) const {
    std::string section = resolveSectionKey(rawSection);
    if (!m_loaded || !m_config.contains(section)) return resolveColor(defaultColor, defaultColor);

    if (key.find('.') != std::string::npos) {
        return getNestedColor(rawSection, key, defaultColor);
    }

    const auto& secObj = m_config[section];

    if (secObj.contains("colors") && secObj["colors"].contains(key)) {
        if (secObj["colors"][key].is_string()) {
            return resolveColor(secObj["colors"][key].get<std::string>(), defaultColor);
        }
    }

    if (secObj.contains(key) && secObj[key].is_string()) {
        return resolveColor(secObj[key].get<std::string>(), defaultColor);
    }

    // Alias fallbacks
    if (key == "item") {
        for (const auto& altKey : {"|->", "~", "#"}) {
            if (secObj.contains("colors") && secObj["colors"].contains(altKey) && secObj["colors"][altKey].is_string()) {
                return resolveColor(secObj["colors"][altKey].get<std::string>(), defaultColor);
            }
            if (secObj.contains(altKey) && secObj[altKey].is_string()) {
                return resolveColor(secObj[altKey].get<std::string>(), defaultColor);
            }
        }
    } else if (key == "item_alt") {
        if (secObj.contains("colors") && secObj["colors"].contains("#->") && secObj["colors"]["#->"].is_string()) {
            return resolveColor(secObj["colors"]["#->"].get<std::string>(), defaultColor);
        }
        if (secObj.contains("#->") && secObj["#->"].is_string()) {
            return resolveColor(secObj["#->"].get<std::string>(), defaultColor);
        }
    } else if (key == "header") {
        for (const auto& altKey : {"#-", ">>~"}) {
            if (secObj.contains("colors") && secObj["colors"].contains(altKey) && secObj["colors"][altKey].is_string()) {
                return resolveColor(secObj["colors"][altKey].get<std::string>(), defaultColor);
            }
            if (secObj.contains(altKey) && secObj[altKey].is_string()) {
                return resolveColor(secObj[altKey].get<std::string>(), defaultColor);
            }
        }
    }

    return resolveColor(defaultColor, defaultColor);
}

std::string ConfigManager::getNestedColor(const std::string& rawModule, const std::string& rawSubsection, const std::string& key, const std::string& defaultColor) const {
    std::string module = resolveSectionKey(rawModule);
    if (!m_loaded || !m_config.contains(module)) return resolveColor(defaultColor, defaultColor);
    std::string subsection = resolveSubsectionKey(module, rawSubsection);
    if (!m_config[module].contains(subsection)) return resolveColor(defaultColor, defaultColor);

    if (m_config[module][subsection].contains("colors") && m_config[module][subsection]["colors"].contains(key)) {
        if (m_config[module][subsection]["colors"][key].is_string()) {
            return resolveColor(m_config[module][subsection]["colors"][key].get<std::string>(), defaultColor);
        }
    }

    if (m_config[module][subsection].contains(key) && m_config[module][subsection][key].is_string()) {
        return resolveColor(m_config[module][subsection][key].get<std::string>(), defaultColor);
    }

    return resolveColor(defaultColor, defaultColor);
}

std::string ConfigManager::getNestedColor(const std::string& rawModule, const std::string& path, const std::string& defaultColor) const {
    std::string module = resolveSectionKey(rawModule);
    if (!m_loaded || !m_config.contains(module)) return resolveColor(defaultColor, defaultColor);

    std::vector<std::string> keys;
    std::stringstream ss(path);
    std::string key;
    while (std::getline(ss, key, '.')) {
        keys.push_back(key);
    }
    if (!keys.empty()) {
        keys[0] = resolveSubsectionKey(module, keys[0]);
    }

    nlohmann::json current = m_config[module];
    for (const auto& k : keys) {
        if (!current.contains(k)) return resolveColor(defaultColor, defaultColor);
        current = current[k];
    }

    if (current.is_string()) {
        return resolveColor(current.get<std::string>(), defaultColor);
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
        return resolveColor(current.get<std::string>(), defaultColor);
    }
    return resolveColor(defaultColor, defaultColor);
}

std::string ConfigManager::getResetColor() const {
    return resolveColor("reset", "reset");
}

// ===================== LABEL RESOLUTION =====================
std::string ConfigManager::getLabel(const std::string& rawSection, const std::string& key, const std::string& defaultLabel) const {
    std::string section = resolveSectionKey(rawSection);
    if (!m_loaded || !m_config.contains(section)) return defaultLabel;

    const auto& secObj = m_config[section];

    if (key.find('.') != std::string::npos) {
        std::vector<std::string> keys;
        std::stringstream ss(key);
        std::string k;
        while (std::getline(ss, k, '.')) keys.push_back(k);

        nlohmann::json current = secObj;
        for (const auto& part : keys) {
            if (!current.contains(part)) return defaultLabel;
            current = current[part];
        }
        if (current.is_string()) return current.get<std::string>();
        return defaultLabel;
    }

    if (secObj.contains("labels") && secObj["labels"].contains(key)) {
        if (secObj["labels"][key].is_string()) {
            return secObj["labels"][key].get<std::string>();
        }
    }

    if (secObj.contains(key) && secObj[key].is_string()) {
        return secObj[key].get<std::string>();
    }

    return defaultLabel;
}

std::string ConfigManager::getNestedLabel(const std::string& rawModule, const std::string& rawSection, const std::string& key, const std::string& defaultLabel) const {
    std::string module = resolveSectionKey(rawModule);
    if (!m_loaded || !m_config.contains(module)) return defaultLabel;
    std::string section = resolveSubsectionKey(module, rawSection);
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

// ===================== PREFIX RESOLUTION =====================
std::string ConfigManager::getPrefix(const std::string& rawSection, const std::string& key, const std::string& defaultPrefix) const {
    std::string section = resolveSectionKey(rawSection);
    if (!m_loaded || !m_config.contains(section)) return defaultPrefix;

    const auto& secObj = m_config[section];

    if (key.find('.') != std::string::npos) {
        std::vector<std::string> keys;
        std::stringstream ss(key);
        std::string k;
        while (std::getline(ss, k, '.')) keys.push_back(k);

        nlohmann::json current = secObj;
        for (const auto& part : keys) {
            if (!current.contains(part)) return defaultPrefix;
            current = current[part];
        }
        if (current.is_string()) return current.get<std::string>();
        return defaultPrefix;
    }

    if (secObj.contains("prefixes") && secObj["prefixes"].contains(key)) {
        if (secObj["prefixes"][key].is_string()) {
            return secObj["prefixes"][key].get<std::string>();
        }
    }

    if (secObj.contains("labels") && secObj["labels"].contains(key)) {
        if (secObj["labels"][key].is_string()) {
            return secObj["labels"][key].get<std::string>();
        }
    }

    if (secObj.contains(key) && secObj[key].is_string()) {
        return secObj[key].get<std::string>();
    }

    if (key == "item") {
        for (const auto& altKey : {"|->", "~", "#"}) {
            if (secObj.contains("prefixes") && secObj["prefixes"].contains(altKey) && secObj["prefixes"][altKey].is_string()) {
                return secObj["prefixes"][altKey].get<std::string>();
            }
            if (secObj.contains(altKey) && secObj[altKey].is_string()) {
                return secObj[altKey].get<std::string>();
            }
        }
    } else if (key == "item_alt") {
        if (secObj.contains("prefixes") && secObj["prefixes"].contains("#->") && secObj["prefixes"]["#->"].is_string()) {
            return secObj["prefixes"]["#->"].get<std::string>();
        }
        if (secObj.contains("#->") && secObj["#->"].is_string()) {
            return secObj["#->"].get<std::string>();
        }
    } else if (key == "header") {
        for (const auto& altKey : {"#-", ">>~"}) {
            if (secObj.contains("prefixes") && secObj["prefixes"].contains(altKey) && secObj["prefixes"][altKey].is_string()) {
                return secObj["prefixes"][altKey].get<std::string>();
            }
            if (secObj.contains(altKey) && secObj[altKey].is_string()) {
                return secObj[altKey].get<std::string>();
            }
        }
    }

    return defaultPrefix;
}

std::string ConfigManager::getNestedPrefix(const std::string& rawModule, const std::string& rawSection, const std::string& key, const std::string& defaultPrefix) const {
    std::string module = resolveSectionKey(rawModule);
    if (!m_loaded || !m_config.contains(module)) return defaultPrefix;
    std::string section = resolveSubsectionKey(module, rawSection);
    if (!m_config[module].contains(section)) return defaultPrefix;

    if (m_config[module][section].contains("prefixes") && m_config[module][section]["prefixes"].contains(key)) {
        if (m_config[module][section]["prefixes"][key].is_string()) {
            return m_config[module][section]["prefixes"][key].get<std::string>();
        }
    }

    if (m_config[module][section].contains("labels") && m_config[module][section]["labels"].contains(key)) {
        if (m_config[module][section]["labels"][key].is_string()) {
            return m_config[module][section]["labels"][key].get<std::string>();
        }
    }

    if (m_config[module][section].contains(key) && m_config[module][section][key].is_string()) {
        return m_config[module][section][key].get<std::string>();
    }

    if (key == "item") {
        for (const auto& altKey : {"|->", "~", "#"}) {
            if (m_config[module][section].contains("prefixes") && m_config[module][section]["prefixes"].contains(altKey) && m_config[module][section]["prefixes"][altKey].is_string()) {
                return m_config[module][section]["prefixes"][altKey].get<std::string>();
            }
            if (m_config[module][section].contains(altKey) && m_config[module][section][key].is_string()) {
                return m_config[module][section][altKey].get<std::string>();
            }
        }
    } else if (key == "header") {
        for (const auto& altKey : {"#-", ">>~"}) {
            if (m_config[module][section].contains("prefixes") && m_config[module][section]["prefixes"].contains(altKey) && m_config[module][section]["prefixes"][altKey].is_string()) {
                return m_config[module][section]["prefixes"][altKey].get<std::string>();
            }
            if (m_config[module][section].contains(altKey) && m_config[module][section][altKey].is_string()) {
                return m_config[module][section][altKey].get<std::string>();
            }
        }
    }

    return defaultPrefix;
}

const nlohmann::json& ConfigManager::getJson() const {
    return m_config;
}