#pragma once

#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class OpenGLWidget;
class VideoPlayer;

class LiveRoomPage : public QWidget {
    Q_OBJECT

public:
    explicit LiveRoomPage(QWidget* parent = nullptr);
    ~LiveRoomPage();

    void enterRoom(const QString& playUrl, int roomId);
    void leaveRoom();

signals:
    void SIG_backToHall();

private:
    void setupUI();

    OpenGLWidget* m_videoWidget;
    VideoPlayer* m_player;
    QPushButton* m_backButton;
    QPushButton* m_fullscreenButton;
    QSlider* m_volumeSlider;
    QLabel* m_volumeLabel;
    QWidget* m_controlBar;

    QString m_playUrl;
    int m_roomId;
    bool m_isFullscreen;
};
