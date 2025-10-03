#pragma once

#include "Define.h"
#include "StandardInclude.h"

#include "Renderer/Material.h"
#include "Renderer/Model.h"
#include "Sound/Sound.h"

#include "Threading/ThreadingSystem.h"

namespace BSE
{
    class DLL_EXPORT FileStreamingSystem
    {
    public:
        FileStreamingSystem() = default;
        ~FileStreamingSystem() = default;

        void LoadModelAsync(const std::string& filepath, const std::function<void(std::shared_ptr<ModelLoader>)>& callback);
        void LoadMaterialAsync(const std::string& filepath, const std::function<void(std::shared_ptr<Material>)>& callback);
        void LoadSoundAsync(const std::string& filepath, const std::function<void(std::shared_ptr<SoundBuffer>)>& callback);

        void Update();

    private:
        ThreadingSystem m_threadSystem;
        std::mutex m_queueMutex;

        struct QueuedTask
        {
            std::function<void()> task;
        };
        std::vector<QueuedTask> m_queuedTasks;
    };
}
