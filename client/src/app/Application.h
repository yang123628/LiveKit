#pragma once

#include <QApplication>
#include <memory>
#include "model/UserInfo.h"

class MainWindow;
class AppConfig;
class IHttpClient;

class Application {
public:
    static Application& instance();

    void initialize(int& argc, char** argv);
    int run();

    AppConfig* config() const;
    MainWindow* mainWindow() const;
    IHttpClient* httpClient() const;
    UserInfo& currentUser();

private:
    Application();
    ~Application() = default;
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    std::unique_ptr<QApplication> m_app;
    MainWindow* m_mainWindow;
    IHttpClient* m_httpClient;
    UserInfo m_currentUser;
};
