#pragma once

#include <QApplication>
#include <memory>

class MainWindow;
class AppConfig;

class Application {
public:
    static Application& instance();

    void initialize(int& argc, char** argv);
    int run();

    AppConfig* config() const;
    MainWindow* mainWindow() const;

private:
    Application();
    ~Application() = default;
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    std::unique_ptr<QApplication> m_app;
    MainWindow* m_mainWindow;
};
