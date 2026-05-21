#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

class LoginPage;
class RegisterPage;
class LiveHallPage;
class StartLivePage;
class AnchorRoomPage;
class LiveRoomPage;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() = default;

    void switchPage(int index);
    void showAuthPage();
    void showMainPage();
    void showLiveRoom(const QString& playUrl, int roomId);
    void showAnchorRoom(const QString& pushUrl, int mode);

private:
    void setupUI();
    void setupNavigationBar();
    void setupPages();
    void setupAuthPages();
    void checkAutoLogin();

    QWidget* m_centralWidget;
    QVBoxLayout* m_mainLayout;

    QStackedWidget* m_topStack;

    QWidget* m_authPage;
    QStackedWidget* m_authStack;
    LoginPage* m_loginPage;
    RegisterPage* m_registerPage;

    QWidget* m_mainPage;
    QVBoxLayout* m_mainPageLayout;
    QStackedWidget* m_contentStack;
    QWidget* m_navigationBar;
    QHBoxLayout* m_navLayout;

    QPushButton* m_btnLiveHall;
    QPushButton* m_btnStartLive;
    QPushButton* m_btnProfile;

    LiveHallPage* m_pageLiveHall;
    StartLivePage* m_pageStartLive;
    QWidget* m_pageProfile;

#ifdef HAS_FFMPEG
    AnchorRoomPage* m_pageAnchorRoom;
    LiveRoomPage* m_pageLiveRoom;
#endif

    int m_currentIndex;
};
