#pragma once

#include <QThread>
#include <QAudioInput>
#include <QIODevice>
#include <QByteArray>
#include <atomic>

class AudioCapture : public QThread {
    Q_OBJECT

public:
    explicit AudioCapture(QObject* parent = nullptr);
    ~AudioCapture() override;

    bool open();
    void close();
    bool isOpened() const;

signals:
    void SIG_sendAudioFrameData(uint8_t* data, int size);

protected:
    void run() override;

private:
    QAudioInput* m_audioInput;
    QIODevice* m_audioDevice;
    std::atomic<bool> m_running;
    void* m_swrCtx;
    int m_sampleRate;
    int m_channels;
    int m_frameSize;
};
