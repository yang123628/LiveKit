#include "core/EventLoop.h"
#include "network/Connection.h"
#include "core/Logger.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>

EventLoop::EventLoop(int port, int numThreads)
    : m_listenFd(-1),
      m_port(port),
      m_threadPool(numThreads),
      m_running(false) {
}

EventLoop::~EventLoop() {
    stop();
}

bool EventLoop::start() {
    m_listenFd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (m_listenFd < 0) {
        LOG_ERROR("create listen socket failed: " << strerror(errno));
        return false;
    }

    int opt = 1;
    ::setsockopt(m_listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(m_port);

    if (::bind(m_listenFd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        LOG_ERROR("bind failed on port " << m_port << ": " << strerror(errno));
        ::close(m_listenFd);
        return false;
    }

    if (::listen(m_listenFd, 128) < 0) {
        LOG_ERROR("listen failed: " << strerror(errno));
        ::close(m_listenFd);
        return false;
    }

    if (!m_epoll.addFd(m_listenFd, EPOLLIN | EPOLLET)) {
        LOG_ERROR("add listen fd to epoll failed");
        ::close(m_listenFd);
        return false;
    }

    m_threadPool.start();
    m_running = true;
    LOG_INFO("EventLoop started on port " << m_port);
    return true;
}

void EventLoop::stop() {
    m_running = false;
    m_threadPool.stop();
    if (m_listenFd >= 0) {
        ::close(m_listenFd);
        m_listenFd = -1;
    }
    {
        std::lock_guard<std::mutex> lock(m_connMutex);
        m_connections.clear();
    }
}

void EventLoop::loop() {
    while (m_running) {
        int n = m_epoll.wait(1000);
        if (n < 0) {
            if (errno == EINTR) continue;
            LOG_ERROR("epoll_wait error: " << strerror(errno));
            break;
        }
        for (int i = 0; i < n; ++i) {
            int eventFd = m_epoll.getEventFd(i);
            uint32_t events = m_epoll.getEventEvents(i);
            if (eventFd == m_listenFd) {
                handleAccept();
            } else {
                ConnectionPtr conn;
                {
                    std::lock_guard<std::mutex> lock(m_connMutex);
                    auto it = m_connections.find(eventFd);
                    if (it != m_connections.end()) {
                        conn = it->second;
                    }
                }
                if (!conn) continue;

                if (events & (EPOLLERR | EPOLLHUP)) {
                    closeConnection(eventFd);
                } else {
                    if (events & EPOLLIN) {
                        conn->handleRead();
                    }
                    if (events & EPOLLOUT) {
                        conn->handleWrite();
                    }
                }
            }
        }
    }
}

void EventLoop::handleAccept() {
    struct sockaddr_in clientAddr;
    socklen_t len = sizeof(clientAddr);
    while (true) {
        int connFd = ::accept4(m_listenFd, (struct sockaddr*)&clientAddr, &len, SOCK_NONBLOCK);
        if (connFd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            LOG_ERROR("accept failed: " << strerror(errno));
            break;
        }
        addConnection(connFd, clientAddr);
    }
}

void EventLoop::addConnection(int fd, struct sockaddr_in addr) {
    auto conn = std::make_shared<Connection>(fd, addr);
    conn->setMessageCallback(m_messageCallback);
    conn->setCloseCallback([this](ConnectionPtr c) { closeConnection(c->fd()); });

    {
        std::lock_guard<std::mutex> lock(m_connMutex);
        m_connections[fd] = conn;
    }

    if (!m_epoll.addFd(fd, EPOLLIN | EPOLLET)) {
        LOG_ERROR("add connection fd=" << fd << " to epoll failed");
        {
            std::lock_guard<std::mutex> lock(m_connMutex);
            m_connections.erase(fd);
        }
        return;
    }

    if (m_newConnectionCallback) {
        m_newConnectionCallback(conn);
    }

    LOG_INFO("new connection from " << conn->ip() << ":" << conn->port() << " fd=" << fd);
}

void EventLoop::closeConnection(int fd) {
    ConnectionPtr conn;
    {
        std::lock_guard<std::mutex> lock(m_connMutex);
        auto it = m_connections.find(fd);
        if (it != m_connections.end()) {
            conn = it->second;
            m_connections.erase(it);
        }
    }
    if (conn) {
        m_epoll.delFd(fd);
        if (m_closeCallback) {
            m_closeCallback(conn);
        }
        LOG_INFO("connection closed fd=" << fd);
    }
}

void EventLoop::setNewConnectionCallback(const NewConnectionCallback& cb) {
    m_newConnectionCallback = cb;
}

void EventLoop::setMessageCallback(const MessageCallback& cb) {
    m_messageCallback = cb;
}

void EventLoop::setCloseCallback(const CloseCallback& cb) {
    m_closeCallback = cb;
}

void EventLoop::removeConnection(int fd) {
    closeConnection(fd);
}

void EventLoop::updateConnection(int fd, uint32_t events) {
    m_epoll.modFd(fd, EPOLLIN | EPOLLET | events);
}

ThreadPool& EventLoop::threadPool() {
    return m_threadPool;
}
