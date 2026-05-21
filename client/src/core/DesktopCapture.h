#pragma once

#include <QThread>
#include <QImage>
#include <atomic>

class DesktopCapture : public QThread {
    Q_OBJECT

public:
    explicit DesktopCapture(QObject* parent = nullptr);
    ~DesktopCapture() override;

    bool open();
    void close();
    bool isOpened() const;

signals:
    void SIG_sendVideoFrame(QImage frame);

protected:
    void run() override;

private:
    std::atomic<bool> m_running;
    int m_fps;
};
