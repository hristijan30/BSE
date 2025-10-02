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
}
