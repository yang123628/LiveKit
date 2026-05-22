#include "app/AppConfig.h"

AppConfig& AppConfig::instance() {
    static AppConfig inst;
    return inst;
}

AppConfig::AppConfig()
    : m_settings("LiveKit", "LiveKitClient")
<<<<<<< HEAD
    , m_serverAddress("http://192.168.124.192:9090")
=======
    , m_serverAddress("http://192.168.124.129:9090")
>>>>>>> 03e5a0dd4a25e1bc5a6dc35acc7c13f0914fb869
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

QSettings& AppConfig::settings() {
    return m_settings;
}
