#include "network/Buffer.h"

const char Buffer::kCRLF[] = "\r\n";

Buffer::Buffer(size_t initialSize)
    : m_buffer(initialSize),
      m_readIndex(0),
      m_writeIndex(0) {
}

void Buffer::append(const char* data, size_t len) {
    ensureWritableBytes(len);
    std::copy(data, data + len, beginWrite());
    hasWritten(len);
}

void Buffer::append(const std::string& data) {
    append(data.c_str(), data.size());
}

size_t Buffer::readableBytes() const {
    return m_writeIndex - m_readIndex;
}

size_t Buffer::writableBytes() const {
    return m_buffer.size() - m_writeIndex;
}

std::string Buffer::retrieveAsString(size_t len) {
    len = std::min(len, readableBytes());
    std::string result(peek(), len);
    retrieve(len);
    return result;
}

std::string Buffer::retrieveAllAsString() {
    return retrieveAsString(readableBytes());
}

void Buffer::retrieve(size_t len) {
    len = std::min(len, readableBytes());
    m_readIndex += len;
    if (m_readIndex == m_writeIndex) {
        m_readIndex = 0;
        m_writeIndex = 0;
    }
}

std::string Buffer::retrieveLine() {
    const char* crlf = std::search(peek(), peek() + readableBytes(), kCRLF, kCRLF + kCRLFLength);
    if (crlf == peek() + readableBytes()) {
        return "";
    }
    size_t len = crlf - peek();
    std::string line(peek(), len);
    retrieve(len + kCRLFLength);
    return line;
}

bool Buffer::hasLine() const {
    const char* crlf = std::search(peek(), peek() + readableBytes(), kCRLF, kCRLF + kCRLFLength);
    return crlf != peek() + readableBytes();
}

const char* Buffer::peek() const {
    return beginPtr() + m_readIndex;
}

char* Buffer::beginWrite() {
    return beginPtr() + m_writeIndex;
}

void Buffer::ensureWritableBytes(size_t len) {
    if (writableBytes() < len) {
        makeSpace(len);
    }
}

void Buffer::hasWritten(size_t len) {
    m_writeIndex += len;
}

void Buffer::shrink() {
    m_buffer.resize(m_writeIndex);
    m_buffer.shrink_to_fit();
}

char* Buffer::beginPtr() {
    return &*m_buffer.begin();
}

const char* Buffer::beginPtr() const {
    return &*m_buffer.begin();
}

void Buffer::makeSpace(size_t len) {
    if (writableBytes() + m_readIndex < len) {
        m_buffer.resize(m_writeIndex + len);
    } else {
        size_t readable = readableBytes();
        std::copy(beginPtr() + m_readIndex, beginPtr() + m_writeIndex, beginPtr());
        m_readIndex = 0;
        m_writeIndex = readable;
    }
}
