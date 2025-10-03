#pragma once

#include "Node.h"
#include "Scene.h"

namespace BSE
{
    class DLL_EXPORT NodeManager
    {
    public:
        NodeManager() = default;
        ~NodeManager() = default;

        std::shared_ptr<Scene> CreateScene(const std::string& name);
        void SetActiveScene(std::shared_ptr<Scene> scene);
        std::shared_ptr<Scene> GetActiveScene() const { return m_activeScene; }

        const std::vector<std::shared_ptr<Scene>>& GetScenes() const { return m_scenes; }

    private:
        std::vector<std::shared_ptr<Scene>> m_scenes;
        std::shared_ptr<Scene> m_activeScene = nullptr;
    };
}
