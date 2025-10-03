#pragma once

#include "../Define.h"
#include "../StandardInclude.h"

#include <tbb/task_group.h>

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
}
