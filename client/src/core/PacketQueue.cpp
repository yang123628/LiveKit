#include "core/PacketQueue.h"
#include <libavcodec/avcodec.h>
#include <libavutil/mem.h>

PacketQueue::PacketQueue()
    : m_maxSize(500)
{
}

PacketQueue::~PacketQueue() {
    clear();
}

void PacketQueue::push(AVPacket* packet) {
    QMutexLocker locker(&m_mutex);
    if (m_queue.size() >= m_maxSize) {
        AVPacket* old = m_queue.dequeue();
        av_packet_unref(old);
        av_packet_free(&old);
    }
    m_queue.enqueue(packet);
    m_notEmpty.wakeOne();
}

bool PacketQueue::pop(AVPacket* packet, int timeoutMs) {
    QMutexLocker locker(&m_mutex);
    if (m_queue.isEmpty()) {
        m_notEmpty.wait(&m_mutex, timeoutMs);
    }
    if (m_queue.isEmpty()) {
        return false;
    }
    AVPacket* front = m_queue.dequeue();
    av_packet_ref(packet, front);
    av_packet_unref(front);
    av_packet_free(&front);
    return true;
}

void PacketQueue::clear() {
    QMutexLocker locker(&m_mutex);
    while (!m_queue.isEmpty()) {
        AVPacket* pkt = m_queue.dequeue();
        av_packet_unref(pkt);
        av_packet_free(&pkt);
    }
}

int PacketQueue::size() const {
    QMutexLocker locker(&m_mutex);
    return m_queue.size();
}

bool PacketQueue::isEmpty() const {
    QMutexLocker locker(&m_mutex);
    return m_queue.isEmpty();
}

void PacketQueue::setMaxSize(int size) {
    m_maxSize = size;
}
