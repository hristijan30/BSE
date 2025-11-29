#pragma once

#include "../Engine/Define.h"
#include "../Engine/StandardInclude.h"

#include <tbb/task_group.h>
#include <tbb/task_arena.h>

namespace BSE
{
    class DLL_EXPORT ThreadingSystem
    {
    public:
        ThreadingSystem() = default;
        ~ThreadingSystem();
        
        template<typename Func>
        void SubmitTask(Func&& task)
        {
            m_taskGroup.run(std::forward<Func>(task));
        }

        void WaitAll();

        void AddCompletedTask(const std::function<void()>& task);
        std::vector<std::function<void()>> RetrieveCompletedTasks();

    private:
        tbb::task_group m_taskGroup;

        std::mutex m_queueMutex;
        std::vector<std::function<void()>> m_completedTasks;
    };

    class DLL_EXPORT ThreadPool
    {
    public:
        ThreadPool(ThreadingSystem& threadingSystem, size_t threadCount = 0);
        ~ThreadPool();

        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;

        template<typename Func>
        void Submit(Func&& task)
        {
            if (m_shuttingDown.load(std::memory_order_acquire))
                return;

            auto work = std::function<void()>(std::forward<Func>(task));

            m_arena.enqueue([this, work = std::move(work)]() mutable {
                m_taskGroup.run([work = std::move(work)]() mutable {
                    work();
                });
            });
        }

        template<typename Func, typename Completion>
        void SubmitWithCompletion(Func&& task, Completion&& onComplete)
        {
            if (m_shuttingDown.load(std::memory_order_acquire))
                return;

            auto work = std::function<void()>(std::forward<Func>(task));
            auto completion = std::function<void()>(std::forward<Completion>(onComplete));

            m_arena.enqueue([this, work = std::move(work), completion = std::move(completion)]() mutable {
                m_taskGroup.run([this, work = std::move(work), completion = std::move(completion)]() mutable {
                    work();

                    m_threadingSystem.AddCompletedTask(completion);
                });
            });
        }

        void WaitAll()
        {
            m_taskGroup.wait();
        }
        size_t GetThreadCount() const noexcept { return m_threadCount; }

    private:
        ThreadingSystem& m_threadingSystem;
        tbb::task_arena m_arena;
        tbb::task_group m_taskGroup;
        std::atomic<bool> m_shuttingDown{ false };
        size_t m_threadCount{ 0 };
    };
}
