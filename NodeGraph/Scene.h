#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include "Node.h"

namespace BSE
{
    class DLL_EXPORT Scene
    {
    public:
        Scene(std::string name)
            : name(std::move(name)) {}
        ~Scene() = default;

        void InitScene()
        {
            for (auto& pair : nodesListUnique)
            {
                if (pair.second)
                    pair.second->InitNode();
            }

            for (auto& pair : nodesListShared)
            {
                if (pair.second)
                    pair.second->InitNode();
            }
        }

        void DeleteScene()
        {
            for (auto& pair : nodesListUnique)
            {
                if (pair.second)
                    pair.second->DeleteNode();
            }
            nodesListUnique.clear();

            for (auto& pair : nodesListShared)
            {
                if (pair.second)
                    pair.second->DeleteNode();
            }
            nodesListShared.clear();
        }

        void Update(double Tick)
        {
            for (auto& pair : nodesListUnique)
            {
                if (pair.second)
                    pair.second->UpdateNode(Tick);
            }

            for (auto& pair : nodesListShared)
            {
                if (pair.second)
                    pair.second->UpdateNode(Tick);
            }
        }

        void Render(double Alpha)
        {
            for (auto& pair : nodesListUnique)
            {
                if (pair.second)
                    pair.second->RenderNode(Alpha);
            }

            for (auto& pair : nodesListShared)
            {
                if (pair.second)
                    pair.second->RenderNode(Alpha);
            }
        }
        
        void AddNodeUnique(std::unique_ptr<Node> node)
        {
            if (!node)
                return;

            const std::string nodeName = node->GetName();
            if (nodeName.empty())
                return;

            if (nodesListUnique.find(nodeName) != nodesListUnique.end())
                return;
            if (nodesListShared.find(nodeName) != nodesListShared.end())
                return;

            nodesListUnique.emplace(nodeName, std::move(node));
        }

        void AddNodeShared(std::shared_ptr<Node> node)
        {
            if (!node)
                return;

            const std::string nodeName = node->GetName();
            if (nodeName.empty())
                return;

            if (nodesListShared.find(nodeName) != nodesListShared.end())
                return;
            if (nodesListUnique.find(nodeName) != nodesListUnique.end())
                return;

            nodesListShared.emplace(nodeName, std::move(node));
        }

        void RemoveNode(std::string name)
        {
            if (name.empty())
                return;

            auto itU = nodesListUnique.find(name);
            if (itU != nodesListUnique.end())
            {
                if (itU->second)
                    itU->second->DeleteNode();
                nodesListUnique.erase(itU);
            }

            auto itS = nodesListShared.find(name);
            if (itS != nodesListShared.end())
            {
                if (itS->second)
                    itS->second->DeleteNode();
                nodesListShared.erase(itS);
            }
        }

    private:
        std::string name;

        std::unordered_map<std::string, std::unique_ptr<Node>> nodesListUnique;
        std::unordered_map<std::string, std::shared_ptr<Node>> nodesListShared;
    };
}
