#pragma once

#include "../Define.h"
#include "../StandardInclude.h"

namespace BSE
{
    class DLL_EXPORT Node
    {
    public:
        Node(const std::string& name = "Node");
        virtual ~Node();

        void SetParent(Node* parent);
        void AddChild(std::shared_ptr<Node> child);
        void RemoveChild(std::shared_ptr<Node> child);

        const std::vector<std::shared_ptr<Node>>& GetChildren() const { return m_children; }
        Node* GetParent() const { return m_parent; }
        const std::string& GetName() const { return m_name; }

    protected:
        std::string m_name;
        Node* m_parent = nullptr;
        std::vector<std::shared_ptr<Node>> m_children;
    };
}
