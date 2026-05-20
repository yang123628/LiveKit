#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() = default;

    void switchPage(int index);

private:
    void setupUI();
    void setupNavigationBar();
    void setupPages();

    QWidget* m_centralWidget;
    QVBoxLayout* m_mainLayout;
    QStackedWidget* m_stackedWidget;
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
