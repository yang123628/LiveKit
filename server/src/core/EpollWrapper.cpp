#include "core/EpollWrapper.h"
#include "core/Logger.h"
#include <cstring>
#include <errno.h>

EpollWrapper::EpollWrapper() : m_events(MAX_EVENTS) {
    m_epollFd = epoll_create1(0);
    if (m_epollFd < 0) {
        LOG_ERROR("epoll_create1 failed: " << strerror(errno));
    }
}

EpollWrapper::~EpollWrapper() {
    if (m_epollFd >= 0) {
        close(m_epollFd);
    }
}

bool EpollWrapper::addFd(int fd, uint32_t events, void* ptr) {
    struct epoll_event ev;
    ev.events = events;
    if (ptr) {
        ev.data.ptr = ptr;
    } else {
        ev.data.fd = fd;
    }
    if (epoll_ctl(m_epollFd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        LOG_ERROR("epoll_ctl ADD failed for fd=" << fd << ": " << strerror(errno));
        return false;
    }
    return true;
}

bool EpollWrapper::modFd(int fd, uint32_t events, void* ptr) {
    struct epoll_event ev;
    ev.events = events;
    if (ptr) {
        ev.data.ptr = ptr;
    } else {
        ev.data.fd = fd;
    }
    if (epoll_ctl(m_epollFd, EPOLL_CTL_MOD, fd, &ev) < 0) {
        LOG_ERROR("epoll_ctl MOD failed for fd=" << fd << ": " << strerror(errno));
        return false;
    }
    return true;
}

bool EpollWrapper::delFd(int fd) {
    if (epoll_ctl(m_epollFd, EPOLL_CTL_DEL, fd, nullptr) < 0) {
        LOG_ERROR("epoll_ctl DEL failed for fd=" << fd << ": " << strerror(errno));
        return false;
    }
    return true;
}

int EpollWrapper::wait(int timeout) {
    return epoll_wait(m_epollFd, m_events.data(), MAX_EVENTS, timeout);
}

int EpollWrapper::getEventFd(int idx) const {
    return m_events[idx].data.fd;
}

void* EpollWrapper::getEventPtr(int idx) const {
    return m_events[idx].data.ptr;
}

uint32_t EpollWrapper::getEventEvents(int idx) const {
    return m_events[idx].events;
}
