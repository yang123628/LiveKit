#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

class TaskQueue {
public:
    using Task = std::function<void()>;

    void push(const Task& task);
    Task pop();
    bool empty() const;
    size_t size() const;
    void stop();

private:
    std::queue<Task> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_cond;
    bool m_stopped = false;
};
