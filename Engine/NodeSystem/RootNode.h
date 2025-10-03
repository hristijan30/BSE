#pragma once

#include "Node.h"

namespace BSE
{
    class DLL_EXPORT RootNode : public Node
    {
    public:
        RootNode(const std::string& name = "RootNode");
        ~RootNode();

        // Placeholder for custom behavior like camera, lights, mesh, etc.
    };
}
