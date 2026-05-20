#pragma once

#include <vector>
#include <string>
#include <cstring>
#include <algorithm>

class Buffer {
public:
    static const size_t kInitialSize = 1024;
    static const char kCRLF[];
    static const size_t kCRLFLength = 2;

    Buffer(size_t initialSize = kInitialSize);

    void append(const char* data, size_t len);
    void append(const std::string& data);

    size_t readableBytes() const;
    size_t writableBytes() const;

    std::string retrieveAsString(size_t len);
    std::string retrieveAllAsString();
    void retrieve(size_t len);

    std::string retrieveLine();
    bool hasLine() const;

    const char* peek() const;
    char* beginWrite();

    void ensureWritableBytes(size_t len);
    void hasWritten(size_t len);

    void shrink();

private:
    std::vector<char> m_buffer;
    size_t m_readIndex;
    size_t m_writeIndex;

    char* beginPtr();
    const char* beginPtr() const;
    void makeSpace(size_t len);
};
