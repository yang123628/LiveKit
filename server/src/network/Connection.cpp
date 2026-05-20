#include "network/Connection.h"
#include "core/Logger.h"

Connection::Connection(int fd, struct sockaddr_in addr)
    : m_fd(fd),
      m_addr(addr),
      m_inputBuffer(4096),
      m_outputBuffer(4096) {
}

Connection::~Connection() {
    ::close(m_fd);
}

int Connection::fd() const {
    return m_fd;
}

std::string Connection::ip() const {
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &m_addr.sin_addr, buf, sizeof(buf));
    return std::string(buf);
}

int Connection::port() const {
    return ntohs(m_addr.sin_port);
}

Buffer& Connection::inputBuffer() {
    return m_inputBuffer;
}

Buffer& Connection::outputBuffer() {
    return m_outputBuffer;
}

void Connection::handleRead() {
    while (true) {
        char buf[65536];
        ssize_t n = ::read(m_fd, buf, sizeof(buf));
        if (n > 0) {
            m_inputBuffer.append(buf, n);
        } else if (n == 0) {
            handleClose();
            return;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            LOG_ERROR("read error on fd=" << m_fd << ": " << strerror(errno));
            handleClose();
            return;
        }
    }
    if (m_messageCallback) {
        m_messageCallback(shared_from_this());
    }
}

void Connection::handleWrite() {
    ssize_t n = ::write(m_fd, m_outputBuffer.peek(), m_outputBuffer.readableBytes());
    if (n > 0) {
        m_outputBuffer.retrieve(n);
    } else if (n < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            LOG_ERROR("write error on fd=" << m_fd << ": " << strerror(errno));
            handleClose();
        }
    }
}

void Connection::handleClose() {
    if (m_closeCallback) {
        m_closeCallback(shared_from_this());
    }
}

void Connection::send(const std::string& data) {
    send(data.c_str(), data.size());
}

void Connection::send(const char* data, size_t len) {
    ssize_t remaining = len;
    ssize_t written = 0;
    while (remaining > 0) {
        ssize_t n = ::write(m_fd, data + written, remaining);
        if (n > 0) {
            written += n;
            remaining -= n;
        } else if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                m_outputBuffer.append(data + written, remaining);
                break;
            } else {
                LOG_ERROR("send error on fd=" << m_fd << ": " << strerror(errno));
                handleClose();
                break;
            }
        }
    }
}

void Connection::setMessageCallback(const MessageCallback& cb) {
    m_messageCallback = cb;
}

void Connection::setCloseCallback(const CloseCallback& cb) {
    m_closeCallback = cb;
}

void Connection::setUserData(void* data) {
    m_userData = data;
}

void* Connection::userData() const {
    return m_userData;
}
