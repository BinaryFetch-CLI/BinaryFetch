#include "core/config_management.h"
#include <windows.h>
#include <direct.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>

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
        // DEV MODE: load directly from the project folder for fast iteration.
        // Run from the project root (e.g. via the VS debugger's working directory).
        configPath = "src\\BinaryFetch\\resources\\Default_JSON_theme_windows_RC\\Default_BinaryFetch_Config.json";

        std::ifstream devCheck(configPath);
        if (!devCheck.good()) {
            std::cerr << "Warning: Could not find development configuration JSON file at: "
                       << configPath << std::endl;
            m_loaded = false;
            return;
        }
    }
    else {
        // PRODUCTION MODE: use the constant public folder, self-healing from
        // a resource embedded in the EXE itself. No filesystem guessing —
        // this works no matter where the exe is installed or launched from.
        configPath = userConfigPath;

        // 1. Create the directory if it doesn't exist
        if (GetFileAttributesA(configDir.c_str()) == INVALID_FILE_ATTRIBUTES) {
            _mkdir(configDir.c_str());
        }

        // 2. Self-healing: if the user config is missing, extract the default
        //    from the EXE's embedded RCDATA resource (IDR_DEFAULT_CONFIG = 101).
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
                else {
                    std::cerr << "Warning: Cannot write default configuration to " << userConfigPath << std::endl;
                }
            }
            else {
                std::cerr << "Warning: Internal resource IDR_DEFAULT_CONFIG not found." << std::endl;
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
        // Treat an empty JSON object {} as "not configured" — same effect as a missing file.
        // This means clearing the config file to {} or empty will show only ASCII art,
        // consistent with the behaviour of DEV_MODE when the source JSON is blank.
        if (m_config.is_object() && m_config.empty()) {
            std::cerr << "Warning: Configuration file is empty ({}). Treating as unconfigured." << std::endl;
            m_loaded = false;
        } else {
            m_loaded = true;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Warning: Failed to parse configuration JSON (" << e.what() << ")." << std::endl;
        m_loaded = false;
    }
    catch (...) {
        std::cerr << "Warning: Failed to parse configuration JSON." << std::endl;
        m_loaded = false;
    }
}

bool ConfigManager::isLoaded() const {
    return m_loaded;
}

std::string ConfigManager::resolveSectionKey(const std::string& section) const {
    if (m_config.contains(section)) return section;

    static const std::unordered_map<std::string, std::string> aliases = {
        {"header", "header_settings"},
        {"header_settings", "header"},
        {"compact_time", "date_and_time"},
        {"date_and_time", "compact_time"},
        {"compact_os", "operating_system"},
        {"operating_system", "compact_os"},
        {"compact_cpu", "processor"},
        {"processor", "compact_cpu"},
        {"compact_gpu", "graphics_card"},
        {"graphics_card", "compact_gpu"},
        {"compact_screen", "display_monitor"},
        {"display_monitor", "compact_screen"},
        {"compact_memory", "system_memory"},
        {"system_memory", "compact_memory"},
        {"compact_audio", "audio_devices"},
        {"audio_devices", "compact_audio"},
        {"compact_performance", "resource_usage"},
        {"resource_usage", "compact_performance"},
        {"compact_user", "user_account"},
        {"user_account", "compact_user"},
        {"compact_network", "network_connection"},
        {"network_connection", "compact_network"},
        {"compact_disk", "disk_storage"},
        {"disk_storage", "compact_disk"},
        {"detailed_memory", "memory_details"},
        {"memory_details", "detailed_memory"},
        {"detailed_storage", "storage_details"},
        {"storage_details", "detailed_storage"},
        {"network_info", "network_details"},
        {"network_details", "network_info"},
        {"os_info", "operating_system_details"},
        {"operating_system_details", "os_info"},
        {"cpu_info", "processor_details"},
        {"processor_details", "cpu_info"},
        {"gpu_info", "graphics_details"},
        {"graphics_details", "gpu_info"},
        {"display_info", "display_details"},
        {"display_details", "display_info"},
        {"bios_mb_info", "bios_and_motherboard"},
        {"bios_and_motherboard", "bios_mb_info"},
        {"user_info", "user_details"},
        {"user_details", "user_info"},
        {"performance_info", "performance_monitor"},
        {"performance_monitor", "performance_info"},
        {"audio_power_info", "audio_and_power"},
        {"audio_and_power", "audio_power_info"}
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
        {"time_section", "time"},
        {"time", "time_section"},
        {"date_section", "date"},
        {"date", "date_section"},
        {"week_section", "week"},
        {"week", "week_section"},
        {"leap_section", "leap_year"},
        {"leap_year", "leap_section"}
    };

    auto it = subAliases.find(rawSubsection);
    if (it != subAliases.end() && m_config[module].contains(it->second)) {
        return it->second;
    }
    return rawSubsection;
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

bool ConfigManager::isEnabled(const std::string& rawSection) const {
    // If config failed to load (missing file, empty {}, parse error) → show nothing.
    // Only the ASCII art renders in this state, which is the expected fail-safe.
    if (!m_loaded) return false;
    std::string section = resolveSectionKey(rawSection);
    // Section missing from a *partial* config → default ON (backward-compatible with older configs).
    if (!m_config.contains(section)) return true;
    return m_config[section].value("enabled", true);
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

std::string ConfigManager::getColor(const std::string& rawSection, const std::string& key, const std::string& defaultColor) const {
    std::string section = resolveSectionKey(rawSection);
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

    // Alias fallbacks for colors
    if (key == "item") {
        for (const auto& altKey : {"|->", "~", "#"}) {
            if (m_config[section].contains("colors") && m_config[section]["colors"].contains(altKey) && m_config[section]["colors"][altKey].is_string()) {
                return resolveColor(m_config[section]["colors"][altKey].get<std::string>(), defaultColor);
            }
            if (m_config[section].contains(altKey) && m_config[section][altKey].is_string()) {
                return resolveColor(m_config[section][altKey].get<std::string>(), defaultColor);
            }
        }
    } else if (key == "item_alt") {
        if (m_config[section].contains("colors") && m_config[section]["colors"].contains("#->") && m_config[section]["colors"]["#->"].is_string()) {
            return resolveColor(m_config[section]["colors"]["#->"].get<std::string>(), defaultColor);
        }
        if (m_config[section].contains("#->") && m_config[section]["#->"].is_string()) {
            return resolveColor(m_config[section]["#->"].get<std::string>(), defaultColor);
        }
    } else if (key == "header") {
        for (const auto& altKey : {"#-", ">>~"}) {
            if (m_config[section].contains("colors") && m_config[section]["colors"].contains(altKey) && m_config[section]["colors"][altKey].is_string()) {
                return resolveColor(m_config[section]["colors"][altKey].get<std::string>(), defaultColor);
            }
            if (m_config[section].contains(altKey) && m_config[section][altKey].is_string()) {
                return resolveColor(m_config[section][altKey].get<std::string>(), defaultColor);
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
            std::string colorName = m_config[module][subsection]["colors"][key].get<std::string>();
            return resolveColor(colorName, defaultColor);
        }
    }

    if (m_config[module][subsection].contains(key) && m_config[module][subsection][key].is_string()) {
        std::string colorName = m_config[module][subsection][key].get<std::string>();
        return resolveColor(colorName, defaultColor);
    }

    // Alias fallbacks for nested colors
    if (key == "item") {
        for (const auto& altKey : {"|->", "~", "#"}) {
            if (m_config[module][subsection].contains("colors") && m_config[module][subsection]["colors"].contains(altKey) && m_config[module][subsection]["colors"][altKey].is_string()) {
                return resolveColor(m_config[module][subsection]["colors"][altKey].get<std::string>(), defaultColor);
            }
            if (m_config[module][subsection].contains(altKey) && m_config[module][subsection][altKey].is_string()) {
                return resolveColor(m_config[module][subsection][altKey].get<std::string>(), defaultColor);
            }
        }
    } else if (key == "header") {
        for (const auto& altKey : {"#-", ">>~"}) {
            if (m_config[module][subsection].contains("colors") && m_config[module][subsection]["colors"].contains(altKey) && m_config[module][subsection]["colors"][altKey].is_string()) {
                return resolveColor(m_config[module][subsection]["colors"][altKey].get<std::string>(), defaultColor);
            }
            if (m_config[module][subsection].contains(altKey) && m_config[module][subsection][altKey].is_string()) {
                return resolveColor(m_config[module][subsection][altKey].get<std::string>(), defaultColor);
            }
        }
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

std::string ConfigManager::getResetColor() const {
    return resolveColor("reset", "reset");
}

std::string ConfigManager::getLabel(const std::string& rawSection, const std::string& key, const std::string& defaultLabel) const {
    std::string section = resolveSectionKey(rawSection);
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

std::string ConfigManager::getPrefix(const std::string& rawSection, const std::string& key, const std::string& defaultPrefix) const {
    std::string section = resolveSectionKey(rawSection);
    if (!m_loaded || !m_config.contains(section)) return defaultPrefix;

    if (m_config[section].contains("prefixes") && m_config[section]["prefixes"].contains(key)) {
        if (m_config[section]["prefixes"][key].is_string()) {
            return m_config[section]["prefixes"][key].get<std::string>();
        }
    }

    if (m_config[section].contains("labels") && m_config[section]["labels"].contains(key)) {
        if (m_config[section]["labels"][key].is_string()) {
            return m_config[section]["labels"][key].get<std::string>();
        }
    }

    if (m_config[section].contains(key) && m_config[section][key].is_string()) {
        return m_config[section][key].get<std::string>();
    }

    // Alias fallbacks for prefixes
    if (key == "item") {
        for (const auto& altKey : {"|->", "~", "#"}) {
            if (m_config[section].contains("prefixes") && m_config[section]["prefixes"].contains(altKey) && m_config[section]["prefixes"][altKey].is_string()) {
                return m_config[section]["prefixes"][altKey].get<std::string>();
            }
            if (m_config[section].contains(altKey) && m_config[section][altKey].is_string()) {
                return m_config[section][altKey].get<std::string>();
            }
        }
    } else if (key == "item_alt") {
        if (m_config[section].contains("prefixes") && m_config[section]["prefixes"].contains("#->") && m_config[section]["prefixes"]["#->"].is_string()) {
            return m_config[section]["prefixes"]["#->"].get<std::string>();
        }
        if (m_config[section].contains("#->") && m_config[section]["#->"].is_string()) {
            return m_config[section]["#->"].get<std::string>();
        }
    } else if (key == "header") {
        for (const auto& altKey : {"#-", ">>~"}) {
            if (m_config[section].contains("prefixes") && m_config[section]["prefixes"].contains(altKey) && m_config[section]["prefixes"][altKey].is_string()) {
                return m_config[section]["prefixes"][altKey].get<std::string>();
            }
            if (m_config[section].contains(altKey) && m_config[section][altKey].is_string()) {
                return m_config[section][altKey].get<std::string>();
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

    // Alias fallbacks for nested prefixes
    if (key == "item") {
        for (const auto& altKey : {"|->", "~", "#"}) {
            if (m_config[module][section].contains("prefixes") && m_config[module][section]["prefixes"].contains(altKey) && m_config[module][section]["prefixes"][altKey].is_string()) {
                return m_config[module][section]["prefixes"][altKey].get<std::string>();
            }
            if (m_config[module][section].contains(altKey) && m_config[module][section][altKey].is_string()) {
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
