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
class GiftPanel;
class GiftAnimation;
class LikeButton;
class FloatingHeartsWidget;

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
    QPushButton* m_giftButton;
    GiftPanel* m_giftPanel;
    GiftAnimation* m_giftAnimation;
    LikeButton* m_likeButton;
    FloatingHeartsWidget* m_floatingHearts;
    QLabel* m_likeCountLabel;
    QSlider* m_volumeSlider;
    QLabel* m_volumeLabel;
    QWidget* m_controlBar;
    QWidget* m_danmakuInputBar;
    QWidget* m_videoContainer;

    QString m_playUrl;
    int m_roomId;
    int m_likeCount;
    bool m_isFullscreen;
};
