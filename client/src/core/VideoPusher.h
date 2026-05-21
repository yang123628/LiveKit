#pragma once

#include <QObject>
#include <QImage>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>
#include <QThread>
#include <atomic>

struct AVFormatContext;
struct AVCodecContext;
struct AVStream;
struct AVFrame;
struct AVPacket;
struct SwsContext;
struct SwrContext;

class VideoPusher : public QObject {
    Q_OBJECT

public:
    explicit VideoPusher(QObject* parent = nullptr);
    ~VideoPusher();

    bool start(const QString& rtmpUrl, int width = 1280, int height = 720,
               int fps = 25, int bitRate = 2500000);
    void stop();
    bool isRunning() const;

    void pushVideoFrame(const QImage& frame);
    void pushAudioFrame(const uint8_t* data, int size);

signals:
    void SIG_error(const QString& msg);
    void SIG_stateChanged(bool running);

private:
    bool initVideoCodec(int width, int height, int fps, int bitRate);
    bool initAudioCodec();
    bool initMuxer(const QString& rtmpUrl);
    void encodeVideoLoop();
    void encodeAudioLoop();
    void cleanup();

    AVFormatContext* m_fmtCtx;
    AVCodecContext* m_videoCodecCtx;
    AVCodecContext* m_audioCodecCtx;
    AVStream* m_videoStream;
    AVStream* m_audioStream;
    SwsContext* m_swsCtx;
    SwrContext* m_swrCtx;

    QThread* m_videoThread;
    QThread* m_audioThread;

    QQueue<QImage> m_videoQueue;
    QQueue<QByteArray> m_audioQueue;
    QMutex m_videoMutex;
    QMutex m_audioMutex;
    QWaitCondition m_videoCond;
    QWaitCondition m_audioCond;

    std::atomic<bool> m_running;
    std::atomic<int64_t> m_videoPts;
    std::atomic<int64_t> m_audioPts;

    int m_width;
    int m_height;
    int m_fps;
    int m_sampleRate;
    int m_channels;
    int m_maxQueueSize;
};
