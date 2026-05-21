#pragma once

#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>

class OpenGLWidget;
class VideoPlayer;
class DanmakuWidget;
class WebSocketClient;

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
    void connectWebSocket();
    void disconnectWebSocket();

    OpenGLWidget* m_videoWidget;
    VideoPlayer* m_player;
    DanmakuWidget* m_danmakuWidget;
    WebSocketClient* m_webSocket;
    QLineEdit* m_danmakuInput;
    QPushButton* m_sendButton;
    QPushButton* m_backButton;
    QPushButton* m_fullscreenButton;
    QSlider* m_volumeSlider;
    QLabel* m_volumeLabel;
    QWidget* m_controlBar;
    QWidget* m_danmakuInputBar;

    QString m_playUrl;
    int m_roomId;
    bool m_isFullscreen;
};
