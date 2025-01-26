#ifndef _cronus_task_system_h_
#define _cronus_task_system_h_

#include <functional>
#include <future>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <memory>

namespace cronus {

class TaskSystem {
public:
    using Task = std::function<void()>;

    TaskSystem();
    ~TaskSystem();

    template<typename F>
    auto enqueue(F&& f) -> std::future<decltype(f())> {
        using ReturnType = decltype(f());
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::forward<F>(f));
        auto future = task->get_future();
        
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_tasks.emplace([task]() { (*task)(); });
        }
        
        m_condition.notify_one();
        return future;
    }

    void stop();

private:
    std::queue<Task> m_tasks;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::thread m_worker;
    bool m_running;

    void workerLoop();
};

} // namespace cronus

#endif//_cronus_task_system_h_
