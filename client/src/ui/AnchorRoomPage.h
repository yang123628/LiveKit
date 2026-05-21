#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QImage>

class OpenGLWidget;
class VideoPusher;
class CameraCapture;
class DesktopCapture;
class AudioCapture;
class PicInPic;
class PicInPicWidget;
class DanmakuWidget;
class WebSocketClient;
class GiftAnimation;
class FloatingHeartsWidget;

class AnchorRoomPage : public QWidget {
    Q_OBJECT

public:
    explicit AnchorRoomPage(QWidget* parent = nullptr);
    ~AnchorRoomPage();

    void startLive(const QString& pushUrl, int mode, int roomId);
    void stopLive();

signals:
    void SIG_stopLive();

private:
    void setupUI();
    void initCapture(int mode);
    void releaseCapture();
    void connectWebSocket();
    void disconnectWebSocket();

    OpenGLWidget* m_preview;
    DanmakuWidget* m_danmakuWidget;
    QLabel* m_viewerCountLabel;
    QLabel* m_likeCountLabel;
    QPushButton* m_stopButton;
    GiftAnimation* m_giftAnimation;
    FloatingHeartsWidget* m_floatingHearts;

    VideoPusher* m_pusher;
    CameraCapture* m_cameraCapture;
    DesktopCapture* m_desktopCapture;
    AudioCapture* m_audioCapture;
    PicInPic* m_picInPic;
    PicInPicWidget* m_pipWidget;
    WebSocketClient* m_webSocket;

    int m_liveMode;
    bool m_isLiving;
    int m_viewerCount;
    int m_likeCount;
    int m_roomId;
    QTimer* m_viewerTimer;
    QImage m_cameraPipFrame;
};
