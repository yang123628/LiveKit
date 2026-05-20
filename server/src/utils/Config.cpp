#include "utils/Config.h"
#include "core/Logger.h"
#include <fstream>

Config& Config::instance() {
    static Config config;
    return config;
}

bool Config::load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        LOG_ERROR("cannot open config file: " << path);
        return false;
    }

    std::string currentSection;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        if (line[0] == '[') {
            size_t end = line.find(']');
            if (end != std::string::npos) {
                currentSection = line.substr(1, end - 1);
            }
        } else {
            size_t eq = line.find('=');
            if (eq != std::string::npos) {
                std::string key = line.substr(0, eq);
                std::string value = line.substr(eq + 1);
                m_config[currentSection][key] = value;
            }
        }
    }

    LOG_INFO("config loaded from " << path);
    return true;
}

std::string Config::get(const std::string& section, const std::string& key, const std::string& defaultValue) const {
    auto secIt = m_config.find(section);
    if (secIt == m_config.end()) return defaultValue;
    auto keyIt = secIt->second.find(key);
    if (keyIt == secIt->second.end()) return defaultValue;
    return keyIt->second;
}

int Config::getInt(const std::string& section, const std::string& key, int defaultValue) const {
    std::string value = get(section, key);
    if (value.empty()) return defaultValue;
    try {
        return std::stoi(value);
    } catch (...) {
        return defaultValue;
    }
}
