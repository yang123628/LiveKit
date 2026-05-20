#pragma once

#include <QString>
#include <QObject>

class ThemeManager : public QObject {
    Q_OBJECT

public:
    static ThemeManager& instance();

    void applyTheme(const QString& themeName);
    QString currentTheme() const;

signals:
    void themeChanged(const QString& themeName);

private:
    ThemeManager(QObject* parent = nullptr);
    ~ThemeManager() = default;
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    QString m_currentTheme;
};
