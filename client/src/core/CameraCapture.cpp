#include "core/CameraCapture.h"
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <QElapsedTimer>

CameraCapture::CameraCapture(QObject* parent)
    : QThread(parent)
    , m_capture(nullptr)
    , m_running(false)
    , m_fps(25)
{
}

CameraCapture::~CameraCapture() {
    close();
    wait();
}

bool CameraCapture::open(int deviceId) {
    if (m_running) return false;

    auto* cap = new cv::VideoCapture(deviceId, cv::CAP_DSHOW);
    if (!cap->isOpened()) {
        delete cap;
        return false;
    }

    cap->set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    cap->set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    cap->set(cv::CAP_PROP_FPS, m_fps);

    m_capture = cap;
    m_running = true;
    start();
    return true;
}

void CameraCapture::close() {
    m_running = false;
    wait();
    if (m_capture) {
        auto* cap = static_cast<cv::VideoCapture*>(m_capture);
        cap->release();
        delete cap;
        m_capture = nullptr;
    }
}

bool CameraCapture::isOpened() const {
    return m_capture != nullptr && m_running;
}

void CameraCapture::run() {
    auto* cap = static_cast<cv::VideoCapture*>(m_capture);
    cv::Mat frame;
    QElapsedTimer timer;
    qint64 frameInterval = 1000 / m_fps;

    while (m_running) {
        timer.start();

        if (!cap->read(frame) || frame.empty()) {
            msleep(10);
            continue;
        }

        cv::Mat rgb;
        cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
        QImage qimg(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888);
        QImage copy = qimg.copy();

        emit SIG_sendVideoFrame(copy);

        qint64 elapsed = timer.elapsed();
        if (elapsed < frameInterval) {
            msleep(static_cast<unsigned long>(frameInterval - elapsed));
        }
    }
}
