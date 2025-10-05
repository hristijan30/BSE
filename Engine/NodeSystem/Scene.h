#pragma once

#include "Node.h"

namespace BSE
{
    class DLL_EXPORT Scene : public Node
    {
    public:
        using UpdateCallback = std::function<void(Scene&, float)>;
        using RenderCallback = std::function<void(Scene&, double)>;

        using UpdateSystem   = std::function<void(Scene&, float)>;
        using RenderSystem   = std::function<void(Scene&, double)>;
        using SystemId       = uint64_t;

        Scene(const std::string& name = "Scene");
        ~Scene();

        void UpdateAll(float tick);
        void RenderAll(double alpha);

        void TraverseChildren(const std::function<void(Node&)>& callback);
        void TraverseChildrenRecursive(Node& parent, const std::function<void(Node&)>& callback);

        void SetUpdateCallback(UpdateCallback cb);
        void ClearUpdateCallback();
        void SetRenderCallback(RenderCallback cb);
        void ClearRenderCallback();

        SystemId RegisterUpdateSystem(int priority, UpdateSystem sys);
        bool UnregisterUpdateSystem(SystemId id);

        SystemId RegisterRenderSystem(int priority, RenderSystem sys);
        bool UnregisterRenderSystem(SystemId id);

        template<typename T, typename... Args>
        std::shared_ptr<T> AddChildNode(Args&&... args)
        {
            static_assert(std::is_base_of<Node, T>::value, "T must derive from Node");
            auto node = std::make_shared<T>(std::forward<Args>(args)...);
            AddChild(node);
            return node;
        }

    private:
        void ExecuteUpdateSystems(float tick);
        void ExecuteRenderSystems(double alpha);

    private:
        UpdateCallback m_customUpdate;
        RenderCallback m_customRender;

        struct UpdateEntry { int priority; SystemId id; UpdateSystem fn; };
        struct RenderEntry { int priority; SystemId id; RenderSystem fn; };

        std::vector<UpdateEntry> m_updateSystems;
        std::vector<RenderEntry> m_renderSystems;

        SystemId m_nextSystemId = 1;
    };
}
