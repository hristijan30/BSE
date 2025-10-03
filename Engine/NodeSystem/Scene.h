#pragma once

#include "Node.h"
#include <string>

namespace BSE
{
    class DLL_EXPORT Scene : public Node
    {
    public:
        Scene(const std::string& name = "Scene");
        ~Scene();
    };
}
