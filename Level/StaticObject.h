#pragma once

#include "../NodeGraph/Components.h"
#include "../NodeGraph/Node.h"

namespace BSE
{
    class DLL_EXPORT StaticObject
    {
    public:
        StaticObject() = default;
        ~StaticObject() = default;

        void Initialize(const std::string& nodeName, std::string modelPath, std::shared_ptr<Material> mat, std::shared_ptr<ShaderProgram> shaProg);
        void Delete();

        void SetPosition(const glm::vec3& pos);
        void SetRotation(const glm::quat& rot);
        void SetScale(const glm::vec3& scale);

        void Render(ModelRenderer& renderer, const glm::mat4& viewProjMatrix);

    private:
        std::shared_ptr<Model> m_model;
        
        std::shared_ptr<ModelComponent> m_modelComponent;
        Node* m_node = nullptr;
    };
}