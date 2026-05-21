#include "ui/AnchorRoomPage.h"
#include "ui/OpenGLWidget.h"
#include "ui/DanmakuWidget.h"
#include "ui/GiftAnimation.h"
#include "ui/FloatingHeartsWidget.h"
#include "core/VideoPusher.h"
#include "core/CameraCapture.h"
#include "core/DesktopCapture.h"
#include "core/AudioCapture.h"
#include "core/PicInPic.h"
#include "ui/PicInPicWidget.h"
#include "network/WebSocketClient.h"
#include "app/Application.h"
#include <QDebug>

AnchorRoomPage::AnchorRoomPage(QWidget* parent)
    : QWidget(parent)
    , m_pusher(nullptr)
    , m_cameraCapture(nullptr)
    , m_desktopCapture(nullptr)
    , m_audioCapture(nullptr)
    , m_picInPic(nullptr)
    , m_pipWidget(nullptr)
    , m_webSocket(nullptr)
    , m_liveMode(0)
    , m_isLiving(false)
    , m_viewerCount(0)
    , m_likeCount(0)
    , m_roomId(0)
{
    setupUI();

    m_viewerTimer = new QTimer(this);
    connect(m_viewerTimer, &QTimer::timeout, [this]() {
        m_viewerCount += qrand() % 3;
        m_viewerCountLabel->setText(QStringLiteral("在线: %1").arg(m_viewerCount));
    });
}

AnchorRoomPage::~AnchorRoomPage() {
    stopLive();
}

void AnchorRoomPage::setupUI() {
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto* leftPanel = new QWidget(this);
    auto* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);

    m_preview = new OpenGLWidget(this);
    m_preview->setObjectName("anchorPreview");
    leftLayout->addWidget(m_preview, 1);

    m_giftAnimation = new GiftAnimation(m_preview);
    m_giftAnimation->setObjectName("giftAnimation");
    m_giftAnimation->move(10, 10);
    m_giftAnimation->raise();

    auto* controlBar = new QWidget(this);
    controlBar->setObjectName("anchorControlBar");
    controlBar->setFixedHeight(60);
    auto* controlLayout = new QHBoxLayout(controlBar);
    controlLayout->setContentsMargins(20, 0, 20, 0);

    m_viewerCountLabel = new QLabel(QStringLiteral("在线: 0"), this);
    m_viewerCountLabel->setObjectName("viewerCountLabel");
    controlLayout->addWidget(m_viewerCountLabel);

    m_likeCountLabel = new QLabel(QStringLiteral("♥ 0"), this);
    m_likeCountLabel->setObjectName("likeCountLabel");
    controlLayout->addWidget(m_likeCountLabel);

    controlLayout->addStretch();

    m_stopButton = new QPushButton(QStringLiteral("停止直播"), this);
    m_stopButton->setObjectName("stopLiveButton");
    m_stopButton->setFixedSize(120, 36);
    controlLayout->addWidget(m_stopButton);

    leftLayout->addWidget(controlBar);

    mainLayout->addWidget(leftPanel, 1);

    auto* rightPanel = new QWidget(this);
    rightPanel->setObjectName("anchorDanmakuPanel");
    rightPanel->setFixedWidth(300);
    auto* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    auto* danmakuTitle = new QLabel(QStringLiteral("弹幕区"), rightPanel);
    danmakuTitle->setObjectName("danmakuTitle");
    danmakuTitle->setAlignment(Qt::AlignCenter);
    danmakuTitle->setFixedHeight(40);
    rightLayout->addWidget(danmakuTitle);

    m_danmakuWidget = new DanmakuWidget(rightPanel);
    m_danmakuWidget->setObjectName("anchorDanmaku");
    rightLayout->addWidget(m_danmakuWidget, 1);

    m_floatingHearts = new FloatingHeartsWidget(rightPanel);
    m_floatingHearts->setObjectName("floatingHearts");
    m_floatingHearts->setFixedHeight(120);
    rightLayout->addWidget(m_floatingHearts);

    mainLayout->addWidget(rightPanel);

    connect(m_stopButton, &QPushButton::clicked, [this]() {
        stopLive();
        emit SIG_stopLive();
    });
}

void AnchorRoomPage::startLive(const QString& pushUrl, int mode, int roomId) {
    m_liveMode = mode;
    m_isLiving = true;
    m_viewerCount = 0;
    m_roomId = roomId;

    m_pusher = new VideoPusher(this);
    if (!m_pusher->start(pushUrl)) {
        qDebug() << "Failed to start pusher";
        delete m_pusher;
        m_pusher = nullptr;
        m_isLiving = false;
        return;
    }

    initCapture(mode);
    connectWebSocket();
    m_viewerTimer->start(5000);
}

void AnchorRoomPage::stopLive() {
    if (!m_isLiving) return;
    m_isLiving = false;

    m_viewerTimer->stop();
    disconnectWebSocket();
    releaseCapture();

    if (m_pusher) {
        m_pusher->stop();
        delete m_pusher;
        m_pusher = nullptr;
    }

    m_preview->clearFrame();
    m_danmakuWidget->clearDanmaku();
    m_viewerCountLabel->setText(QStringLiteral("在线: 0"));
    m_likeCount = 0;
    m_likeCountLabel->setText(QStringLiteral("♥ 0"));
}

void AnchorRoomPage::initCapture(int mode) {
    m_audioCapture = new AudioCapture(this);
    connect(m_audioCapture, &AudioCapture::SIG_sendAudioFrameData,
        m_pusher, &VideoPusher::pushAudioFrame);
    m_audioCapture->open();

    if (mode == 0) {
        m_cameraCapture = new CameraCapture(this);
        connect(m_cameraCapture, &CameraCapture::SIG_sendVideoFrame,
            this, [this](QImage frame) {
                m_preview->updateFrame(frame);
                m_pusher->pushVideoFrame(frame);
            });
        m_cameraCapture->open();
    } else if (mode == 1) {
        m_desktopCapture = new DesktopCapture(this);
        connect(m_desktopCapture, &DesktopCapture::SIG_sendVideoFrame,
            this, [this](QImage frame) {
                m_preview->updateFrame(frame);
                m_pusher->pushVideoFrame(frame);
            });
        m_desktopCapture->open();
    } else if (mode == 2) {
        m_picInPic = new PicInPic();

        m_cameraCapture = new CameraCapture(this);
        m_desktopCapture = new DesktopCapture(this);

        m_pipWidget = new PicInPicWidget();

        connect(m_cameraCapture, &CameraCapture::SIG_sendVideoFrame,
            this, [this](QImage frame) {
                if (m_pipWidget) {
                    m_pipWidget->updateFrame(frame);
                }
                m_cameraPipFrame = frame;
            });

        connect(m_desktopCapture, &DesktopCapture::SIG_sendVideoFrame,
            this, [this](QImage frame) {
                if (m_picInPic && !m_cameraPipFrame.isNull()) {
                    QImage composed = m_picInPic->composite(frame, m_cameraPipFrame);
                    m_preview->updateFrame(composed);
                    m_pusher->pushVideoFrame(composed);
                } else {
                    m_preview->updateFrame(frame);
                    m_pusher->pushVideoFrame(frame);
                }
            });

        m_cameraCapture->open();
        m_desktopCapture->open();
        m_pipWidget->show();
    }
}

void AnchorRoomPage::releaseCapture() {
    if (m_cameraCapture) {
        m_cameraCapture->close();
        delete m_cameraCapture;
        m_cameraCapture = nullptr;
    }
    if (m_desktopCapture) {
        m_desktopCapture->close();
        delete m_desktopCapture;
        m_desktopCapture = nullptr;
    }
    if (m_audioCapture) {
        m_audioCapture->close();
        delete m_audioCapture;
        m_audioCapture = nullptr;
    }
    if (m_picInPic) {
        delete m_picInPic;
        m_picInPic = nullptr;
    }
    if (m_pipWidget) {
        m_pipWidget->close();
        delete m_pipWidget;
        m_pipWidget = nullptr;
    }
}

void AnchorRoomPage::connectWebSocket() {
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
            m_likeCountLabel->setText(QStringLiteral("♥ %1").arg(count));
            m_floatingHearts->addHeart();
        });

    connect(m_webSocket, &WebSocketClient::viewerCountChanged,
        this, [this](int count) {
            m_viewerCount = count;
            m_viewerCountLabel->setText(QStringLiteral("在线: %1").arg(count));
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

void AnchorRoomPage::disconnectWebSocket() {
    if (m_webSocket) {
        m_webSocket->disconnectFromServer();
        delete m_webSocket;
        m_webSocket = nullptr;
    }
}
