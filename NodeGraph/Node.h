#pragma once

#include "../Engine/Define.h"
#include "../Engine/StandardInclude.h"

namespace BSE
{
    struct Component
    {
        virtual ~Component() = default;
        virtual void InitComponent() {}
        virtual void DeleteComponentData() {}

        // Alpha and tick are used to make the program run at a fixed update time 
            //but render wise can run at a much larger framerate whit out braking the update code by running to fast
        virtual void Update(double Tick) {}
        virtual void Render(double Alpha) {}
    };

    class DLL_EXPORT Node
    {
    public:
        explicit Node(std::string name)
            : name(std::move(name)) {}
        virtual ~Node() = default;

        virtual bool InitNode()
        {
            for (auto& pair : childrenByName)
            {
                if (pair.second)
                    pair.second->InitNode();
            }

            for (auto& pair : components)
            {
                if (pair.second)
                    pair.second->InitComponent();
            }
            return true;
        }

        virtual void DeleteNode()
        {
            for (auto& pair : childrenByName)
            {
                if (pair.second)
                    pair.second->DeleteNode();
            }

            for (auto& pair : components)
            {
                if (pair.second)
                    pair.second->DeleteComponentData();
            }
        }

        virtual void UpdateNode(double Tick)
        {
            for (auto& pair : components)
            {
                if (pair.second)
                    pair.second->Update(Tick);
            }

            for (auto& pair : childrenByName)
            {
                auto& child = pair.second;
                if (child)
                    child->UpdateNode(Tick);
            }
        }

        virtual void RenderNode(double Alpha)
        {
            for (auto& pair : components)
            {
                if (pair.second)
                    pair.second->Render(Alpha);
            }

            for (auto& pair : childrenByName)
            {
                auto& child = pair.second;
                if (child)
                    child->RenderNode(Alpha);
            }
        }

        std::string GetName() const { return name; }

        bool HasChild(const std::string& childName) const
        {
            return childrenByName.find(childName) != childrenByName.end();
        }

        bool AddChild(std::shared_ptr<Node> node)
        {
            if (!node)
                return false;

            const auto& nodeName = node->GetName();
            if (HasChild(nodeName))
                return false;

            childrenByName[nodeName] = std::move(node);
            return true;
        }

        bool RemoveChild(const std::string& childName)
        {
            return childrenByName.erase(childName) > 0;
        }

        bool HasComponent(const std::string& compName) const
        {
            return components.find(compName) != components.end();
        }

        bool AddComponent(std::unique_ptr<Component> component, const std::string& compName)
        {
            if (!component)
                return false;

            if (HasComponent(compName))
                return false;

            components[compName] = std::move(component);
            return true;
        }

        bool RemoveComponent(const std::string& compName)
        {
            return components.erase(compName) > 0;
        }

        std::unique_ptr<Component> ExtractComponent(const std::string& compName)
        {
            auto it = components.find(compName);
            if (it == components.end())
                return nullptr;

            std::unique_ptr<Component> comp = std::move(it->second);
            components.erase(it);
            return comp;
        }

    private:
        std::string name;
        std::unordered_map<std::string, std::shared_ptr<Node>> childrenByName;
        std::unordered_map<std::string, std::unique_ptr<Component>> components;
    };
}
