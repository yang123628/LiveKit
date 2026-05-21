#include "core/VideoPlayer.h"
#include "core/PacketQueue.h"
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <SDL2/SDL.h>
#include <QDebug>
#include <QElapsedTimer>

static const int MAX_QUEUE_SIZE = 500;

VideoPlayer::VideoPlayer(QObject* parent)
    : QObject(parent)
    , m_fmtCtx(nullptr)
    , m_videoCodecCtx(nullptr)
    , m_audioCodecCtx(nullptr)
    , m_swsCtx(nullptr)
    , m_swrCtx(nullptr)
    , m_videoStreamIdx(-1)
    , m_audioStreamIdx(-1)
    , m_videoQueue(nullptr)
    , m_audioQueue(nullptr)
    , m_demuxThreadHandle(nullptr)
    , m_videoThreadHandle(nullptr)
    , m_audioThreadHandle(nullptr)
    , m_running(false)
    , m_paused(false)
    , m_seeking(false)
    , m_audioClock(0.0)
    , m_volume(80)
    , m_sampleRate(44100)
    , m_channels(2)
    , m_videoWidth(0)
    , m_videoHeight(0)
{
    m_videoQueue = new PacketQueue();
    m_videoQueue->setMaxSize(MAX_QUEUE_SIZE);
    m_audioQueue = new PacketQueue();
    m_audioQueue->setMaxSize(MAX_QUEUE_SIZE);
}

VideoPlayer::~VideoPlayer() {
    stop();
    delete m_videoQueue;
    delete m_audioQueue;
}

bool VideoPlayer::play(const QString& url) {
    if (m_running) stop();

    m_fmtCtx = nullptr;
    if (avformat_open_input(&m_fmtCtx, url.toUtf8().constData(), nullptr, nullptr) < 0) {
        emit SIG_error("Failed to open stream");
        return false;
    }

    if (avformat_find_stream_info(m_fmtCtx, nullptr) < 0) {
        emit SIG_error("Failed to find stream info");
        avformat_close_input(&m_fmtCtx);
        return false;
    }

    m_videoStreamIdx = -1;
    m_audioStreamIdx = -1;
    for (unsigned int i = 0; i < m_fmtCtx->nb_streams; ++i) {
        if (m_fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && m_videoStreamIdx < 0) {
            m_videoStreamIdx = i;
        } else if (m_fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && m_audioStreamIdx < 0) {
            m_audioStreamIdx = i;
        }
    }

    if (m_videoStreamIdx < 0) {
        emit SIG_error("No video stream found");
        avformat_close_input(&m_fmtCtx);
        return false;
    }

    const AVCodec* videoCodec = avcodec_find_decoder(m_fmtCtx->streams[m_videoStreamIdx]->codecpar->codec_id);
    if (!videoCodec) {
        emit SIG_error("Video decoder not found");
        avformat_close_input(&m_fmtCtx);
        return false;
    }

    m_videoCodecCtx = avcodec_alloc_context3(videoCodec);
    avcodec_parameters_to_context(m_videoCodecCtx, m_fmtCtx->streams[m_videoStreamIdx]->codecpar);
    if (avcodec_open2(m_videoCodecCtx, videoCodec, nullptr) < 0) {
        emit SIG_error("Failed to open video decoder");
        avformat_close_input(&m_fmtCtx);
        return false;
    }

    m_videoWidth = m_videoCodecCtx->width;
    m_videoHeight = m_videoCodecCtx->height;

    m_swsCtx = sws_getContext(m_videoWidth, m_videoHeight, m_videoCodecCtx->pix_fmt,
        m_videoWidth, m_videoHeight, AV_PIX_FMT_RGB24,
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    if (m_audioStreamIdx >= 0) {
        const AVCodec* audioCodec = avcodec_find_decoder(m_fmtCtx->streams[m_audioStreamIdx]->codecpar->codec_id);
        if (audioCodec) {
            m_audioCodecCtx = avcodec_alloc_context3(audioCodec);
            avcodec_parameters_to_context(m_audioCodecCtx, m_fmtCtx->streams[m_audioStreamIdx]->codecpar);
            if (avcodec_open2(m_audioCodecCtx, audioCodec, nullptr) >= 0) {
                m_sampleRate = m_audioCodecCtx->sample_rate;
                m_channels = m_audioCodecCtx->channels;

                m_swrCtx = swr_alloc_set_opts(nullptr,
                    AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, m_sampleRate,
                    m_audioCodecCtx->channel_layout, m_audioCodecCtx->sample_fmt, m_sampleRate,
                    0, nullptr);
                swr_init(static_cast<SwrContext*>(m_swrCtx));

                initSDLAudio();
            }
        }
    }

    m_videoQueue->clear();
    m_audioQueue->clear();
    m_running = true;
    m_paused = false;
    m_audioClock = 0.0;

    m_demuxThreadHandle = QThread::create([this]() { demuxThread(); });
    m_videoThreadHandle = QThread::create([this]() { videoThread(); });
    m_audioThreadHandle = QThread::create([this]() { audioThread(); });

    m_demuxThreadHandle->start();
    m_videoThreadHandle->start();
    m_audioThreadHandle->start();

    emit SIG_stateChanged(true);
    return true;
}

void VideoPlayer::pause() {
    m_paused = true;
}

void VideoPlayer::resume() {
    m_paused = false;
}

void VideoPlayer::stop() {
    if (!m_running) return;
    m_running = false;

    if (m_demuxThreadHandle) {
        m_demuxThreadHandle->quit();
        m_demuxThreadHandle->wait(3000);
        delete m_demuxThreadHandle;
        m_demuxThreadHandle = nullptr;
    }
    if (m_videoThreadHandle) {
        m_videoThreadHandle->quit();
        m_videoThreadHandle->wait(3000);
        delete m_videoThreadHandle;
        m_videoThreadHandle = nullptr;
    }
    if (m_audioThreadHandle) {
        m_audioThreadHandle->quit();
        m_audioThreadHandle->wait(3000);
        delete m_audioThreadHandle;
        m_audioThreadHandle = nullptr;
    }

    closeSDLAudio();

    if (m_swsCtx) {
        sws_freeContext(static_cast<SwsContext*>(m_swsCtx));
        m_swsCtx = nullptr;
    }
    if (m_swrCtx) {
        swr_free(static_cast<SwrContext**>(&m_swrCtx));
        m_swrCtx = nullptr;
    }
    if (m_videoCodecCtx) {
        avcodec_free_context(&m_videoCodecCtx);
    }
    if (m_audioCodecCtx) {
        avcodec_free_context(&m_audioCodecCtx);
    }
    if (m_fmtCtx) {
        avformat_close_input(&m_fmtCtx);
    }

    m_videoQueue->clear();
    m_audioQueue->clear();

    emit SIG_stateChanged(false);
}

void VideoPlayer::seek(int64_t ms) {
    Q_UNUSED(ms)
}

bool VideoPlayer::isPlaying() const {
    return m_running && !m_paused;
}

void VideoPlayer::setVolume(int volume) {
    m_volume = qBound(0, volume, 100);
}

int VideoPlayer::volume() const {
    return m_volume;
}

void VideoPlayer::demuxThread() {
    AVPacket* pkt = av_packet_alloc();

    while (m_running) {
        if (m_paused) {
            QThread::msleep(10);
            continue;
        }

        if (m_videoQueue->size() > MAX_QUEUE_SIZE || m_audioQueue->size() > MAX_QUEUE_SIZE) {
            QThread::msleep(10);
            continue;
        }

        int ret = av_read_frame(m_fmtCtx, pkt);
        if (ret < 0) {
            if (ret == AVERROR_EOF) {
                QThread::msleep(100);
                continue;
            }
            QThread::msleep(10);
            continue;
        }

        if (pkt->stream_index == m_videoStreamIdx) {
            AVPacket* copy = av_packet_alloc();
            av_packet_ref(copy, pkt);
            m_videoQueue->push(copy);
        } else if (pkt->stream_index == m_audioStreamIdx) {
            AVPacket* copy = av_packet_alloc();
            av_packet_ref(copy, pkt);
            m_audioQueue->push(copy);
        }

        av_packet_unref(pkt);
    }

    av_packet_free(&pkt);
}

void VideoPlayer::videoThread() {
    AVFrame* frame = av_frame_alloc();
    AVPacket* pkt = av_packet_alloc();

    while (m_running) {
        if (m_paused) {
            QThread::msleep(10);
            continue;
        }

        if (!m_videoQueue->pop(pkt, 100)) {
            continue;
        }

        int ret = avcodec_send_packet(m_videoCodecCtx, pkt);
        av_packet_unref(pkt);

        if (ret < 0) continue;

        while (ret >= 0) {
            ret = avcodec_receive_frame(m_videoCodecCtx, frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
            if (ret < 0) break;

            double pts = frame->pts * av_q2d(m_fmtCtx->streams[m_videoStreamIdx]->time_base);
            double audioClk;
            {
                QMutexLocker locker(&m_clockMutex);
                audioClk = m_audioClock;
            }

            double diff = pts - audioClk;
            if (diff > 0.1 && diff < 5.0) {
                QThread::usleep(static_cast<unsigned long>(diff * 1000000));
            } else if (diff > 5.0) {
                // skip
            }

            QImage img(m_videoWidth, m_videoHeight, QImage::Format_RGB888);
            uint8_t* dstData[1] = {img.bits()};
            int dstLineSize[1] = {img.bytesPerLine()};

            sws_scale(static_cast<SwsContext*>(m_swsCtx),
                frame->data, frame->linesize, 0, m_videoHeight,
                dstData, dstLineSize);

            emit SIG_frameReady(img);
            av_frame_unref(frame);
        }
    }

    av_frame_free(&frame);
    av_packet_free(&pkt);
}

void VideoPlayer::audioThread() {
    AVFrame* frame = av_frame_alloc();
    AVPacket* pkt = av_packet_alloc();
    auto* swr = static_cast<SwrContext*>(m_swrCtx);

    while (m_running) {
        if (m_paused) {
            QThread::msleep(10);
            continue;
        }

        if (!m_audioQueue->pop(pkt, 100)) {
            continue;
        }

        int ret = avcodec_send_packet(m_audioCodecCtx, pkt);
        av_packet_unref(pkt);

        if (ret < 0) continue;

        while (ret >= 0) {
            ret = avcodec_receive_frame(m_audioCodecCtx, frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
            if (ret < 0) break;

            double pts = frame->pts * av_q2d(m_fmtCtx->streams[m_audioStreamIdx]->time_base);
            {
                QMutexLocker locker(&m_clockMutex);
                m_audioClock = pts;
            }

            if (swr) {
                int maxDstNbSamples = swr_get_out_samples(swr, frame->nb_samples);
                uint8_t* dstData[1] = {nullptr};
                int dstLineSize[1] = {0};
                av_samples_alloc_array_and_samples(&dstData, dstLineSize, m_channels,
                    maxDstNbSamples, AV_SAMPLE_FMT_S16, 0);

                int nbSamples = swr_convert(swr, dstData, maxDstNbSamples,
                    const_cast<const uint8_t**>(frame->data), frame->nb_samples);

                if (nbSamples > 0) {
                    int bufSize = nbSamples * m_channels * 2;
                    float vol = m_volume / 100.0f;

                    if (vol < 1.0f) {
                        int16_t* samples = reinterpret_cast<int16_t*>(dstData[0]);
                        int totalSamples = nbSamples * m_channels;
                        for (int i = 0; i < totalSamples; ++i) {
                            samples[i] = static_cast<int16_t>(samples[i] * vol);
                        }
                    }

                    SDL_QueueAudio(1, dstData[0], bufSize);
                }

                av_freep(&dstData[0]);
            }

            av_frame_unref(frame);
        }
    }

    av_frame_free(&frame);
    av_packet_free(&pkt);
}

void VideoPlayer::initSDLAudio() {
    if (SDL_WasInit(SDL_INIT_AUDIO)) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }

    SDL_Init(SDL_INIT_AUDIO);

    SDL_AudioSpec wanted;
    SDL_zero(wanted);
    wanted.freq = m_sampleRate;
    wanted.format = AUDIO_S16SYS;
    wanted.channels = m_channels;
    wanted.silence = 0;
    wanted.samples = 1024;
    wanted.callback = nullptr;

    SDL_AudioSpec obtained;
    if (SDL_OpenAudio(&wanted, &obtained) < 0) {
        qDebug() << "SDL_OpenAudio failed:" << SDL_GetError();
        return;
    }

    SDL_PauseAudio(0);
}

void VideoPlayer::closeSDLAudio() {
    SDL_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}
