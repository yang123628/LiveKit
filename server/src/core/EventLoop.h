#pragma once

#include <memory>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <atomic>
#include "core/EpollWrapper.h"
#include "core/ThreadPool.h"

class Connection;

class EventLoop {
public:
    using ConnectionPtr = std::shared_ptr<Connection>;
    using NewConnectionCallback = std::function<void(ConnectionPtr)>;
    using MessageCallback = std::function<void(ConnectionPtr)>;
    using CloseCallback = std::function<void(ConnectionPtr)>;

    EventLoop(int port, int numThreads);
    ~EventLoop();

    bool start();
    void stop();
    void loop();

    void setNewConnectionCallback(const NewConnectionCallback& cb);
    void setMessageCallback(const MessageCallback& cb);
    void setCloseCallback(const CloseCallback& cb);

    void removeConnection(int fd);
    void updateConnection(int fd, uint32_t events);

    ThreadPool& threadPool();

private:
    void handleAccept();
    void addConnection(int fd, struct sockaddr_in addr);
    void closeConnection(int fd);

    int m_listenFd;
    int m_port;
    EpollWrapper m_epoll;
    ThreadPool m_threadPool;
    std::unordered_map<int, ConnectionPtr> m_connections;
    std::mutex m_connMutex;

    NewConnectionCallback m_newConnectionCallback;
    MessageCallback m_messageCallback;
    CloseCallback m_closeCallback;

    std::atomic<bool> m_running;
};
