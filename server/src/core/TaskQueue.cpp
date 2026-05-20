#include "core/TaskQueue.h"

void TaskQueue::push(const Task& task) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(task);
    }
    m_cond.notify_one();
}

TaskQueue::Task TaskQueue::pop() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cond.wait(lock, [this] { return !m_queue.empty() || m_stopped; });
    if (m_stopped && m_queue.empty()) {
        return nullptr;
    }
    Task task = m_queue.front();
    m_queue.pop();
    return task;
}

bool TaskQueue::empty() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.empty();
}

size_t TaskQueue::size() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.size();
}

void TaskQueue::stop() {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stopped = true;
    }
    m_cond.notify_all();
}
