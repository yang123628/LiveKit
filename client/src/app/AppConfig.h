#pragma once

#include <QString>
#include <QSettings>

class AppConfig {
public:
    static AppConfig& instance();

    QString serverAddress() const;
    void setServerAddress(const QString& address);

    QString themeName() const;
    void setThemeName(const QString& name);

    QString token() const;
    void setToken(const QString& token);

    void save();
    void load();

private:
    AppConfig();
    ~AppConfig() = default;
    AppConfig(const AppConfig&) = delete;
    AppConfig& operator=(const AppConfig&) = delete;

    QSettings m_settings;
    QString m_serverAddress;
    QString m_themeName;
    QString m_token;
};
