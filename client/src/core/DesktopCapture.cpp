#include "core/DesktopCapture.h"
#include <QScreen>
#include <QGuiApplication>
#include <QPixmap>
#include <QElapsedTimer>

DesktopCapture::DesktopCapture(QObject* parent)
    : QThread(parent)
    , m_running(false)
    , m_fps(25)
{
}

DesktopCapture::~DesktopCapture() {
    close();
    wait();
}

bool DesktopCapture::open() {
    if (m_running) return false;
    m_running = true;
    start();
    return true;
}

void DesktopCapture::close() {
    m_running = false;
    wait();
}

bool DesktopCapture::isOpened() const {
    return m_running;
}

void DesktopCapture::run() {
    QScreen* screen = QGuiApplication::primaryScreen();
    if (!screen) return;

    qint64 frameInterval = 1000 / m_fps;
    QElapsedTimer timer;

    while (m_running) {
        timer.start();

        QScreen* currentScreen = QGuiApplication::primaryScreen();
        if (!currentScreen) {
            msleep(100);
            continue;
        }

        QPixmap pixmap = currentScreen->grabWindow(0);
        if (pixmap.isNull()) {
            msleep(10);
            continue;
        }

        QImage frame = pixmap.toImage().convertToFormat(QImage::Format_RGB888);
        emit SIG_sendVideoFrame(frame);

        qint64 elapsed = timer.elapsed();
        if (elapsed < frameInterval) {
            msleep(static_cast<unsigned long>(frameInterval - elapsed));
        }
    }
}
