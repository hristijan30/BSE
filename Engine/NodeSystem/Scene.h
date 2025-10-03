#pragma once

#include "Node.h"

namespace BSE
{
    class DLL_EXPORT Scene : public Node
    {
    public:
        Scene(const std::string& name = "Scene");
        ~Scene();

        void UpdateAll(float tick);
        void RenderAll(float alpha);
    };
}
