#include "Node.h"

namespace BSE
{
    Node::Node(const std::string& name)
        : m_name(name), m_parent(nullptr)
    {}

    Node::~Node()
    {
        for (auto& child : m_children)
        {
            child->m_parent = nullptr;
        }
        m_children.clear();
    }

    void Node::SetParent(Node* parent)
    {
        if (m_parent == parent) return;

        if (m_parent)
        {
            auto& siblings = m_parent->m_children;
            siblings.erase(std::remove_if(siblings.begin(), siblings.end(),
                [this](const std::shared_ptr<Node>& n) { return n.get() == this; }),
                siblings.end());
        }

        m_parent = parent;

        if (m_parent)
        {
            m_parent->m_children.push_back(std::shared_ptr<Node>(this, [](Node*) {}));
        }
    }

    std::vector<std::shared_ptr<Node>> Node::GetAllDescendants() const
    {
        std::vector<std::shared_ptr<Node>> descendants;

        for (const auto& child : m_children)
        {
            descendants.push_back(child);
            auto childDescendants = child->GetAllDescendants();
            descendants.insert(descendants.end(), childDescendants.begin(), childDescendants.end());
        }

        return descendants;
    }

    void Node::AddChild(std::shared_ptr<Node> child)
    {
        if (!child) return;

        child->SetParent(this);
    }

    void Node::RemoveChild(std::shared_ptr<Node> child)
    {
        if (!child) return;

        auto it = std::find(m_children.begin(), m_children.end(), child);
        if (it != m_children.end())
        {
            (*it)->m_parent = nullptr;
            m_children.erase(it);
        }
    }
}
