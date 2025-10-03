#include "Scene.h"

namespace BSE
{
    Scene::Scene(const std::string& name)
        : Node(name)
    {}

    Scene::~Scene()
    {
        m_children.clear();
    }

    void Scene::UpdateAll(float tick)
    {
        Update(tick);

        for (auto& child : m_children)
        {
            if (child)
                child->Update(tick);
        }
    }

    void Scene::RenderAll(float alpha)
    {
        Render(alpha);

        for (auto& child : m_children)
        {
            if (child)
                child->Render(alpha);
        }
    }
}
