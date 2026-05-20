#include "theme/ThemeManager.h"
#include "app/AppConfig.h"
#include <QFile>
#include <QApplication>

ThemeManager& ThemeManager::instance() {
    static ThemeManager inst;
    return inst;
}

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent)
    , m_currentTheme("dark")
{
}

void ThemeManager::applyTheme(const QString& themeName) {
    m_currentTheme = themeName;

    QString qssPath;
    if (themeName == "light") {
        qssPath = ":/qss/light.qss";
    } else {
        qssPath = ":/qss/dark.qss";
    }

    QFile file(qssPath);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QString styleSheet = QString::fromUtf8(file.readAll());
        qApp->setStyleSheet(styleSheet);
        file.close();
    }

    AppConfig::instance().setThemeName(themeName);
    AppConfig::instance().save();

    emit themeChanged(themeName);
}

QString ThemeManager::currentTheme() const {
    return m_currentTheme;
}
