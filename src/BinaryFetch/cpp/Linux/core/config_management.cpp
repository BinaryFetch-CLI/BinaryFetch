// Dummy/stub implementation of ConfigManager, matching config_management.h exactly.
// Purpose: give you something that compiles and links so you can smoke-test the
// rest of BinaryFetch's build while you fix the real implementation.
//
// NOTE on your original error: your .cpp defined
//     ConfigManager::ConfigManager(bool devMode)
//     void ConfigManager::loadPlatformConfig(bool devMode)
// but the header only declares:
//     explicit ConfigManager(ConfigMode mode = ConfigMode::Production);
//     void loadPlatformConfig(ConfigMode mode);
// There is no bool-taking overload in this header, hence "no declaration matches".
// This stub uses ConfigMode everywhere, as the header requires.

#include "core/config_management.h"
#include <iostream>

// ---- Constructor ----------------------------------------------------------

ConfigManager::ConfigManager(ConfigMode mode) {
    loadPlatformConfig(mode);
    loadColorPalette();
    loadAsciiColorPrefixes();
    loadEmojiSettings();
}

// ---- Loaded state -----------------------------------------------------------

bool ConfigManager::isLoaded() const {
    return m_loaded;
}

// ---- Enabled checks ---------------------------------------------------------

bool ConfigManager::isEnabled(const std::string& section) const {
    (void)section;
    return true;
}

bool ConfigManager::isFieldEnabled(const std::string& section, const std::string& fieldPath) const {
    (void)section; (void)fieldPath;
    return true;
}

bool ConfigManager::isSubEnabled(const std::string& section, const std::string& key) const {
    (void)section; (void)key;
    return true;
}

bool ConfigManager::isSectionEnabled(const std::string& module, const std::string& section) const {
    (void)module; (void)section;
    return true;
}

bool ConfigManager::isNestedEnabled(const std::string& module, const std::string& section, const std::string& key) const {
    (void)module; (void)section; (void)key;
    return true;
}

bool ConfigManager::getNestedBool(const std::string& module, const std::string& path, bool defaultValue) const {
    (void)module; (void)path;
    return defaultValue;
}

bool ConfigManager::getNestedBool(const std::string& path, bool defaultValue) const {
    (void)path;
    return defaultValue;
}

int ConfigManager::getNestedInt(const std::string& module, const std::string& path, int defaultValue) const {
    (void)module; (void)path;
    return defaultValue;
}

std::string ConfigManager::getNestedString(const std::string& module, const std::string& path, const std::string& defaultValue) const {
    return getNestedStringRaw(module, path, defaultValue);
}

// ---- Color resolution ---------------------------------------------------------

std::string ConfigManager::getColor(const std::string& section, const std::string& key, const std::string& defaultColor) const {
    (void)section; (void)key;
    return resolveColor(defaultColor, defaultColor);
}

std::string ConfigManager::getNestedColor(const std::string& module, const std::string& subsection, const std::string& key, const std::string& defaultColor) const {
    (void)module; (void)subsection; (void)key;
    return resolveColor(defaultColor, defaultColor);
}

std::string ConfigManager::getNestedColor(const std::string& module, const std::string& path, const std::string& defaultColor) const {
    (void)module; (void)path;
    return resolveColor(defaultColor, defaultColor);
}

std::string ConfigManager::getNestedColor(const std::string& path, const std::string& defaultColor) const {
    (void)path;
    return resolveColor(defaultColor, defaultColor);
}

std::string ConfigManager::getResetColor() const {
    return "\033[0m";
}

// ---- Label / Prefix resolution ---------------------------------------------------------

std::string ConfigManager::getLabel(const std::string& section, const std::string& key, const std::string& defaultLabel) const {
    return applyEmojiStyle(getLabelRaw(section, key, defaultLabel));
}

std::string ConfigManager::getNestedLabel(const std::string& module, const std::string& section, const std::string& key, const std::string& defaultLabel) const {
    return applyEmojiStyle(getNestedLabelRaw(module, section, key, defaultLabel));
}

std::string ConfigManager::getPrefix(const std::string& section, const std::string& key, const std::string& defaultPrefix) const {
    return applyEmojiStyle(getPrefixRaw(section, key, defaultPrefix));
}

std::string ConfigManager::getNestedPrefix(const std::string& module, const std::string& section, const std::string& key, const std::string& defaultPrefix) const {
    return applyEmojiStyle(getNestedPrefixRaw(module, section, key, defaultPrefix));
}

// ---- String array resolution ---------------------------------------------------------

std::vector<std::string> ConfigManager::getStringArray(
    const std::string& module,
    const std::string& path,
    const std::vector<std::string>& fallback) const {
    (void)module; (void)path;
    return fallback;
}

std::vector<std::string> ConfigManager::getLayoutOrder() const {
    return {"header", "body", "footer"};
}

// ---- Ascii color map ---------------------------------------------------------

const std::map<int, std::string>& ConfigManager::getAsciiColorMap() const {
    return m_asciiColorMap;
}

bool ConfigManager::isAsciiShowColorsEnabled() const {
    return m_asciiShowColors;
}

// ---- Raw JSON access ---------------------------------------------------------

const nlohmann::json& ConfigManager::getJson() const {
    return m_config;
}

// ---- Private helpers ---------------------------------------------------------

void ConfigManager::loadPlatformConfig(ConfigMode mode) {
    switch (mode) {
        case ConfigMode::Dev:
            std::cout << "[ConfigManager] (dummy) loading Dev config\n";
            break;
        case ConfigMode::ReleaseSource:
            std::cout << "[ConfigManager] (dummy) loading ReleaseSource config\n";
            break;
        case ConfigMode::Production:
        default:
            std::cout << "[ConfigManager] (dummy) loading Production config\n";
            break;
    }
    m_config = nlohmann::json::object();
    m_loaded = true;
}

std::string ConfigManager::resolveSectionKey(const std::string& section) const {
    return section;
}

std::string ConfigManager::resolveSubsectionKey(const std::string& module, const std::string& subsection) const {
    return module + "." + subsection;
}

std::string ConfigManager::resolveColor(const std::string& colorName, const std::string& defaultColor) const {
    auto it = m_colors.find(colorName);
    if (it != m_colors.end()) {
        return it->second;
    }
    return defaultColor;
}

std::string ConfigManager::parseColorValue(const std::string& raw) const {
    // Dummy passthrough — real implementation would parse hex/rgb/raw ANSI here.
    return raw;
}

void ConfigManager::loadColorPalette() {
    m_colors["white"] = "\033[37m";
    m_colors["red"]   = "\033[31m";
    m_colors["green"] = "\033[32m";
}

void ConfigManager::loadAsciiColorPrefixes() {
    m_asciiColorMap[1] = "\033[31m";
    m_asciiColorMap[2] = "\033[32m";
    m_asciiColorMap[3] = "\033[33m";
    m_asciiShowColors = true;
}

void ConfigManager::loadEmojiSettings() {
    m_emojiEnabled = true;
    m_emojiStyle = "auto";
}

std::string ConfigManager::applyEmojiStyle(const std::string& raw) const {
    if (!m_emojiEnabled) {
        return raw;
    }
    return raw; // dummy: no-op styling
}

std::string ConfigManager::getLabelRaw(const std::string& rawSection, const std::string& key, const std::string& defaultLabel) const {
    (void)rawSection; (void)key;
    return defaultLabel;
}

std::string ConfigManager::getNestedLabelRaw(const std::string& rawModule, const std::string& rawSection, const std::string& key, const std::string& defaultLabel) const {
    (void)rawModule; (void)rawSection; (void)key;
    return defaultLabel;
}

std::string ConfigManager::getPrefixRaw(const std::string& rawSection, const std::string& key, const std::string& defaultPrefix) const {
    (void)rawSection; (void)key;
    return defaultPrefix;
}

std::string ConfigManager::getNestedPrefixRaw(const std::string& rawModule, const std::string& rawSection, const std::string& key, const std::string& defaultPrefix) const {
    (void)rawModule; (void)rawSection; (void)key;
    return defaultPrefix;
}

std::string ConfigManager::getNestedStringRaw(const std::string& rawModule, const std::string& path, const std::string& defaultValue) const {
    (void)rawModule; (void)path;
    return defaultValue;
}

// ---- main() so you can actually run it ---------------------------------------------------------

#ifdef CONFIG_MANAGEMENT_STANDALONE_MAIN
int main() {
    ConfigManager cfg(ConfigMode::Dev);

    std::cout << "loaded: " << std::boolalpha << cfg.isLoaded() << "\n";
    std::cout << "color(theme.accent, default=green): " << cfg.getColor("theme", "accent", "green") << "\n";
    std::cout << "label: " << cfg.getLabel("header", "title", "BinaryFetch") << "\n";
    std::cout << "layout order:";
    for (const auto& s : cfg.getLayoutOrder()) std::cout << " " << s;
    std::cout << "\n";

    return 0;
}
#endif