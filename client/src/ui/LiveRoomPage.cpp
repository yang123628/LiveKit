#include "ui/LiveRoomPage.h"
#include "ui/OpenGLWidget.h"
#include "ui/DanmakuWidget.h"
#include "core/VideoPlayer.h"
#include "network/WebSocketClient.h"
#include "app/Application.h"
#include <QDebug>

LiveRoomPage::LiveRoomPage(QWidget* parent)
    : QWidget(parent)
    , m_player(nullptr)
    , m_webSocket(nullptr)
    , m_roomId(0)
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

    auto* videoContainer = new QWidget(this);
    videoContainer->setObjectName("videoContainer");
    auto* videoLayout = new QVBoxLayout(videoContainer);
    videoLayout->setContentsMargins(0, 0, 0, 0);
    videoLayout->setSpacing(0);

    m_videoWidget = new OpenGLWidget(this);
    m_videoWidget->setObjectName("liveVideoWidget");
    videoLayout->addWidget(m_videoWidget, 1);

    m_danmakuWidget = new DanmakuWidget(this);
    m_danmakuWidget->setObjectName("danmakuOverlay");
    m_danmakuWidget->setFixedHeight(200);
    videoLayout->addWidget(m_danmakuWidget);

    contentLayout->addWidget(videoContainer, 1);

    mainLayout->addLayout(contentLayout, 1);

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
}

void LiveRoomPage::enterRoom(const QString& playUrl, int roomId) {
    m_playUrl = playUrl;
    m_roomId = roomId;

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
            Q_UNUSED(giftId)
            m_danmakuWidget->addDanmaku(
                QStringLiteral("系统"),
                QStringLiteral("%1 送出了 %2").arg(username).arg(giftName));
        });

    connect(m_webSocket, &WebSocketClient::likeReceived,
        this, [this](int count) {
            m_danmakuWidget->addDanmaku(
                QStringLiteral("系统"),
                QStringLiteral("点赞数: %1").arg(count));
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
