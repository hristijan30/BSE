#include "ThreadingSystem.h"

namespace BSE
{
    ThreadingSystem::~ThreadingSystem()
    {
        m_taskGroup.wait();
    }

    void ThreadingSystem::WaitAll()
    {
        m_taskGroup.wait();

        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_completedTasks.clear();
    }

    void ThreadingSystem::AddCompletedTask(const std::function<void()>& task)
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_completedTasks.push_back(task);
    }

    std::vector<std::function<void()>> ThreadingSystem::RetrieveCompletedTasks()
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        auto tasks = std::move(m_completedTasks);
        m_completedTasks.clear();
        return tasks;
    }

    ThreadPool::ThreadPool(ThreadingSystem& threadingSystem, size_t threadCount)
        : m_threadingSystem(threadingSystem)
        , m_threadCount(threadCount)
        , m_arena(static_cast<int>(threadCount == 0 ? std::max(1u, std::thread::hardware_concurrency()) : threadCount))
    {
        if (m_threadCount == 0)
            m_threadCount = std::max<size_t>(1, std::thread::hardware_concurrency());
    }

    ThreadPool::~ThreadPool()
    {
        m_shuttingDown.store(true, std::memory_order_release);
        WaitAll();
    }
}

