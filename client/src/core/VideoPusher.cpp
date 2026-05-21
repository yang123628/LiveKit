#include "core/VideoPusher.h"
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <QDebug>
#include <QElapsedTimer>

VideoPusher::VideoPusher(QObject* parent)
    : QObject(parent)
    , m_fmtCtx(nullptr)
    , m_videoCodecCtx(nullptr)
    , m_audioCodecCtx(nullptr)
    , m_videoStream(nullptr)
    , m_audioStream(nullptr)
    , m_swsCtx(nullptr)
    , m_swrCtx(nullptr)
    , m_videoThread(nullptr)
    , m_audioThread(nullptr)
    , m_running(false)
    , m_videoPts(0)
    , m_audioPts(0)
    , m_width(1280)
    , m_height(720)
    , m_fps(25)
    , m_sampleRate(44100)
    , m_channels(2)
    , m_maxQueueSize(30)
{
}

VideoPusher::~VideoPusher() {
    stop();
}

bool VideoPusher::start(const QString& rtmpUrl, int width, int height, int fps, int bitRate) {
    m_width = width;
    m_height = height;
    m_fps = fps;

    if (!initVideoCodec(width, height, fps, bitRate)) return false;
    if (!initAudioCodec()) {
        cleanup();
        return false;
    }
    if (!initMuxer(rtmpUrl)) {
        cleanup();
        return false;
    }

    m_running = true;
    m_videoPts = 0;
    m_audioPts = 0;

    m_videoThread = QThread::create([this]() { encodeVideoLoop(); });
    m_audioThread = QThread::create([this]() { encodeAudioLoop(); });

    m_videoThread->start();
    m_audioThread->start();

    emit SIG_stateChanged(true);
    return true;
}

void VideoPusher::stop() {
    if (!m_running) return;
    m_running = false;

    m_videoCond.wakeAll();
    m_audioCond.wakeAll();

    if (m_videoThread) {
        m_videoThread->quit();
        m_videoThread->wait();
        delete m_videoThread;
        m_videoThread = nullptr;
    }
    if (m_audioThread) {
        m_audioThread->quit();
        m_audioThread->wait();
        delete m_audioThread;
        m_audioThread = nullptr;
    }

    if (m_fmtCtx) {
        if (m_fmtCtx->pb) {
            av_write_trailer(m_fmtCtx);
        }
    }

    cleanup();
    emit SIG_stateChanged(false);
}

bool VideoPusher::isRunning() const {
    return m_running;
}

void VideoPusher::pushVideoFrame(const QImage& frame) {
    QMutexLocker locker(&m_videoMutex);
    if (m_videoQueue.size() >= m_maxQueueSize) {
        m_videoQueue.dequeue();
    }
    m_videoQueue.enqueue(frame.copy());
    m_videoCond.wakeOne();
}

void VideoPusher::pushAudioFrame(const uint8_t* data, int size) {
    QMutexLocker locker(&m_audioMutex);
    if (m_audioQueue.size() >= m_maxQueueSize) {
        m_audioQueue.dequeue();
    }
    m_audioQueue.enqueue(QByteArray(reinterpret_cast<const char*>(data), size));
    m_audioCond.wakeOne();
}

bool VideoPusher::initVideoCodec(int width, int height, int fps, int bitRate) {
    const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        emit SIG_error("H.264 encoder not found");
        return false;
    }

    m_videoCodecCtx = avcodec_alloc_context3(codec);
    m_videoCodecCtx->codec_id = AV_CODEC_ID_H264;
    m_videoCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    m_videoCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    m_videoCodecCtx->width = width;
    m_videoCodecCtx->height = height;
    m_videoCodecCtx->time_base = {1, fps};
    m_videoCodecCtx->framerate = {fps, 1};
    m_videoCodecCtx->bit_rate = bitRate;
    m_videoCodecCtx->gop_size = fps * 2;
    m_videoCodecCtx->max_b_frames = 0;

    av_opt_set(m_videoCodecCtx->priv_data, "preset", "superfast", 0);
    av_opt_set(m_videoCodecCtx->priv_data, "tune", "zerolatency", 0);

    if (avcodec_open2(m_videoCodecCtx, codec, nullptr) < 0) {
        emit SIG_error("Failed to open H.264 encoder");
        return false;
    }

    m_swsCtx = sws_getContext(width, height, AV_PIX_FMT_RGB24,
        width, height, AV_PIX_FMT_YUV420P,
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    return true;
}

bool VideoPusher::initAudioCodec() {
    const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!codec) {
        emit SIG_error("AAC encoder not found");
        return false;
    }

    m_audioCodecCtx = avcodec_alloc_context3(codec);
    m_audioCodecCtx->codec_id = AV_CODEC_ID_AAC;
    m_audioCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;
    m_audioCodecCtx->sample_fmt = AV_SAMPLE_FMT_FLTP;
    m_audioCodecCtx->sample_rate = m_sampleRate;
    m_audioCodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
    m_audioCodecCtx->channels = m_channels;
    m_audioCodecCtx->bit_rate = 128000;
    m_audioCodecCtx->time_base = {1, m_sampleRate};

    if (avcodec_open2(m_audioCodecCtx, codec, nullptr) < 0) {
        emit SIG_error("Failed to open AAC encoder");
        return false;
    }

    m_swrCtx = swr_alloc_set_opts(nullptr,
        AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_FLTP, m_sampleRate,
        AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_FLTP, m_sampleRate,
        0, nullptr);
    if (!m_swrCtx || swr_init(static_cast<SwrContext*>(m_swrCtx)) < 0) {
        emit SIG_error("Failed to init audio resampler");
        return false;
    }

    return true;
}

bool VideoPusher::initMuxer(const QString& rtmpUrl) {
    int ret = avformat_alloc_output_context2(&m_fmtCtx, nullptr, "flv", rtmpUrl.toUtf8().constData());
    if (ret < 0 || !m_fmtCtx) {
        emit SIG_error("Failed to allocate output context");
        return false;
    }

    m_videoStream = avformat_new_stream(m_fmtCtx, nullptr);
    m_videoStream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    m_videoStream->codecpar->codec_id = AV_CODEC_ID_H264;
    m_videoStream->codecpar->width = m_width;
    m_videoStream->codecpar->height = m_height;
    m_videoStream->codecpar->format = AV_PIX_FMT_YUV420P;
    m_videoStream->time_base = {1, m_fps};
    avcodec_parameters_from_context(m_videoStream->codecpar, m_videoCodecCtx);

    m_audioStream = avformat_new_stream(m_fmtCtx, nullptr);
    m_audioStream->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
    m_audioStream->codecpar->codec_id = AV_CODEC_ID_AAC;
    m_audioStream->codecpar->sample_rate = m_sampleRate;
    m_audioStream->codecpar->channel_layout = AV_CH_LAYOUT_STEREO;
    m_audioStream->codecpar->format = AV_SAMPLE_FMT_FLTP;
    m_audioStream->time_base = {1, m_sampleRate};
    avcodec_parameters_from_context(m_audioStream->codecpar, m_audioCodecCtx);

    if (!(m_fmtCtx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open2(&m_fmtCtx->pb, rtmpUrl.toUtf8().constData(), AVIO_FLAG_WRITE, nullptr, nullptr);
        if (ret < 0) {
            emit SIG_error("Failed to open RTMP URL");
            return false;
        }
    }

    ret = avformat_write_header(m_fmtCtx, nullptr);
    if (ret < 0) {
        emit SIG_error("Failed to write FLV header");
        return false;
    }

    return true;
}

void VideoPusher::encodeVideoLoop() {
    AVFrame* frame = av_frame_alloc();
    frame->format = AV_PIX_FMT_YUV420P;
    frame->width = m_width;
    frame->height = m_height;
    av_frame_get_buffer(frame, 32);

    AVPacket* pkt = av_packet_alloc();

    QElapsedTimer timer;
    timer.start();
    qint64 frameInterval = 1000 / m_fps;
    qint64 nextFrameTime = 0;

    while (m_running) {
        QImage img;
        {
            QMutexLocker locker(&m_videoMutex);
            if (m_videoQueue.isEmpty()) {
                m_videoCond.wait(&m_videoMutex, 100);
                continue;
            }
            img = m_videoQueue.dequeue();
        }

        qint64 now = timer.elapsed();
        if (now < nextFrameTime) {
            QThread::msleep(static_cast<unsigned long>(nextFrameTime - now));
        }
        nextFrameTime += frameInterval;

        QImage rgb = img.convertToFormat(QImage::Format_RGB888).scaled(m_width, m_height,
            Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        const uint8_t* srcData[1] = {rgb.constBits()};
        int srcLineSize[1] = {rgb.bytesPerLine()};

        sws_scale(static_cast<SwsContext*>(m_swsCtx),
            srcData, srcLineSize, 0, m_height,
            frame->data, frame->linesize);

        frame->pts = m_videoPts.fetch_add(1);

        int ret = avcodec_send_frame(m_videoCodecCtx, frame);
        if (ret < 0) continue;

        while (ret >= 0) {
            ret = avcodec_receive_packet(m_videoCodecCtx, pkt);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
            if (ret < 0) break;

            pkt->stream_index = m_videoStream->index;
            av_packet_rescale_ts(pkt, m_videoCodecCtx->time_base, m_videoStream->time_base);

            QMutexLocker fmtLocker(&m_videoMutex);
            av_interleaved_write_frame(m_fmtCtx, pkt);
            av_packet_unref(pkt);
        }
    }

    av_frame_free(&frame);
    av_packet_free(&pkt);
}

void VideoPusher::encodeAudioLoop() {
    AVFrame* frame = av_frame_alloc();
    frame->format = AV_SAMPLE_FMT_FLTP;
    frame->channel_layout = AV_CH_LAYOUT_STEREO;
    frame->sample_rate = m_sampleRate;
    frame->nb_samples = m_audioCodecCtx->frame_size;
    av_frame_get_buffer(frame, 0);

    AVPacket* pkt = av_packet_alloc();
    auto* swr = static_cast<SwrContext*>(m_swrCtx);

    while (m_running) {
        QByteArray audioData;
        {
            QMutexLocker locker(&m_audioMutex);
            if (m_audioQueue.isEmpty()) {
                m_audioCond.wait(&m_audioMutex, 100);
                continue;
            }
            audioData = m_audioQueue.dequeue();
        }

        int nbSamples = audioData.size() / (m_channels * sizeof(float));
        if (nbSamples <= 0) continue;

        const uint8_t* srcData[2] = {nullptr, nullptr};
        std::vector<uint8_t> ch0(nbSamples * sizeof(float));
        std::vector<uint8_t> ch1(nbSamples * sizeof(float));

        const float* interleaved = reinterpret_cast<const float*>(audioData.constData());
        for (int i = 0; i < nbSamples; ++i) {
            reinterpret_cast<float*>(ch0.data())[i] = interleaved[i * 2];
            reinterpret_cast<float*>(ch1.data())[i] = interleaved[i * 2 + 1];
        }
        srcData[0] = ch0.data();
        srcData[1] = ch1.data();

        int offset = 0;
        while (offset < nbSamples && m_running) {
            int remaining = nbSamples - offset;
            int toEncode = qMin(remaining, m_audioCodecCtx->frame_size);

            const uint8_t* inData[2] = {
                srcData[0] + offset * sizeof(float),
                srcData[1] + offset * sizeof(float)
            };

            int ret = swr_convert(swr, frame->data, frame->nb_samples,
                inData, toEncode);
            if (ret < 0) break;

            frame->pts = m_audioPts.fetch_add(ret);
            frame->nb_samples = ret;

            ret = avcodec_send_frame(m_audioCodecCtx, frame);
            if (ret < 0) break;

            while (ret >= 0) {
                ret = avcodec_receive_packet(m_audioCodecCtx, pkt);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
                if (ret < 0) break;

                pkt->stream_index = m_audioStream->index;
                av_packet_rescale_ts(pkt, m_audioCodecCtx->time_base, m_audioStream->time_base);

                QMutexLocker fmtLocker(&m_audioMutex);
                av_interleaved_write_frame(m_fmtCtx, pkt);
                av_packet_unref(pkt);
            }

            offset += toEncode;
        }
    }

    av_frame_free(&frame);
    av_packet_free(&pkt);
}

void VideoPusher::cleanup() {
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
        if (m_fmtCtx->pb && !(m_fmtCtx->oformat->flags & AVFMT_NOFILE)) {
            avio_closep(&m_fmtCtx->pb);
        }
        avformat_free_context(m_fmtCtx);
        m_fmtCtx = nullptr;
    }
    m_videoStream = nullptr;
    m_audioStream = nullptr;

    {
        QMutexLocker locker(&m_videoMutex);
        m_videoQueue.clear();
    }
    {
        QMutexLocker locker(&m_audioMutex);
        m_audioQueue.clear();
    }
}
