#include "ui/MainWindow.h"
#include "ui/LoginPage.h"
#include "ui/RegisterPage.h"
#include "ui/LiveHallPage.h"
#include "theme/ThemeManager.h"
#include "app/AppConfig.h"
#include "app/Application.h"
#include <QDebug>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_currentIndex(0)
{
    setupUI();
    checkAutoLogin();
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

    m_topStack = new QStackedWidget(this);

    setupAuthPages();
    setupPages();
    setupNavigationBar();

    m_mainPage = new QWidget(this);
    m_mainPage->setObjectName("mainPage");
    m_mainPageLayout = new QVBoxLayout(m_mainPage);
    m_mainPageLayout->setContentsMargins(0, 0, 0, 0);
    m_mainPageLayout->setSpacing(0);
    m_mainPageLayout->addWidget(m_contentStack, 1);
    m_mainPageLayout->addWidget(m_navigationBar);

    m_topStack->addWidget(m_authPage);
    m_topStack->addWidget(m_mainPage);

    m_mainLayout->addWidget(m_topStack);
}

void MainWindow::setupAuthPages() {
    m_authPage = new QWidget(this);
    m_authPage->setObjectName("authPage");

    auto* authLayout = new QVBoxLayout(m_authPage);
    authLayout->setContentsMargins(0, 0, 0, 0);

    m_authStack = new QStackedWidget(m_authPage);
    m_loginPage = new LoginPage(m_authStack);
    m_registerPage = new RegisterPage(m_authStack);

    m_authStack->addWidget(m_loginPage);
    m_authStack->addWidget(m_registerPage);

    authLayout->addWidget(m_authStack);

    connect(m_loginPage, &LoginPage::switchToRegister, [this]() {
        m_authStack->setCurrentWidget(m_registerPage);
    });
    connect(m_registerPage, &RegisterPage::switchToLogin, [this]() {
        m_authStack->setCurrentWidget(m_loginPage);
    });
    connect(m_loginPage, &LoginPage::loginSuccess, this, &MainWindow::showMainPage);
    connect(m_registerPage, &RegisterPage::registerSuccess, this, &MainWindow::showMainPage);
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
    m_contentStack = new QStackedWidget(this);

    m_pageLiveHall = new LiveHallPage(this);
    m_pageLiveHall->setObjectName("pageLiveHall");

    connect(m_pageLiveHall, &LiveHallPage::roomClicked, [](int roomId) {
        qDebug() << "Room clicked:" << roomId;
    });

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

    m_contentStack->addWidget(m_pageLiveHall);
    m_contentStack->addWidget(m_pageStartLive);
    m_contentStack->addWidget(m_pageProfile);
}

void MainWindow::switchPage(int index) {
    if (index < 0 || index >= m_contentStack->count()) return;

    m_currentIndex = index;
    m_contentStack->setCurrentIndex(index);

    m_btnLiveHall->setChecked(index == 0);
    m_btnStartLive->setChecked(index == 1);
    m_btnProfile->setChecked(index == 2);
}

void MainWindow::showAuthPage() {
    m_authStack->setCurrentWidget(m_loginPage);
    m_topStack->setCurrentWidget(m_authPage);
}

void MainWindow::showMainPage() {
    m_topStack->setCurrentWidget(m_mainPage);
    switchPage(0);
    m_pageLiveHall->refreshRooms();
}

void MainWindow::checkAutoLogin() {
    auto& config = AppConfig::instance();
    auto token = config.token();

    if (!token.isEmpty()) {
        auto& user = Application::instance().currentUser();
        user.setToken(token);
        showMainPage();
    } else {
        showAuthPage();
    }
}
