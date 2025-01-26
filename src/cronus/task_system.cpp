#include "cronus/task_system.h"

namespace cronus {

TaskSystem::TaskSystem() : m_running(true) {
    m_worker = std::thread(&TaskSystem::workerLoop, this);
}

TaskSystem::~TaskSystem() {
    stop();
}

void TaskSystem::stop() {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_running = false;
    }
    m_condition.notify_all();
    if (m_worker.joinable()) {
        m_worker.join();
    }
}

void TaskSystem::workerLoop() {
    while (true) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_condition.wait(lock, [this] { 
                return !m_tasks.empty() || !m_running; 
            });
            
            if (!m_running && m_tasks.empty()) {
                return;
            }
            
            task = std::move(m_tasks.front());
            m_tasks.pop();
        }
        task();
    }
}

} // namespace cronus
