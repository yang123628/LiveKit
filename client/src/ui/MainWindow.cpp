#include "ui/MainWindow.h"
#include "ui/LoginPage.h"
#include "ui/RegisterPage.h"
#include "ui/LiveHallPage.h"
#include "ui/StartLivePage.h"
#include "theme/ThemeManager.h"
#include "app/AppConfig.h"
#include "app/Application.h"
#include <QDebug>

#ifdef HAS_FFMPEG
#include "ui/AnchorRoomPage.h"
#include "ui/LiveRoomPage.h"
#endif

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

    connect(m_pageLiveHall, &LiveHallPage::roomClicked, [this](int roomId) {
#ifdef HAS_FFMPEG
        QString playUrl = QString("rtmp://%1/live/room_%2")
            .arg(Application::instance().config()->serverAddress().replace("http://", ""))
            .arg(roomId);
        showLiveRoom(playUrl, roomId);
#else
        Q_UNUSED(roomId)
        qDebug() << "FFmpeg not available, cannot play stream";
#endif
    });

    m_pageStartLive = new StartLivePage(this);
    m_pageStartLive->setObjectName("pageStartLive");

    connect(m_pageStartLive, &StartLivePage::SIG_startLive,
        this, [this](const QString& title, const QString& category, int mode) {
#ifdef HAS_FFMPEG
            auto* client = Application::instance().httpClient();
            QJsonObject body;
            body["token"] = Application::instance().currentUser().token();
            body["title"] = title;
            body["category"] = category;
            body["mode"] = mode;

            client->post("/api/live/create", body, [this, mode](const ApiResponse& resp) {
                if (resp.isSuccess()) {
                    QJsonObject data = resp.data();
                    QString pushUrl = data["push_url"].toString();
                    int roomId = data["room_id"].toInt();
                    if (pushUrl.isEmpty()) {
                        QString serverIp = Application::instance().config()->serverAddress();
                        serverIp.replace("http://", "");
                        pushUrl = QString("rtmp://%1/live/stream_%2")
                            .arg(serverIp)
                            .arg(roomId);
                    }
                    showAnchorRoom(pushUrl, mode, roomId);
                } else {
                    qDebug() << "Create live failed:" << resp.msg();
                }
            });
#else
            Q_UNUSED(title)
            Q_UNUSED(category)
            Q_UNUSED(mode)
            qDebug() << "FFmpeg not available, cannot start live";
#endif
        });

#ifdef HAS_FFMPEG
    m_pageAnchorRoom = new AnchorRoomPage(this);
    m_pageAnchorRoom->setObjectName("pageAnchorRoom");

    connect(m_pageAnchorRoom, &AnchorRoomPage::SIG_stopLive, [this]() {
        auto* client = Application::instance().httpClient();
        QJsonObject body;
        body["token"] = Application::instance().currentUser().token();
        body["room_id"] = 0;
        client->post("/api/live/end", body, [](const ApiResponse& resp) {
            Q_UNUSED(resp)
        });
        switchPage(0);
    });

    m_pageLiveRoom = new LiveRoomPage(this);
    m_pageLiveRoom->setObjectName("pageLiveRoom");

    connect(m_pageLiveRoom, &LiveRoomPage::SIG_backToHall, [this]() {
        switchPage(0);
    });
#endif

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

#ifdef HAS_FFMPEG
    m_contentStack->addWidget(m_pageAnchorRoom);
    m_contentStack->addWidget(m_pageLiveRoom);
#endif
}

void MainWindow::switchPage(int index) {
    if (index < 0 || index >= 3) return;

    m_currentIndex = index;

    if (index == 0) {
        m_contentStack->setCurrentWidget(m_pageLiveHall);
    } else if (index == 1) {
        m_contentStack->setCurrentWidget(m_pageStartLive);
    } else if (index == 2) {
        m_contentStack->setCurrentWidget(m_pageProfile);
    }

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

void MainWindow::showLiveRoom(const QString& playUrl, int roomId) {
#ifdef HAS_FFMPEG
    m_contentStack->setCurrentWidget(m_pageLiveRoom);
    m_pageLiveRoom->enterRoom(playUrl, roomId);
    m_navigationBar->hide();
#else
    Q_UNUSED(playUrl)
    Q_UNUSED(roomId)
#endif
}

void MainWindow::showAnchorRoom(const QString& pushUrl, int mode, int roomId) {
#ifdef HAS_FFMPEG
    m_contentStack->setCurrentWidget(m_pageAnchorRoom);
    m_pageAnchorRoom->startLive(pushUrl, mode, roomId);
    m_navigationBar->hide();
#else
    Q_UNUSED(pushUrl)
    Q_UNUSED(mode)
    Q_UNUSED(roomId)
#endif
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
