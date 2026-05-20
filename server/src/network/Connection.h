#pragma once

#include <memory>
#include <functional>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <cstring>
#include "network/Buffer.h"

class Connection : public std::enable_shared_from_this<Connection> {
public:
    using Pointer = std::shared_ptr<Connection>;
    using MessageCallback = std::function<void(Pointer)>;
    using CloseCallback = std::function<void(Pointer)>;

    Connection(int fd, struct sockaddr_in addr);
    ~Connection();

    int fd() const;
    std::string ip() const;
    int port() const;

    Buffer& inputBuffer();
    Buffer& outputBuffer();

    void handleRead();
    void handleWrite();
    void handleClose();

    void send(const std::string& data);
    void send(const char* data, size_t len);

    void setMessageCallback(const MessageCallback& cb);
    void setCloseCallback(const CloseCallback& cb);

    void setUserData(void* data);
    void* userData() const;

private:
    int m_fd;
    struct sockaddr_in m_addr;
    Buffer m_inputBuffer;
    Buffer m_outputBuffer;
    MessageCallback m_messageCallback;
    CloseCallback m_closeCallback;
    void* m_userData = nullptr;
};
