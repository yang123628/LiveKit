#pragma once

#include <QObject>
#include <QThread>
#include <QImage>
#include <QMutex>
#include <QWaitCondition>
#include <atomic>

struct AVFormatContext;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
struct SwsContext;
struct SwrContext;

class PacketQueue;

class VideoPlayer : public QObject {
    Q_OBJECT

public:
    explicit VideoPlayer(QObject* parent = nullptr);
    ~VideoPlayer();

    bool play(const QString& url);
    void pause();
    void resume();
    void stop();
    void seek(int64_t ms);
    bool isPlaying() const;
    void setVolume(int volume);
    int volume() const;

signals:
    void SIG_frameReady(QImage frame);
    void SIG_stateChanged(bool playing);
    void SIG_error(const QString& msg);
    void SIG_positionChanged(int64_t ms);
    void SIG_durationChanged(int64_t ms);

private:
    void demuxThread();
    void videoThread();
    void audioThread();
    void initSDLAudio();
    void closeSDLAudio();

    AVFormatContext* m_fmtCtx;
    AVCodecContext* m_videoCodecCtx;
    AVCodecContext* m_audioCodecCtx;
    SwsContext* m_swsCtx;
    SwrContext* m_swrCtx;

    int m_videoStreamIdx;
    int m_audioStreamIdx;

    PacketQueue* m_videoQueue;
    PacketQueue* m_audioQueue;

    QThread* m_demuxThreadHandle;
    QThread* m_videoThreadHandle;
    QThread* m_audioThreadHandle;

    std::atomic<bool> m_running;
    std::atomic<bool> m_paused;
    std::atomic<bool> m_seeking;

    double m_audioClock;
    QMutex m_clockMutex;

    int m_volume;
    int m_sampleRate;
    int m_channels;

    int m_videoWidth;
    int m_videoHeight;
};
