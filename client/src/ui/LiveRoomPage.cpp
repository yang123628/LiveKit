#include "ui/LiveRoomPage.h"
#include "ui/OpenGLWidget.h"
#include "ui/DanmakuWidget.h"
#include "ui/GiftPanel.h"
#include "ui/GiftAnimation.h"
#include "ui/LikeButton.h"
#include "ui/FloatingHeartsWidget.h"
#include "core/VideoPlayer.h"
#include "network/WebSocketClient.h"
#include "app/Application.h"
#include <QDebug>

LiveRoomPage::LiveRoomPage(QWidget* parent)
    : QWidget(parent)
    , m_player(nullptr)
    , m_webSocket(nullptr)
    , m_giftPanel(nullptr)
    , m_roomId(0)
    , m_likeCount(0)
    , m_isFullscreen(false)
{
    setupUI();
}

LiveRoomPage::~LiveRoomPage() {
    leaveRoom();
}

void LiveRoomPage::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto* contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    m_videoContainer = new QWidget(this);
    m_videoContainer->setObjectName("videoContainer");
    auto* videoLayout = new QVBoxLayout(m_videoContainer);
    videoLayout->setContentsMargins(0, 0, 0, 0);
    videoLayout->setSpacing(0);

    m_videoWidget = new OpenGLWidget(this);
    m_videoWidget->setObjectName("liveVideoWidget");
    videoLayout->addWidget(m_videoWidget, 1);

    m_danmakuWidget = new DanmakuWidget(this);
    m_danmakuWidget->setObjectName("danmakuOverlay");
    m_danmakuWidget->setFixedHeight(200);
    videoLayout->addWidget(m_danmakuWidget);

    contentLayout->addWidget(m_videoContainer, 1);

    auto* rightOverlay = new QWidget(this);
    rightOverlay->setObjectName("rightOverlay");
    rightOverlay->setFixedWidth(60);
    rightOverlay->setAttribute(Qt::WA_TranslucentBackground);
    auto* rightLayout = new QVBoxLayout(rightOverlay);
    rightLayout->setContentsMargins(4, 10, 4, 10);
    rightLayout->setSpacing(8);

    m_likeButton = new LikeButton(this);
    rightLayout->addWidget(m_likeButton, 0, Qt::AlignHCenter);

    m_likeCountLabel = new QLabel(QStringLiteral("0"), this);
    m_likeCountLabel->setObjectName("likeCountLabel");
    m_likeCountLabel->setAlignment(Qt::AlignCenter);
    m_likeCountLabel->setFixedHeight(20);
    rightLayout->addWidget(m_likeCountLabel, 0, Qt::AlignHCenter);

    rightLayout->addStretch();

    m_floatingHearts = new FloatingHeartsWidget(this);
    m_floatingHearts->setObjectName("floatingHearts");
    m_floatingHearts->setFixedWidth(80);
    m_floatingHearts->setMinimumHeight(200);
    rightLayout->addWidget(m_floatingHearts, 1);

    contentLayout->addWidget(rightOverlay);

    mainLayout->addLayout(contentLayout, 1);

    m_giftAnimation = new GiftAnimation(m_videoContainer);
    m_giftAnimation->setObjectName("giftAnimation");
    m_giftAnimation->move(10, 10);
    m_giftAnimation->raise();

    m_danmakuInputBar = new QWidget(this);
    m_danmakuInputBar->setObjectName("danmakuInputBar");
    m_danmakuInputBar->setFixedHeight(44);
    auto* inputLayout = new QHBoxLayout(m_danmakuInputBar);
    inputLayout->setContentsMargins(12, 4, 12, 4);
    inputLayout->setSpacing(8);

    m_danmakuInput = new QLineEdit(this);
    m_danmakuInput->setObjectName("danmakuInput");
    m_danmakuInput->setPlaceholderText(QStringLiteral("说点什么..."));
    m_danmakuInput->setMaxLength(200);
    inputLayout->addWidget(m_danmakuInput, 1);

    m_sendButton = new QPushButton(QStringLiteral("发送"), this);
    m_sendButton->setObjectName("danmakuSendButton");
    m_sendButton->setFixedSize(60, 32);
    inputLayout->addWidget(m_sendButton);

    m_giftButton = new QPushButton(QStringLiteral("🎁"), this);
    m_giftButton->setObjectName("giftOpenButton");
    m_giftButton->setFixedSize(44, 32);
    m_giftButton->setToolTip(QStringLiteral("送礼物"));
    inputLayout->addWidget(m_giftButton);

    mainLayout->addWidget(m_danmakuInputBar);

    m_controlBar = new QWidget(this);
    m_controlBar->setObjectName("liveControlBar");
    m_controlBar->setFixedHeight(50);
    auto* controlLayout = new QHBoxLayout(m_controlBar);
    controlLayout->setContentsMargins(16, 0, 16, 0);

    m_backButton = new QPushButton(QStringLiteral("返回"), this);
    m_backButton->setObjectName("backButton");
    m_backButton->setFixedSize(70, 32);
    controlLayout->addWidget(m_backButton);

    controlLayout->addStretch();

    m_volumeLabel = new QLabel(QStringLiteral("音量"), this);
    m_volumeLabel->setObjectName("volumeLabel");
    controlLayout->addWidget(m_volumeLabel);

    m_volumeSlider = new QSlider(Qt::Horizontal, this);
    m_volumeSlider->setObjectName("volumeSlider");
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(80);
    m_volumeSlider->setFixedWidth(120);
    controlLayout->addWidget(m_volumeSlider);

    m_fullscreenButton = new QPushButton(QStringLiteral("全屏"), this);
    m_fullscreenButton->setObjectName("fullscreenButton");
    m_fullscreenButton->setFixedSize(70, 32);
    controlLayout->addWidget(m_fullscreenButton);

    mainLayout->addWidget(m_controlBar);

    m_giftPanel = new GiftPanel(this);

    connect(m_backButton, &QPushButton::clicked, [this]() {
        leaveRoom();
        emit SIG_backToHall();
    });

    connect(m_volumeSlider, &QSlider::valueChanged, [this](int value) {
        if (m_player) {
            m_player->setVolume(value);
        }
    });

    connect(m_fullscreenButton, &QPushButton::clicked, [this]() {
        if (m_isFullscreen) {
            setWindowState(windowState() & ~Qt::WindowFullScreen);
            m_fullscreenButton->setText(QStringLiteral("全屏"));
            m_danmakuInputBar->show();
            m_controlBar->show();
        } else {
            setWindowState(windowState() | Qt::WindowFullScreen);
            m_fullscreenButton->setText(QStringLiteral("退出全屏"));
        }
        m_isFullscreen = !m_isFullscreen;
    });

    auto sendDanmaku = [this]() {
        QString text = m_danmakuInput->text().trimmed();
        if (text.isEmpty()) return;
        if (m_webSocket && m_webSocket->isConnected()) {
            m_webSocket->sendDanmaku(text);
        }
        m_danmakuWidget->addDanmaku(
            Application::instance().currentUser().username(), text);
        m_danmakuInput->clear();
    };

    connect(m_sendButton, &QPushButton::clicked, sendDanmaku);
    connect(m_danmakuInput, &QLineEdit::returnPressed, sendDanmaku);

    connect(m_giftButton, &QPushButton::clicked, [this]() {
        if (m_giftPanel->isPanelVisible()) {
            m_giftPanel->hidePanel();
        } else {
            QPoint pos = m_giftButton->mapToGlobal(QPoint(0, -m_giftPanel->height()));
            m_giftPanel->move(pos);
            m_giftPanel->showPanel();
        }
    });

    connect(m_giftPanel, &GiftPanel::SIG_giftSelected, this, [this](int giftId) {
        if (m_webSocket && m_webSocket->isConnected()) {
            m_webSocket->sendGift(giftId);
        }
        m_giftAnimation->showGift(
            Application::instance().currentUser().username(), giftId);
    });

    connect(m_likeButton, &LikeButton::clicked, this, [this]() {
        if (m_webSocket && m_webSocket->isConnected()) {
            m_webSocket->sendLike();
        }
        m_likeCount++;
        m_likeCountLabel->setText(QString::number(m_likeCount));
        m_floatingHearts->addHeart();
    });
}

void LiveRoomPage::enterRoom(const QString& playUrl, int roomId) {
    m_playUrl = playUrl;
    m_roomId = roomId;
    m_likeCount = 0;
    m_likeCountLabel->setText(QStringLiteral("0"));

    if (m_player) {
        m_player->stop();
        delete m_player;
    }

    m_player = new VideoPlayer(this);
    connect(m_player, &VideoPlayer::SIG_frameReady, m_videoWidget, &OpenGLWidget::updateFrame);
    connect(m_player, &VideoPlayer::SIG_error, [](const QString& msg) {
        qDebug() << "VideoPlayer error:" << msg;
    });

    m_player->setVolume(m_volumeSlider->value());
    m_player->play(playUrl);

    connectWebSocket();
}

void LiveRoomPage::leaveRoom() {
    disconnectWebSocket();

    if (m_player) {
        m_player->stop();
        delete m_player;
        m_player = nullptr;
    }
    m_videoWidget->clearFrame();
    m_danmakuWidget->clearDanmaku();
    m_playUrl.clear();
    m_roomId = 0;
    m_likeCount = 0;
}

void LiveRoomPage::connectWebSocket() {
    disconnectWebSocket();

    m_webSocket = new WebSocketClient(this);

    connect(m_webSocket, &WebSocketClient::danmakuReceived,
        this, [this](const QString& username, const QString& content) {
            m_danmakuWidget->addDanmaku(username, content);
        });

    connect(m_webSocket, &WebSocketClient::giftReceived,
        this, [this](const QString& username, int giftId, const QString& giftName) {
            m_danmakuWidget->addDanmaku(
                QStringLiteral("系统"),
                QStringLiteral("%1 送出了 %2").arg(username).arg(giftName));
            m_giftAnimation->showGift(username, giftId);
        });

    connect(m_webSocket, &WebSocketClient::likeReceived,
        this, [this](int count) {
            m_likeCount = count;
            m_likeCountLabel->setText(QString::number(count));
            m_floatingHearts->addHeart();
        });

    connect(m_webSocket, &WebSocketClient::viewerCountChanged,
        this, [](int count) {
            qDebug() << "Viewer count:" << count;
        });

    connect(m_webSocket, &WebSocketClient::viewerJoined,
        this, [this](const QString& username) {
            m_danmakuWidget->addDanmaku(
                QStringLiteral("系统"),
                QStringLiteral("%1 进入了直播间").arg(username));
        });

    connect(m_webSocket, &WebSocketClient::error,
        this, [](const QString& msg) {
            qDebug() << "WebSocket error:" << msg;
        });

    QString token = Application::instance().currentUser().token();
    m_webSocket->connectToServer(token, m_roomId);
}

void LiveRoomPage::disconnectWebSocket() {
    if (m_webSocket) {
        m_webSocket->disconnectFromServer();
        delete m_webSocket;
        m_webSocket = nullptr;
    }
}
