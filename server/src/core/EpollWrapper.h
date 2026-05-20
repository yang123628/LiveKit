#pragma once

#include <sys/epoll.h>
#include <vector>
#include <unistd.h>

class EpollWrapper {
public:
    EpollWrapper();
    ~EpollWrapper();

    bool addFd(int fd, uint32_t events, void* ptr = nullptr);
    bool modFd(int fd, uint32_t events, void* ptr = nullptr);
    bool delFd(int fd);
    int wait(int timeout = -1);

    int getEventFd(int idx) const;
    void* getEventPtr(int idx) const;
    uint32_t getEventEvents(int idx) const;

private:
    int m_epollFd;
    std::vector<struct epoll_event> m_events;
    static const int MAX_EVENTS = 1024;
};
