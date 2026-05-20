#pragma once

#include <string>
#include <unordered_map>
#include <map>

class Config {
public:
    static Config& instance();

    bool load(const std::string& path);

    std::string get(const std::string& section, const std::string& key, const std::string& defaultValue = "") const;
    int getInt(const std::string& section, const std::string& key, int defaultValue = 0) const;

private:
    Config() = default;
    ~Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    std::map<std::string, std::unordered_map<std::string, std::string>> m_config;
};
