#pragma once

#include <QThread>
#include <QImage>
#include <atomic>

class CameraCapture : public QThread {
    Q_OBJECT

public:
    explicit CameraCapture(QObject* parent = nullptr);
    ~CameraCapture() override;

    bool open(int deviceId = 0);
    void close();
    bool isOpened() const;

signals:
    void SIG_sendVideoFrame(QImage frame);

protected:
    void run() override;

private:
    void* m_capture;
    std::atomic<bool> m_running;
    int m_fps;
};
