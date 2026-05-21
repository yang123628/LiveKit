#include "core/PicInPic.h"
#include <QPainter>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

PicInPic::PicInPic()
    : m_pipWidth(320)
    , m_pipHeight(240)
    , m_pipX(-1)
    , m_pipY(-1)
    , m_swsCtx(nullptr)
    , m_lastWidth(0)
    , m_lastHeight(0)
{
}

PicInPic::~PicInPic() {
    if (m_swsCtx) {
        sws_freeContext(static_cast<SwsContext*>(m_swsCtx));
        m_swsCtx = nullptr;
    }
}

QImage PicInPic::composite(const QImage& desktopFrame, const QImage& cameraFrame) {
    QImage result = desktopFrame.copy();
    QImage scaledCamera = cameraFrame.scaled(m_pipWidth, m_pipHeight,
        Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    int x = (m_pipX < 0) ? (result.width() - m_pipWidth - 20) : m_pipX;
    int y = (m_pipY < 0) ? (result.height() - m_pipHeight - 20) : m_pipY;

    QPainter painter(&result);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 160));
    painter.drawRoundedRect(x - 2, y - 2, scaledCamera.width() + 4, scaledCamera.height() + 4, 6, 6);
    painter.drawImage(x, y, scaledCamera);
    painter.end();

    return result;
}

AVFrame* PicInPic::qimageToYUV420P(const QImage& image) {
    QImage rgb = image.convertToFormat(QImage::Format_RGB888);

    int w = rgb.width();
    int h = rgb.height();

    if (m_lastWidth != w || m_lastHeight != h) {
        if (m_swsCtx) {
            sws_freeContext(static_cast<SwsContext*>(m_swsCtx));
        }
        m_swsCtx = sws_getContext(w, h, AV_PIX_FMT_RGB24,
            w, h, AV_PIX_FMT_YUV420P,
            SWS_BILINEAR, nullptr, nullptr, nullptr);
        m_lastWidth = w;
        m_lastHeight = h;
    }

    AVFrame* frame = av_frame_alloc();
    frame->format = AV_PIX_FMT_YUV420P;
    frame->width = w;
    frame->height = h;
    av_frame_get_buffer(frame, 32);

    const uint8_t* srcData[1] = {rgb.constBits()};
    int srcLineSize[1] = {rgb.bytesPerLine()};

    sws_scale(static_cast<SwsContext*>(m_swsCtx),
        srcData, srcLineSize, 0, h,
        frame->data, frame->linesize);

    return frame;
}

void PicInPic::setPipSize(int width, int height) {
    m_pipWidth = width;
    m_pipHeight = height;
}

void PicInPic::setPipPosition(int x, int y) {
    m_pipX = x;
    m_pipY = y;
}
