#pragma once

#include "Model.h"

namespace BSE
{
    bool LoadModelWithAssimp(const std::string& filepath, std::vector<MeshData>& outMeshes);
}
