#ifndef CONFIG_MANAGEMENT_H
#define CONFIG_MANAGEMENT_H

#include <string>
#include <map>
#include <vector>
#include "nlohmann/json.hpp"

class ConfigManager {
public:
    explicit ConfigManager(bool devMode = false);

    bool isLoaded() const;

    // Enabled checks
    bool isEnabled(const std::string& section) const;
    bool isSubEnabled(const std::string& section, const std::string& key) const;
    bool isSectionEnabled(const std::string& module, const std::string& section) const;
    bool isNestedEnabled(const std::string& module, const std::string& section, const std::string& key) const;
    bool getNestedBool(const std::string& module, const std::string& path, bool defaultValue = true) const;
    bool getNestedBool(const std::string& path, bool defaultValue = true) const;

    // Color resolution
    std::string getColor(const std::string& section, const std::string& key, const std::string& defaultColor = "white") const;
    std::string getNestedColor(const std::string& module, const std::string& subsection, const std::string& key, const std::string& defaultColor = "white") const;
    std::string getNestedColor(const std::string& module, const std::string& path, const std::string& defaultColor = "white") const;
    std::string getNestedColor(const std::string& path, const std::string& defaultColor = "white") const;
    std::string getResetColor() const;

    // Label resolution
    std::string getLabel(const std::string& section, const std::string& key, const std::string& defaultLabel = "") const;
    std::string getNestedLabel(const std::string& module, const std::string& section, const std::string& key, const std::string& defaultLabel = "") const;

    // Raw JSON access if required
    const nlohmann::json& getJson() const;

private:
    void loadPlatformConfig(bool devMode);
    std::string resolveColor(const std::string& colorName, const std::string& defaultColor) const;

    nlohmann::json m_config;
    bool m_loaded{false};
    std::map<std::string, std::string> m_colors;
};

#endif // CONFIG_MANAGEMENT_H
