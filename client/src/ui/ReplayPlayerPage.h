#pragma once

#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>

class OpenGLWidget;
class VideoPlayer;

class ReplayPlayerPage : public QWidget {
    Q_OBJECT

public:
    explicit ReplayPlayerPage(QWidget* parent = nullptr);
    ~ReplayPlayerPage();

    void play(const QString& url);
    void stop();

signals:
    void SIG_backToHall();

private:
    void setupUI();
    void setupConnections();
    QString formatTime(int64_t ms) const;

    OpenGLWidget* m_videoWidget;
    VideoPlayer* m_player;
    QPushButton* m_backButton;
    QPushButton* m_playPauseButton;
    QPushButton* m_fullscreenButton;
    QSlider* m_progressSlider;
    QLabel* m_timeLabel;
    QWidget* m_controlBar;

    int64_t m_duration;
    int64_t m_position;
    bool m_isFullscreen;
    bool m_sliderPressed;
    QTimer* m_updateTimer;
};
