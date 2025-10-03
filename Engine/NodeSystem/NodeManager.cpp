#include "NodeManager.h"

namespace BSE
{
    std::shared_ptr<Scene> NodeManager::CreateScene(const std::string& name)
    {
        auto scene = std::make_shared<Scene>(name);
        m_scenes.push_back(scene);

        if (!m_activeScene)
        {
            m_activeScene = scene;
        }

        return scene;
    }

    void NodeManager::SetActiveScene(std::shared_ptr<Scene> scene)
    {
        if (!scene) return;

        auto it = std::find(m_scenes.begin(), m_scenes.end(), scene);
        if (it != m_scenes.end())
        {
            m_activeScene = scene;
        }
        else
        {
            std::cerr << "NodeManager: Attempted to set active scene that is not managed!" << std::endl;
        }
    }
}
