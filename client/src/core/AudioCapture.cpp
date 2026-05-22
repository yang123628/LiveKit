#include "core/AudioCapture.h"
#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>

AudioCapture::AudioCapture(QObject* parent)
    : QThread(parent)
    , m_audioInput(nullptr)
    , m_audioDevice(nullptr)
    , m_running(false)
    , m_swrCtx(nullptr)
    , m_sampleRate(44100)
    , m_channels(2)
    , m_frameSize(1024)
{
}

AudioCapture::~AudioCapture() {
    close();
    wait();
}

bool AudioCapture::open() {
    if (m_running) return false;

    QAudioFormat format;
    format.setSampleRate(m_sampleRate);
    format.setChannelCount(m_channels);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
    if (!info.isFormatSupported(format)) {
        format = info.nearestFormat(format);
    }

    m_audioInput = new QAudioInput(info, format, this);
    m_audioDevice = m_audioInput->start();

    if (!m_audioDevice) {
        delete m_audioInput;
        m_audioInput = nullptr;
        return false;
    }

    SwrContext* swr = swr_alloc_set_opts(nullptr,
        AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_FLTP, m_sampleRate,
        AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, m_sampleRate,
        0, nullptr);
    if (!swr || swr_init(swr) < 0) {
        m_audioInput->stop();
        delete m_audioInput;
        m_audioInput = nullptr;
        m_audioDevice = nullptr;
        return false;
    }
    m_swrCtx = swr;

    m_running = true;
    start();
    return true;
}

void AudioCapture::close() {
    m_running = false;
    wait();

    if (m_swrCtx) {
        swr_free((SwrContext**)&m_swrCtx);
        m_swrCtx = nullptr;
    }
    if (m_audioInput) {
        m_audioInput->stop();
        delete m_audioInput;
        m_audioInput = nullptr;
        m_audioDevice = nullptr;
    }
}

bool AudioCapture::isOpened() const {
    return m_running;
}

void AudioCapture::run() {
    auto* swr = static_cast<SwrContext*>(m_swrCtx);
    int srcNbSamples = m_frameSize;
    int srcLineSize = srcNbSamples * m_channels * 2;
    QByteArray srcBuf(srcLineSize, 0);

    int maxDstNbSamples = swr_get_out_samples(swr, srcNbSamples);
    uint8_t** dstData = nullptr;
    int dstLineSize = 0;
    av_samples_alloc_array_and_samples(&dstData, &dstLineSize, m_channels,
    maxDstNbSamples, AV_SAMPLE_FMT_FLTP, 0);

    while (m_running) {
        qint64 bytesReady = m_audioInput->bytesReady();
        if (bytesReady < srcLineSize) {
            msleep(5);
            continue;
        }

        qint64 bytesRead = m_audioDevice->read(srcBuf.data(), srcLineSize);
        if (bytesRead < srcLineSize) {
            msleep(5);
            continue;
        }

        const uint8_t* srcData[1] = {reinterpret_cast<const uint8_t*>(srcBuf.constData())};
        int dstNbSamples = swr_convert(swr, dstData, maxDstNbSamples,
            srcData, srcNbSamples);

        if (dstNbSamples > 0) {
            int fltpSize = dstNbSamples * m_channels * sizeof(float);
            uint8_t* outBuf = static_cast<uint8_t*>(av_malloc(fltpSize));
            if (outBuf) {
                int offset = 0;
                for (int c = 0; c < m_channels; ++c) {
                    int chSize = dstNbSamples * sizeof(float);
                    memcpy(outBuf + offset, dstData[c], chSize);
                    offset += chSize;
                }
                emit SIG_sendAudioFrameData(outBuf, fltpSize);
            }
        }
    }

    if (dstData[0]) {
        av_freep(&dstData[0]);
    }
}
