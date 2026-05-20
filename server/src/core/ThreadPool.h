#pragma once

#include <vector>
#include <thread>
#include <future>
#include <atomic>
#include "core/TaskQueue.h"

class ThreadPool {
public:
    explicit ThreadPool(int numThreads);
    ~ThreadPool();

    template<typename F>
    auto submit(F&& f) -> std::future<decltype(f())>;

    void start();
    void stop();

private:
    void workerThread();

    int m_numThreads;
    std::vector<std::thread> m_threads;
    TaskQueue m_taskQueue;
    std::atomic<bool> m_running;
};

template<typename F>
auto ThreadPool::submit(F&& f) -> std::future<decltype(f())> {
    using ReturnType = decltype(f());
    auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::forward<F>(f));
    std::future<ReturnType> result = task->get_future();
    m_taskQueue.push([task]() { (*task)(); });
    return result;
}
