#include "ui/ReplayPlayerPage.h"
#include "ui/OpenGLWidget.h"
#include "core/VideoPlayer.h"
#include <QDebug>

ReplayPlayerPage::ReplayPlayerPage(QWidget* parent)
    : QWidget(parent)
    , m_player(nullptr)
    , m_duration(0)
    , m_position(0)
    , m_isFullscreen(false)
    , m_sliderPressed(false)
{
    setupUI();
    setupConnections();

    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(200);
    connect(m_updateTimer, &QTimer::timeout, this, [this]() {
        if (m_player && m_player->isPlaying() && !m_sliderPressed) {
            m_progressSlider->setMaximum(static_cast<int>(m_duration / 1000));
            m_progressSlider->setValue(static_cast<int>(m_position / 1000));
            m_timeLabel->setText(formatTime(m_position) + " / " + formatTime(m_duration));
        }
    });
}

ReplayPlayerPage::~ReplayPlayerPage() {
    stop();
}

void ReplayPlayerPage::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_videoWidget = new OpenGLWidget(this);
    m_videoWidget->setObjectName("replayVideoWidget");
    mainLayout->addWidget(m_videoWidget, 1);

    m_controlBar = new QWidget(this);
    m_controlBar->setObjectName("replayControlBar");
    m_controlBar->setFixedHeight(70);
    auto* controlLayout = new QVBoxLayout(m_controlBar);
    controlLayout->setContentsMargins(16, 4, 16, 8);
    controlLayout->setSpacing(4);

    m_progressSlider = new QSlider(Qt::Horizontal, this);
    m_progressSlider->setObjectName("replayProgressSlider");
    m_progressSlider->setRange(0, 0);
    m_progressSlider->setValue(0);
    controlLayout->addWidget(m_progressSlider);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);

    m_backButton = new QPushButton(QStringLiteral("返回"), this);
    m_backButton->setObjectName("backButton");
    m_backButton->setFixedSize(70, 32);
    buttonLayout->addWidget(m_backButton);

    m_playPauseButton = new QPushButton(QStringLiteral("暂停"), this);
    m_playPauseButton->setObjectName("playPauseButton");
    m_playPauseButton->setFixedSize(70, 32);
    buttonLayout->addWidget(m_playPauseButton);

    m_timeLabel = new QLabel(QStringLiteral("00:00 / 00:00"), this);
    m_timeLabel->setObjectName("replayTimeLabel");
    m_timeLabel->setFixedHeight(32);
    buttonLayout->addWidget(m_timeLabel);

    buttonLayout->addStretch();

    m_fullscreenButton = new QPushButton(QStringLiteral("全屏"), this);
    m_fullscreenButton->setObjectName("fullscreenButton");
    m_fullscreenButton->setFixedSize(70, 32);
    buttonLayout->addWidget(m_fullscreenButton);

    controlLayout->addLayout(buttonLayout);

    mainLayout->addWidget(m_controlBar);
}

void ReplayPlayerPage::setupConnections() {
    connect(m_backButton, &QPushButton::clicked, this, [this]() {
        stop();
        emit SIG_backToHall();
    });

    connect(m_playPauseButton, &QPushButton::clicked, this, [this]() {
        if (!m_player) return;
        if (m_player->isPlaying()) {
            m_player->pause();
            m_playPauseButton->setText(QStringLiteral("继续"));
        } else {
            m_player->resume();
            m_playPauseButton->setText(QStringLiteral("暂停"));
        }
    });

    connect(m_fullscreenButton, &QPushButton::clicked, this, [this]() {
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

    connect(m_progressSlider, &QSlider::sliderPressed, this, [this]() {
        m_sliderPressed = true;
    });

    connect(m_progressSlider, &QSlider::sliderReleased, this, [this]() {
        if (m_player) {
            int64_t seekMs = static_cast<int64_t>(m_progressSlider->value()) * 1000;
            m_player->seek(seekMs);
        }
        m_sliderPressed = false;
    });

    connect(m_progressSlider, &QSlider::valueChanged, this, [this](int value) {
        if (m_sliderPressed) {
            int64_t posMs = static_cast<int64_t>(value) * 1000;
            m_timeLabel->setText(formatTime(posMs) + " / " + formatTime(m_duration));
        }
    });
}

void ReplayPlayerPage::play(const QString& url) {
    stop();

    m_player = new VideoPlayer(this);
    connect(m_player, &VideoPlayer::SIG_frameReady, m_videoWidget, &OpenGLWidget::updateFrame);
    connect(m_player, &VideoPlayer::SIG_error, this, [](const QString& msg) {
        qDebug() << "ReplayPlayer error:" << msg;
    });
    connect(m_player, &VideoPlayer::SIG_durationChanged, this, [this](int64_t ms) {
        m_duration = ms;
        m_progressSlider->setMaximum(static_cast<int>(ms / 1000));
        m_timeLabel->setText(formatTime(0) + " / " + formatTime(ms));
    });
    connect(m_player, &VideoPlayer::SIG_positionChanged, this, [this](int64_t ms) {
        m_position = ms;
    });

    m_player->play(url);
    m_playPauseButton->setText(QStringLiteral("暂停"));
    m_updateTimer->start();
}

void ReplayPlayerPage::stop() {
    m_updateTimer->stop();

    if (m_player) {
        m_player->stop();
        delete m_player;
        m_player = nullptr;
    }

    m_videoWidget->clearFrame();
    m_duration = 0;
    m_position = 0;
    m_progressSlider->setRange(0, 0);
    m_progressSlider->setValue(0);
    m_timeLabel->setText(QStringLiteral("00:00 / 00:00"));
    m_playPauseButton->setText(QStringLiteral("暂停"));
}

QString ReplayPlayerPage::formatTime(int64_t ms) const {
    int totalSeconds = static_cast<int>(ms / 1000);
    int h = totalSeconds / 3600;
    int m = (totalSeconds % 3600) / 60;
    int s = totalSeconds % 60;
    if (h > 0) {
        return QString("%1:%2:%3")
            .arg(h)
            .arg(m, 2, 10, QChar('0'))
            .arg(s, 2, 10, QChar('0'));
    }
    return QString("%1:%2")
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0'));
}
