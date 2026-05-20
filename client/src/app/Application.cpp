#include "app/Application.h"
#include "app/AppConfig.h"
#include "ui/MainWindow.h"
#include "theme/ThemeManager.h"

Application& Application::instance() {
    static Application inst;
    return inst;
}

Application::Application()
    : m_mainWindow(nullptr)
{
}

void Application::initialize(int& argc, char** argv) {
    m_app = std::make_unique<QApplication>(argc, argv);

    QApplication::setApplicationName("LiveKit");
    QApplication::setApplicationVersion("0.1.0");
    QApplication::setOrganizationName("LiveKit");

    AppConfig::instance().load();

    m_mainWindow = new MainWindow;

    auto& themeMgr = ThemeManager::instance();
    themeMgr.applyTheme(AppConfig::instance().themeName());
}

int Application::run() {
    m_mainWindow->show();
    return m_app->exec();
}

AppConfig* Application::config() const {
    return &AppConfig::instance();
}

MainWindow* Application::mainWindow() const {
    return m_mainWindow;
}
