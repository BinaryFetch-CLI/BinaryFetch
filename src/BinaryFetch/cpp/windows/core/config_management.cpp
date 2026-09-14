#include "core/config_management.h"
#include <windows.h>
#include <direct.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>

ConfigManager::ConfigManager(bool devMode) {
    loadPlatformConfig(devMode);
}

// ===================== CONFIG LOADING (JSON + JSONC) =====================
//
// BinaryFetch supports two config extensions side by side:
//   .jsonc  - preferred going forward; parsed with comments allowed
//   .json   - legacy; still parsed exactly as before (comments happen to
//             be allowed too now, but a comment-free file behaves
//             identically either way, so nothing already deployed breaks)
//
// Resolution order, checked fresh on every launch:
//   1. Both .jsonc and .json exist   -> .jsonc wins.
//   2. Only .jsonc exists            -> load it.
//   3. Only .json exists             -> load it AS-IS. We never silently
//                                        create a .jsonc next to it — an
//                                        existing legacy install stays on
//                                        .json until the user removes that
//                                        file themselves.
//   4. Neither exists                -> self-heal from the embedded EXE
//                                        resource. This is the ONLY branch
//                                        that ever creates a new file, and
//                                        it always writes .jsonc.
//
// So: delete the .json later (with no .jsonc present) and BinaryFetch will
// recreate a fresh .jsonc default on the next run, per case 4. Nothing
// about this ever overwrites a config file that already exists.
void ConfigManager::loadPlatformConfig(bool devMode) {
    std::string configDir       = "C:\\Users\\Public\\BinaryFetch";
    std::string userConfigJsonc = configDir + "\\BinaryFetch_Config.jsonc";
    std::string userConfigJson  = configDir + "\\BinaryFetch_Config.json";
    std::string configPath;

    if (devMode) {
        std::string devJsonc = "src\\BinaryFetch\\resources\\Default_JSON_theme_windows_RC\\Default_BinaryFetch_Config.jsonc";
        std::string devJson  = "src\\BinaryFetch\\resources\\Default_JSON_theme_windows_RC\\Default_BinaryFetch_Config.json";

        std::ifstream jsoncCheck(devJsonc);
        bool devJsoncExists = jsoncCheck.good();
        jsoncCheck.close();

        if (devJsoncExists) {
            configPath = devJsonc;
        } else {
            std::ifstream jsonCheck(devJson);
            bool devJsonExists = jsonCheck.good();
            jsonCheck.close();

            if (!devJsonExists) {
                std::cerr << "Warning: Could not find development configuration file at: "
                          << devJsonc << " or " << devJson << std::endl;
                m_loaded = false;
                return;
            }
            configPath = devJson;
        }
    } else {
        if (GetFileAttributesA(configDir.c_str()) == INVALID_FILE_ATTRIBUTES) {
            _mkdir(configDir.c_str());
        }

        std::ifstream jsoncCheck(userConfigJsonc);
        bool jsoncExists = jsoncCheck.good();
        jsoncCheck.close();

        std::ifstream jsonCheck(userConfigJson);
        bool jsonExists = jsonCheck.good();
        jsonCheck.close();

        if (jsoncExists) {
            // Case 1 (both present) and case 2 (jsonc only) both land here.
            configPath = userConfigJsonc;
        } else if (jsonExists) {
            // Case 3: legacy .json only. Load as-is, create nothing.
            configPath = userConfigJson;
        } else {
            // Case 4: neither present. Self-heal, writing .jsonc.
            configPath = userConfigJsonc;
            HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(101), RT_RCDATA);
            if (hRes) {
                HGLOBAL hData = LoadResource(NULL, hRes);
                DWORD size = SizeofResource(NULL, hRes);
                const char* data = static_cast<const char*>(LockResource(hData));
                std::ofstream userConfig(userConfigJsonc, std::ios::binary);
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
        // ignore_comments = true is the entire JSONC upgrade. A file with
        // zero comments in it — i.e. every existing .json config — parses
        // byte-for-byte the same way as before, so this is purely additive
        // and requires no migration step for anyone already running
        // BinaryFetch.
        m_config = nlohmann::json::parse(
            configFile,
            /* callback */ nullptr,
            /* allow_exceptions */ true,
            /* ignore_comments */ true
        );
         if (m_config.is_object() && m_config.empty()) {
            m_loaded = false;
        } else {
            m_loaded = true;
            loadColorPalette();   // populate m_colors from JSON "colors" section
            loadEmojiSettings();  // populate m_emojiEnabled / m_emojiStyle from JSON "emoji" section
        }
    } catch (...) {
        m_loaded = false;
    }
}

bool ConfigManager::isLoaded() const { return m_loaded; }

// ===================== COLOR PALETTE (JSON-DRIVEN) =====================
//
// Every color the app uses comes from the top-level "colors" object in
// JSON. Three formats are accepted per entry:
//   1. Raw ANSI escape  - starts with '\033', used exactly as written.
//   2. Hex              - "#RRGGBB", converted to 24-bit truecolor.
//   3. Plain RGB         - "R,G,B",   converted to 24-bit truecolor.
// A value of "RESET" is special-cased to the reset code. Anything else
// unrecognized is skipped (with a _DEBUG warning) rather than crashing
// or silently corrupting output.
std::string ConfigManager::parseColorValue(const std::string& raw) const {
    if (raw.empty()) return "";

    if (raw[0] == '\033') return raw; // literal escape sequence, passed through untouched

    if (raw[0] == '#' && raw.size() == 7) {
        try {
            int r = std::stoi(raw.substr(1, 2), nullptr, 16);
            int g = std::stoi(raw.substr(3, 2), nullptr, 16);
            int b = std::stoi(raw.substr(5, 2), nullptr, 16);
            return "\033[38;2;" + std::to_string(r) + ";" +
                   std::to_string(g) + ";" + std::to_string(b) + "m";
        } catch (...) {
            return "";
        }
    }

    std::stringstream ss(raw);
    std::string token;
    std::vector<int> parts;
    while (std::getline(ss, token, ',')) {
        try { parts.push_back(std::stoi(token)); }
        catch (...) { return ""; }
    }
    if (parts.size() == 3) {
        return "\033[38;2;" + std::to_string(parts[0]) + ";" +
               std::to_string(parts[1]) + ";" + std::to_string(parts[2]) + "m";
    }
    return "";
}

// Rebuilds m_colors entirely from JSON's "colors" object. No color name
// is hardcoded here — if "colors" is absent or empty, m_colors ends up
// empty and resolveColor() degrades every lookup to plain white via its
// single hardcoded safety net, without affecting any other config value.
void ConfigManager::loadColorPalette() {
    m_colors.clear();

    if (!m_config.contains("colors") || !m_config["colors"].is_object())
        return;

    for (auto& [name, value] : m_config["colors"].items()) {
        if (!value.is_string()) continue;
        std::string raw = value.get<std::string>();

        if (raw == "RESET") { m_colors[name] = "\033[0m"; continue; }

        std::string ansi = parseColorValue(raw);
        if (!ansi.empty()) {
            m_colors[name] = ansi;
        }
#ifdef _DEBUG
        else {
            std::cerr << "Warning: invalid color value for '" << name << "': " << raw << "\n";
        }
#endif
    }
}

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
    if (whiteIt != m_colors.end()) return whiteIt->second;
    return "\033[37m"; // sole hardcoded fallback: only used if "colors" section is missing/empty in JSON
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


// nested int, read int data from json
int ConfigManager::getNestedInt(
    const std::string& rawModule,
    const std::string& path,
    int defaultValue) const
{
    std::string module = resolveSectionKey(rawModule);

    if (!m_loaded || !m_config.contains(module))
        return defaultValue;

    std::vector<std::string> keys;
    std::stringstream ss(path);
    std::string key;

    while (std::getline(ss, key, '.'))
        keys.push_back(key);

    if (!keys.empty())
        keys[0] = resolveSubsectionKey(module, keys[0]);

    nlohmann::json current = m_config[module];

    for (const auto& k : keys)
    {
        if (!current.contains(k))
            return defaultValue;

        current = current[k];
    }

    if (current.is_number_integer())
        return current.get<int>();

    return defaultValue;
}





//nested string:
std::string ConfigManager::getNestedStringRaw(
    const std::string& rawModule,
    const std::string& path,
    const std::string& defaultValue) const
{
    std::string module = resolveSectionKey(rawModule);

    if (!m_loaded || !m_config.contains(module))
        return defaultValue;

    std::vector<std::string> keys;
    std::stringstream ss(path);
    std::string key;

    while (std::getline(ss, key, '.'))
    {
        keys.push_back(key);
    }

    if (!keys.empty())
    {
        keys[0] = resolveSubsectionKey(module, keys[0]);
    }

    nlohmann::json current = m_config[module];

    for (const auto& k : keys)
    {
        if (!current.contains(k))
            return defaultValue;

        current = current[k];
    }

    if (current.is_string())
        return current.get<std::string>();

    return defaultValue;
}


// ===================== EMOJI STYLE (NEW) =====================
//
// Central place where emoji presentation gets applied, so no individual
// module (CPU, GPU, memory, etc.) needs to know this feature exists.
// Every module already pulls its icons/labels through getLabel/getPrefix/
// getNestedLabel/getNestedPrefix/getNestedString — those five are now
// thin wrappers around the *Raw versions above, piped through
// applyEmojiStyle(). Section/alias resolution, defaults, and fallback
// behavior are all unchanged; only the final returned string differs,
// and only when an "emoji" section is present and non-default.

namespace {

// Decodes one UTF-8 codepoint at byte index i, writes the number of
// bytes consumed into len. Malformed/truncated sequences fall back to
// treating the single byte as-is, so a stray byte never corrupts or
// crashes the rest of the string.
char32_t decodeUtf8(const std::string& s, size_t i, size_t& len) {
    unsigned char c0 = static_cast<unsigned char>(s[i]);
    size_t remaining = s.size() - i;

    auto isCont = [&](size_t idx) {
        return idx < s.size() && (static_cast<unsigned char>(s[idx]) & 0xC0) == 0x80;
    };

    if (c0 < 0x80) { len = 1; return c0; }

    if ((c0 & 0xE0) == 0xC0 && remaining >= 2 && isCont(i + 1)) {
        len = 2;
        return ((c0 & 0x1F) << 6) | (static_cast<unsigned char>(s[i + 1]) & 0x3F);
    }
    if ((c0 & 0xF0) == 0xE0 && remaining >= 3 && isCont(i + 1) && isCont(i + 2)) {
        len = 3;
        return ((c0 & 0x0F) << 12)
             | ((static_cast<unsigned char>(s[i + 1]) & 0x3F) << 6)
             |  (static_cast<unsigned char>(s[i + 2]) & 0x3F);
    }
    if ((c0 & 0xF8) == 0xF0 && remaining >= 4 && isCont(i + 1) && isCont(i + 2) && isCont(i + 3)) {
        len = 4;
        return ((c0 & 0x07) << 18)
             | ((static_cast<unsigned char>(s[i + 1]) & 0x3F) << 12)
             | ((static_cast<unsigned char>(s[i + 2]) & 0x3F) << 6)
             |  (static_cast<unsigned char>(s[i + 3]) & 0x3F);
    }

    len = 1;
    return c0; // unrecognised lead byte -> pass through untouched
}

void encodeUtf8(char32_t cp, std::string& out) {
    if (cp < 0x80) {
        out += static_cast<char>(cp);
    } else if (cp < 0x800) {
        out += static_cast<char>(0xC0 | (cp >> 6));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    } else if (cp < 0x10000) {
        out += static_cast<char>(0xE0 | (cp >> 12));
        out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    } else {
        out += static_cast<char>(0xF0 | (cp >> 18));
        out += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
        out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    }
}

// Heuristic "is this codepoint the kind of symbol/pictograph emoji
// presentation selectors apply to". Covers the blocks BinaryFetch's own
// icon set actually uses (dingbats/misc symbols + the supplementary
// pictograph planes) plus a few adjacent symbol blocks. Not the full
// Unicode emoji-property table, deliberately — no dependency needed —
// but it's a safe superset for this app: nothing in these ranges is
// normal prose text that could get accidentally mangled.
bool isEmojiEligible(char32_t cp) {
    return (cp >= 0x2190 && cp <= 0x21FF)    // Arrows
        || (cp >= 0x2300 && cp <= 0x23FF)    // Misc Technical (⌚⏰⏱ etc.)
        || (cp >= 0x25A0 && cp <= 0x25FF)    // Geometric Shapes
        || (cp >= 0x2600 && cp <= 0x27BF)    // Misc Symbols + Dingbats (⚙️☀️✂️)
        || (cp >= 0x2B00 && cp <= 0x2BFF)    // Misc Symbols and Arrows (⭐)
        || (cp >= 0x1F000 && cp <= 0x1FFFF); // All main emoji pictograph blocks
}

constexpr char32_t VS_TEXT  = 0xFE0E; // U+FE0E - text presentation
constexpr char32_t VS_EMOJI = 0xFE0F; // U+FE0F - emoji presentation

} // namespace

void ConfigManager::loadEmojiSettings() {
    m_emojiEnabled = true;
    m_emojiStyle   = "auto";

    if (!m_config.contains("emoji") || !m_config["emoji"].is_object())
        return; // section absent -> unchanged behavior, byte-identical output

    const auto& e = m_config["emoji"];

    if (e.contains("enabled") && e["enabled"].is_boolean())
        m_emojiEnabled = e["enabled"].get<bool>();

    if (e.contains("style") && e["style"].is_string()) {
        std::string s = e["style"].get<std::string>();
        if (s == "auto" || s == "color" || s == "text") {
            m_emojiStyle = s;
        }
#ifdef _DEBUG
        else {
            std::cerr << "Warning: invalid emoji style '" << s << "', defaulting to 'auto'\n";
        }
#endif
    }
}

std::string ConfigManager::applyEmojiStyle(const std::string& raw) const {
    // Fast path: enabled + auto is a pure no-op — what every config
    // written before this feature existed will hit.
    if (m_emojiEnabled && m_emojiStyle == "auto") return raw;
    if (raw.empty()) return raw;

    std::string out;
    out.reserve(raw.size());

    size_t i = 0;
    while (i < raw.size()) {
        size_t len = 1;
        char32_t cp = decodeUtf8(raw, i, len);

        if (isEmojiEligible(cp)) {
            // Does an explicit variation selector already follow it?
            // Consume it either way — we're about to decide the
            // presentation ourselves.
            size_t next = i + len;
            size_t vsLen = 0;
            if (next < raw.size()) {
                size_t peekLen;
                char32_t peekCp = decodeUtf8(raw, next, peekLen);
                if (peekCp == VS_TEXT || peekCp == VS_EMOJI) vsLen = peekLen;
            }

            if (!m_emojiEnabled) {
                // Drop the glyph (and its selector) entirely.
            } else if (m_emojiStyle == "text") {
                out.append(raw, i, len);
                encodeUtf8(VS_TEXT, out);
            } else if (m_emojiStyle == "color") {
                out.append(raw, i, len);
                encodeUtf8(VS_EMOJI, out);
            } else {
                // Shouldn't happen (loadEmojiSettings validates), stay safe.
                out.append(raw, i, len + vsLen);
            }

            i = next + vsLen;
            continue;
        }

        out.append(raw, i, len);
        i += len;
    }

    return out;
}

// ===================== PUBLIC WRAPPERS (NEW) =====================
// Same signatures modules already call — every module keeps working
// with zero edits. Each just pipes the *Raw result through the emoji
// styling layer.

std::string ConfigManager::getLabel(const std::string& rawSection, const std::string& key, const std::string& defaultLabel) const {
    return applyEmojiStyle(getLabelRaw(rawSection, key, defaultLabel));
}

std::string ConfigManager::getNestedLabel(const std::string& rawModule, const std::string& rawSection, const std::string& key, const std::string& defaultLabel) const {
    return applyEmojiStyle(getNestedLabelRaw(rawModule, rawSection, key, defaultLabel));
}

std::string ConfigManager::getPrefix(const std::string& rawSection, const std::string& key, const std::string& defaultPrefix) const {
    return applyEmojiStyle(getPrefixRaw(rawSection, key, defaultPrefix));
}

std::string ConfigManager::getNestedPrefix(const std::string& rawModule, const std::string& rawSection, const std::string& key, const std::string& defaultPrefix) const {
    return applyEmojiStyle(getNestedPrefixRaw(rawModule, rawSection, key, defaultPrefix));
}

std::string ConfigManager::getNestedString(const std::string& rawModule, const std::string& path, const std::string& defaultValue) const {
    return applyEmojiStyle(getNestedStringRaw(rawModule, path, defaultValue));
}


// nested string array, read array of strings from json (used for "layout" / "order")
std::vector<std::string> ConfigManager::getStringArray(
    const std::string& rawModule,
    const std::string& path,
    const std::vector<std::string>& fallback) const
{
    if (!m_loaded)
        return fallback;

    nlohmann::json current;

    if (rawModule.empty()) {
        // Root-level lookup (used by getLayoutOrder for top-level "layout")
        current = m_config;
    } else {
        std::string module = resolveSectionKey(rawModule);
        if (!m_config.contains(module))
            return fallback;
        current = m_config[module];
    }

    std::vector<std::string> keys;
    std::stringstream ss(path);
    std::string key;

    while (std::getline(ss, key, '.'))
        keys.push_back(key);

    if (!rawModule.empty() && !keys.empty())
        keys[0] = resolveSubsectionKey(resolveSectionKey(rawModule), keys[0]);

    for (const auto& k : keys)
    {
        if (!current.contains(k))
            return fallback;

        current = current[k];
    }

    if (!current.is_array() || current.empty())
        return fallback;

    std::vector<std::string> result;
    result.reserve(current.size());
    for (const auto& item : current) {
        if (item.is_string()) {
            result.push_back(item.get<std::string>());
        }
        // non-string entries are silently skipped, per §6.2 of the ordering spec
    }

    return result.empty() ? fallback : result;
}

// convenience wrapper for the top-level "layout" key
std::vector<std::string> ConfigManager::getLayoutOrder() const
{
    static const std::vector<std::string> defaultLayout = {
        "header_settings",
        "compact_date_and_time",
        "compact_operating_system",
        "compact_processor",
        "compact_graphics_card",
        "compact_display_monitor",
        "compact_system_memory",
        "compact_audio_devices",
        "compact_resource_usage",
        "compact_user_account",
        "compact_network_connection",
        "compact_disk_storage",
        "detailed_system_memory",
        "detailed_disk_storage",
        "detailed_network_connection",
        "detailed_operating_system",
        "detailed_processor",
        "detailed_graphics_card",
        "detailed_display_monitor",
        "detailed_bios_and_motherboard",
        "detailed_user_account",
        "detailed_resource_usage",
        "detailed_audio_and_power"
    };

    return getStringArray("", "section_order", defaultLayout);
}



// ===================== LABEL RESOLUTION =====================
std::string ConfigManager::getLabelRaw(const std::string& rawSection, const std::string& key, const std::string& defaultLabel) const {
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

std::string ConfigManager::getNestedLabelRaw(const std::string& rawModule, const std::string& rawSection, const std::string& key, const std::string& defaultLabel) const {
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
std::string ConfigManager::getPrefixRaw(const std::string& rawSection, const std::string& key, const std::string& defaultPrefix) const {
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

std::string ConfigManager::getNestedPrefixRaw(const std::string& rawModule, const std::string& rawSection, const std::string& key, const std::string& defaultPrefix) const {
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