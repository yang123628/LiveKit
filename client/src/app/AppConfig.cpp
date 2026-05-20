#include "app/AppConfig.h"

AppConfig& AppConfig::instance() {
    static AppConfig inst;
    return inst;
}

AppConfig::AppConfig()
    : m_settings("LiveKit", "LiveKitClient")
    , m_serverAddress("http://192.168.1.100:8080")
    , m_themeName("dark")
    , m_token("")
{
    load();
}

QString AppConfig::serverAddress() const {
    return m_serverAddress;
}

void AppConfig::setServerAddress(const QString& address) {
    m_serverAddress = address;
}

QString AppConfig::themeName() const {
    return m_themeName;
}

void AppConfig::setThemeName(const QString& name) {
    m_themeName = name;
}

QString AppConfig::token() const {
    return m_token;
}

void AppConfig::setToken(const QString& token) {
    m_token = token;
}

void AppConfig::save() {
    m_settings.setValue("server/address", m_serverAddress);
    m_settings.setValue("theme/name", m_themeName);
    m_settings.setValue("user/token", m_token);
    m_settings.sync();
}

void AppConfig::load() {
    if (m_settings.contains("server/address")) {
        m_serverAddress = m_settings.value("server/address").toString();
    }
    if (m_settings.contains("theme/name")) {
        m_themeName = m_settings.value("theme/name").toString();
    }
    if (m_settings.contains("user/token")) {
        m_token = m_settings.value("user/token").toString();
    }
}
