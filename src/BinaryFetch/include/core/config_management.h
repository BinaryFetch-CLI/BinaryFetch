#ifndef CONFIG_MANAGEMENT_H
#define CONFIG_MANAGEMENT_H

#include <string>
#include <map>
#include <vector>
#include "nlohmann/json.hpp"


enum class ConfigMode {
    Dev,           // Dev_jsonc\Dev_BinaryFetch_Config.jsonc
    ReleaseSource, // Default_JSON_theme_windows_RC\Default_BinaryFetch_Config.jsonc (edit directly)
    Production     // C:\Users\Public\BinaryFetch\... self-heals from embedded resource 101
};

class ConfigManager {
public:
    explicit ConfigManager(ConfigMode mode = ConfigMode::Production);

    bool isLoaded() const;

    // Enabled checks
    bool isEnabled(const std::string& section) const;
    bool isFieldEnabled(const std::string& section, const std::string& fieldPath) const;  // NEW
    bool isSubEnabled(const std::string& section, const std::string& key) const;
    bool isSectionEnabled(const std::string& module, const std::string& section) const;
    bool isNestedEnabled(const std::string& module, const std::string& section, const std::string& key) const;
    bool getNestedBool(const std::string& module, const std::string& path, bool defaultValue = true) const;
    bool getNestedBool(const std::string& path, bool defaultValue = true) const;
    int getNestedInt(const std::string& module, const std::string& path, int defaultValue = 0) const;

    std::string getNestedString(const std::string& module,const std::string& path,const std::string& defaultValue = "") const;

    // Color resolution
    std::string getColor(const std::string& section, const std::string& key, const std::string& defaultColor = "white") const;
    std::string getNestedColor(const std::string& module, const std::string& subsection, const std::string& key, const std::string& defaultColor) const;
    std::string getNestedColor(const std::string& module, const std::string& path, const std::string& defaultColor = "white") const;
    std::string getNestedColor(const std::string& path, const std::string& defaultColor = "white") const;
    std::string getResetColor() const;

    // Label and Prefix resolution
    std::string getLabel(const std::string& section, const std::string& key, const std::string& defaultLabel = "") const;
    std::string getNestedLabel(const std::string& module, const std::string& section, const std::string& key, const std::string& defaultLabel = "") const;
    std::string getPrefix(const std::string& section, const std::string& key, const std::string& defaultPrefix = "") const;
    std::string getNestedPrefix(const std::string& module, const std::string& section, const std::string& key, const std::string& defaultPrefix = "") const;

    // String array resolution (for "layout" / "order" JSON keys)
    std::vector<std::string> getStringArray(
        const std::string& module,
        const std::string& path,
        const std::vector<std::string>& fallback) const;
        
    std::vector<std::string> getLayoutOrder() const;

    // Ascii-art $N color prefixes (built from JSON "ascii_color_prefixes",
    // falling back to the built-in default table for any $N not specified)
    const std::map<int, std::string>& getAsciiColorMap() const;

    // Raw JSON access if required
    const nlohmann::json& getJson() const;

private:
    void loadPlatformConfig(ConfigMode mode);
    std::string resolveSectionKey(const std::string& section) const;
    std::string resolveSubsectionKey(const std::string& module, const std::string& subsection) const;
    std::string resolveColor(const std::string& colorName, const std::string& defaultColor) const;
    
    // NEW: hex / rgb / raw-ansi -> escape code
    std::string parseColorValue(const std::string& raw) const;   
    void loadColorPalette();             // builds m_colors entirely from JSON
    void loadAsciiColorPrefixes();       // builds m_asciiColorMap from JSON "ascii_color_prefixes"

    // EMOJI STYLE SUPPORT (NEW)
    void loadEmojiSettings();
    std::string applyEmojiStyle(const std::string& raw) const;

    // Renamed originals — same logic as before, just called "Raw" now.
    // The public getLabel/getNestedLabel/getPrefix/getNestedPrefix/getNestedString
    // below are thin wrappers that pipe these through applyEmojiStyle().
    std::string getLabelRaw(const std::string& rawSection, const std::string& key, const std::string& defaultLabel) const;
    std::string getNestedLabelRaw(const std::string& rawModule, const std::string& rawSection, const std::string& key, const std::string& defaultLabel) const;
    std::string getPrefixRaw(const std::string& rawSection, const std::string& key, const std::string& defaultPrefix) const;
    std::string getNestedPrefixRaw(const std::string& rawModule, const std::string& rawSection, const std::string& key, const std::string& defaultPrefix) const;
    std::string getNestedStringRaw(const std::string& rawModule, const std::string& path, const std::string& defaultValue) const;

    nlohmann::json m_config;
    bool m_loaded{false};
    std::map<std::string, std::string> m_colors;
    std::map<int, std::string> m_asciiColorMap;       // $N -> ANSI escape, for ASCII art color prefixes

    bool m_emojiEnabled{true};   
    std::string m_emojiStyle{"auto"}; 
};

#endif // CONFIG_MANAGEMENT_H