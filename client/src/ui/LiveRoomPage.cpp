#include "ui/LiveRoomPage.h"
#include "ui/OpenGLWidget.h"
#include "core/VideoPlayer.h"
#include <QDebug>

LiveRoomPage::LiveRoomPage(QWidget* parent)
    : QWidget(parent)
    , m_player(nullptr)
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

    m_videoWidget = new OpenGLWidget(this);
    m_videoWidget->setObjectName("liveVideoWidget");
    mainLayout->addWidget(m_videoWidget, 1);

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
            m_controlBar->show();
        } else {
            setWindowState(windowState() | Qt::WindowFullScreen);
            m_fullscreenButton->setText(QStringLiteral("退出全屏"));
        }
        m_isFullscreen = !m_isFullscreen;
    });
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
}

void LiveRoomPage::leaveRoom() {
    if (m_player) {
        m_player->stop();
        delete m_player;
        m_player = nullptr;
    }
    m_videoWidget->clearFrame();
    m_playUrl.clear();
    m_roomId = 0;
}
