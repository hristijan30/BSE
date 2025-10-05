#include "Scene.h"

namespace BSE
{
    Scene::Scene(const std::string& name)
        : Node(name)
        , m_customUpdate(nullptr)
        , m_customRender(nullptr)
        , m_nextSystemId(1)
    {}

    Scene::~Scene()
    {
        m_children.clear();
        m_updateSystems.clear();
        m_renderSystems.clear();
    }

    void Scene::UpdateAll(float tick)
    {
        if (m_customUpdate)
        {
            m_customUpdate(*this, tick);
            return;
        }

        ExecuteUpdateSystems(tick);

        Update(tick);
        TraverseChildren([tick](Node& n) { n.Update(tick); });
    }

    void Scene::RenderAll(double alpha)
    {
        if (m_customRender)
        {
            m_customRender(*this, alpha);
            return;
        }

        ExecuteRenderSystems(alpha);

        Render(alpha);
        TraverseChildren([alpha](Node& n) { n.Render(alpha); });
    }

    void Scene::TraverseChildren(const std::function<void(Node&)>& callback)
    {
        for (auto& child : m_children)
        {
            if (!child) continue;
            callback(*child);
            if (!child->GetChildren().empty())
                TraverseChildrenRecursive(*child, callback);
        }
    }

    void Scene::TraverseChildrenRecursive(Node& parent, const std::function<void(Node&)>& callback)
    {
        for (auto& child : parent.GetChildren())
        {
            if (!child) continue;
            callback(*child);
            if (!child->GetChildren().empty())
                TraverseChildrenRecursive(*child, callback);
        }
    }

    void Scene::SetUpdateCallback(UpdateCallback cb)
    {
        m_customUpdate = std::move(cb);
    }

    void Scene::ClearUpdateCallback()
    {
        m_customUpdate = nullptr;
    }

    void Scene::SetRenderCallback(RenderCallback cb)
    {
        m_customRender = std::move(cb);
    }

    void Scene::ClearRenderCallback()
    {
        m_customRender = nullptr;
    }

    Scene::SystemId Scene::RegisterUpdateSystem(int priority, UpdateSystem sys)
    {
        SystemId id = m_nextSystemId++;
        m_updateSystems.push_back({ priority, id, std::move(sys) });
        // keep vector sorted by priority (low -> high)
        std::sort(m_updateSystems.begin(), m_updateSystems.end(),
            [](const UpdateEntry& a, const UpdateEntry& b) { return a.priority < b.priority; });
        return id;
    }

    bool Scene::UnregisterUpdateSystem(SystemId id)
    {
        auto it = std::remove_if(m_updateSystems.begin(), m_updateSystems.end(),
            [id](const UpdateEntry& e) { return e.id == id; });
        if (it == m_updateSystems.end()) return false;
        m_updateSystems.erase(it, m_updateSystems.end());
        return true;
    }

    Scene::SystemId Scene::RegisterRenderSystem(int priority, RenderSystem sys)
    {
        SystemId id = m_nextSystemId++;
        m_renderSystems.push_back({ priority, id, std::move(sys) });
        std::sort(m_renderSystems.begin(), m_renderSystems.end(),
            [](const RenderEntry& a, const RenderEntry& b) { return a.priority < b.priority; });
        return id;
    }

    bool Scene::UnregisterRenderSystem(SystemId id)
    {
        auto it = std::remove_if(m_renderSystems.begin(), m_renderSystems.end(),
            [id](const RenderEntry& e) { return e.id == id; });
        if (it == m_renderSystems.end()) return false;
        m_renderSystems.erase(it, m_renderSystems.end());
        return true;
    }

    void Scene::ExecuteUpdateSystems(float tick)
    {
        for (const auto& entry : m_updateSystems)
        {
            if (entry.fn) entry.fn(*this, tick);
        }
    }

    void Scene::ExecuteRenderSystems(double alpha)
    {
        for (const auto& entry : m_renderSystems)
        {
            if (entry.fn) entry.fn(*this, alpha);
        }
    }
}
