#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

class LoginPage;
class RegisterPage;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() = default;

    void switchPage(int index);
    void showAuthPage();
    void showMainPage();

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

    QWidget* m_pageLiveHall;
    QWidget* m_pageStartLive;
    QWidget* m_pageProfile;

    int m_currentIndex;
};
