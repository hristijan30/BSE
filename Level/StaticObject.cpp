#include "StaticObject.h"

namespace BSE
{
    void StaticObject::Initialize(const std::string& nodeName, std::string modelPath, std::shared_ptr<Material> mat, std::shared_ptr<ShaderProgram> shaProg)
    {
        m_model = std::make_shared<Model>();
        m_model->LoadFromFile(modelPath);

        m_modelComponent = std::make_shared<ModelComponent>();
        m_modelComponent->SetModelData(m_model, mat, shaProg);

        m_node = new Node(nodeName);
        m_node->AddComponent(m_modelComponent, "ModelComponent");
        m_node->InitNode();
    }

    void StaticObject::Delete()
    {
        if (m_node)
        {
            m_node->DeleteNode();
            delete m_node;
            m_node = nullptr;
        }
        m_modelComponent.reset();
        m_model.reset();
    }

    void StaticObject::SetPosition(const glm::vec3& pos)
    {
        if (m_model)
        {
            m_model->SetPosition(pos);
        }
        m_modelComponent->Update(0.0);
    }

    void StaticObject::SetRotation(const glm::quat& rot)
    {
        if (m_model)
        {
            m_model->SetRotation(rot);
        }
        m_modelComponent->Update(0.0);
    }

    void StaticObject::SetScale(const glm::vec3& scale)
    {
        if (m_model)
        {
            m_model->SetScale(scale);
        }
        m_modelComponent->Update(0.0);
    }

    void StaticObject::Render(ModelRenderer& renderer, const glm::mat4& viewProjMatrix)
    {
        if (m_modelComponent)
        {
            m_modelComponent->SetExtras(renderer, viewProjMatrix);
            m_modelComponent->Render(0.0);
        }
    }
}
