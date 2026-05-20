#include "ui/MainWindow.h"
#include "theme/ThemeManager.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_currentIndex(0)
{
    setupUI();
}

void MainWindow::setupUI() {
    setWindowTitle("LiveKit");
    setMinimumSize(960, 640);
    resize(1200, 800);

    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    setupPages();
    setupNavigationBar();

    m_mainLayout->addWidget(m_stackedWidget, 1);
    m_mainLayout->addWidget(m_navigationBar);
}

void MainWindow::setupNavigationBar() {
    m_navigationBar = new QWidget(this);
    m_navigationBar->setObjectName("navigationBar");
    m_navigationBar->setFixedHeight(60);

    m_navLayout = new QHBoxLayout(m_navigationBar);
    m_navLayout->setContentsMargins(0, 0, 0, 0);
    m_navLayout->setSpacing(0);

    m_btnLiveHall = new QPushButton(QStringLiteral("直播大厅"), this);
    m_btnLiveHall->setObjectName("navButton");
    m_btnLiveHall->setCheckable(true);
    m_btnLiveHall->setChecked(true);

    m_btnStartLive = new QPushButton(QStringLiteral("我要开播"), this);
    m_btnStartLive->setObjectName("navButton");
    m_btnStartLive->setCheckable(true);

    m_btnProfile = new QPushButton(QStringLiteral("个人中心"), this);
    m_btnProfile->setObjectName("navButton");
    m_btnProfile->setCheckable(true);

    m_navLayout->addWidget(m_btnLiveHall);
    m_navLayout->addWidget(m_btnStartLive);
    m_navLayout->addWidget(m_btnProfile);

    connect(m_btnLiveHall, &QPushButton::clicked, [this]() { switchPage(0); });
    connect(m_btnStartLive, &QPushButton::clicked, [this]() { switchPage(1); });
    connect(m_btnProfile, &QPushButton::clicked, [this]() { switchPage(2); });
}

void MainWindow::setupPages() {
    m_stackedWidget = new QStackedWidget(this);

    m_pageLiveHall = new QWidget(this);
    m_pageLiveHall->setObjectName("pageLiveHall");
    auto* hallLayout = new QVBoxLayout(m_pageLiveHall);
    auto* hallLabel = new QLabel(QStringLiteral("直播大厅"), m_pageLiveHall);
    hallLabel->setObjectName("pageTitle");
    hallLabel->setAlignment(Qt::AlignCenter);
    hallLayout->addWidget(hallLabel);

    m_pageStartLive = new QWidget(this);
    m_pageStartLive->setObjectName("pageStartLive");
    auto* startLayout = new QVBoxLayout(m_pageStartLive);
    auto* startLabel = new QLabel(QStringLiteral("我要开播"), m_pageStartLive);
    startLabel->setObjectName("pageTitle");
    startLabel->setAlignment(Qt::AlignCenter);
    startLayout->addWidget(startLabel);

    m_pageProfile = new QWidget(this);
    m_pageProfile->setObjectName("pageProfile");
    auto* profileLayout = new QVBoxLayout(m_pageProfile);
    auto* profileLabel = new QLabel(QStringLiteral("个人中心"), m_pageProfile);
    profileLabel->setObjectName("pageTitle");
    profileLabel->setAlignment(Qt::AlignCenter);
    profileLayout->addWidget(profileLabel);

    m_stackedWidget->addWidget(m_pageLiveHall);
    m_stackedWidget->addWidget(m_pageStartLive);
    m_stackedWidget->addWidget(m_pageProfile);
}

void MainWindow::switchPage(int index) {
    if (index < 0 || index >= m_stackedWidget->count()) return;

    m_currentIndex = index;
    m_stackedWidget->setCurrentIndex(index);

    m_btnLiveHall->setChecked(index == 0);
    m_btnStartLive->setChecked(index == 1);
    m_btnProfile->setChecked(index == 2);
}
