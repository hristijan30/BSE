#include "FileSystem.h"

namespace BSE
{
    FileStreamingSystem::FileStreamingSystem(ThreadingSystem& threadSystem)
        : m_threadSystem(threadSystem)
    {}

    void FileStreamingSystem::LoadModelAsync(const std::string& filepath, const std::function<void(std::shared_ptr<ModelLoader>)>& callback)
    {
        m_threadSystem.SubmitTask([this, filepath, callback]()
        {
            auto loader = std::make_shared<ModelLoader>();
            if (!loader->Load(filepath))
            {
                std::cerr << "Failed to load model: " << filepath << std::endl;
            }

            {
                std::lock_guard<std::mutex> lock(m_queueMutex);
                m_queuedTasks.push_back({ [loader, callback]() { callback(loader); } });
            }
        });
    }

    void FileStreamingSystem::LoadMaterialAsync(const std::string& filepath, const std::function<void(std::shared_ptr<Material>)>& callback)
    {
        m_threadSystem.SubmitTask([this, filepath, callback]()
        {
            auto material = std::make_shared<Material>();
            if (!material->LoadFromFile(filepath))
            {
                std::cerr << "Failed to load material: " << filepath << std::endl;
            }

            {
                std::lock_guard<std::mutex> lock(m_queueMutex);
                m_queuedTasks.push_back({ [material, callback]() { callback(material); } });
            }
        });
    }

    void FileStreamingSystem::LoadSoundAsync(const std::string& filepath, const std::function<void(std::shared_ptr<SoundBuffer>)>& callback)
    {
        m_threadSystem.SubmitTask([this, filepath, callback]()
        {
            auto sound = std::make_shared<SoundBuffer>();
            if (!sound->LoadFromFile(filepath))
            {
                std::cerr << "Failed to load sound: " << filepath << std::endl;
            }

            {
                std::lock_guard<std::mutex> lock(m_queueMutex);
                m_queuedTasks.push_back({ [sound, callback]() { callback(sound); } });
            }
        });
    }

    void FileStreamingSystem::Update()
    {
        std::vector<QueuedTask> tasksToExecute;
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            tasksToExecute.swap(m_queuedTasks);
        }

        for (auto& task : tasksToExecute)
        {
            task.task();
        }
    }
}
