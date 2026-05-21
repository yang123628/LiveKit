#pragma once

#include <QImage>

struct AVFrame;

class PicInPic {
public:
    PicInPic();
    ~PicInPic();

    QImage composite(const QImage& desktopFrame, const QImage& cameraFrame);
    AVFrame* qimageToYUV420P(const QImage& image);

    void setPipSize(int width, int height);
    void setPipPosition(int x, int y);

private:
    int m_pipWidth;
    int m_pipHeight;
    int m_pipX;
    int m_pipY;
    void* m_swsCtx;
    int m_lastWidth;
    int m_lastHeight;
};
