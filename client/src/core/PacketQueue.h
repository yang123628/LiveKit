#pragma once

#include <QQueue>
#include <QMutex>
#include <QWaitCondition>
#include <QMutexLocker>

struct AVPacket;

class PacketQueue {
public:
    PacketQueue();
    ~PacketQueue();

    void push(AVPacket* packet);
    bool pop(AVPacket* packet, int timeoutMs = 100);
    void clear();
    int size() const;
    bool isEmpty() const;
    void setMaxSize(int size);

private:
    QQueue<AVPacket*> m_queue;
    mutable QMutex m_mutex;
    QWaitCondition m_notEmpty;
    int m_maxSize;
};
