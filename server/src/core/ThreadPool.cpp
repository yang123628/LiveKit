#include "core/ThreadPool.h"
#include "core/Logger.h"

ThreadPool::ThreadPool(int numThreads)
    : m_numThreads(numThreads),
      m_running(false) {
}

ThreadPool::~ThreadPool() {
    stop();
}

void ThreadPool::start() {
    m_running = true;
    for (int i = 0; i < m_numThreads; ++i) {
        m_threads.emplace_back(&ThreadPool::workerThread, this);
    }
    LOG_INFO("ThreadPool started with " << m_numThreads << " threads");
}

void ThreadPool::stop() {
    m_running = false;
    m_taskQueue.stop();
    for (auto& t : m_threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    m_threads.clear();
    LOG_INFO("ThreadPool stopped");
}

void ThreadPool::workerThread() {
    while (m_running) {
        auto task = m_taskQueue.pop();
        if (task) {
            task();
        }
    }
}
