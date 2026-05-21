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

class AnchorRoomPage : public QWidget {
    Q_OBJECT

public:
    explicit AnchorRoomPage(QWidget* parent = nullptr);
    ~AnchorRoomPage();

    void startLive(const QString& pushUrl, int mode);
    void stopLive();

signals:
    void SIG_stopLive();

private:
    void setupUI();
    void initCapture(int mode);
    void releaseCapture();

    OpenGLWidget* m_preview;
    QLabel* m_viewerCountLabel;
    QPushButton* m_stopButton;

    VideoPusher* m_pusher;
    CameraCapture* m_cameraCapture;
    DesktopCapture* m_desktopCapture;
    AudioCapture* m_audioCapture;
    PicInPic* m_picInPic;
    PicInPicWidget* m_pipWidget;

    int m_liveMode;
    bool m_isLiving;
    int m_viewerCount;
    QTimer* m_viewerTimer;
    QImage m_cameraPipFrame;
};
