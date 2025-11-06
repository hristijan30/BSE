#pragma once

#include "../Renderer/Model.h"
#include "../Renderer/Material.h"
#include "../Renderer/Shader.h"

#include "Node.h"

namespace BSE
{
    struct ModelComponent : Component
    {
        virtual ~ModelComponent() = 0;

        virtual void InitComponent() override {}
        virtual void DeleteComponentData() override {}

        std::shared_ptr<Model> model; // Shared so you can update the transform data outside of the component while  
        std::shared_ptr<Material> mat;
        std::shared_ptr<ShaderProgram> shaProg;

        void SetModelData(std::shared_ptr<Model> model, std::shared_ptr<Material> mat, std::shared_ptr<ShaderProgram> shaProg)
        {
            this->model = model;
            this->mat = mat;
            this->shaProg = shaProg;
        }

        ModelRenderer& renderer;
        glm::mat4 viewProjMatrix;

        void SetExtras(ModelRenderer& renderer, glm::mat4 viewProjMatrix) // Put in the loop to always update the view-projection matrix
        {
            this->renderer = renderer;
            this->viewProjMatrix = viewProjMatrix;
        }

        virtual void Update(double Tick) override
        {
            this->model->UpdateRenderTransforms();
        }

        virtual void Render(double Alpha) override
        {
            this->shaProg->Bind();
            this->mat->Bind(this->shaProg->GetID());
            this->model->Render(this->renderer, this->viewProjMatrix, this->shaProg->GetID());
            this->shaProg->Unbind();
        }
    };

    struct Camera3DComponent : Component
    {
        glm::vec3 Position = glm::vec3(0.0f);
        glm::vec3 Forward  = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 Up       = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 Right    = glm::vec3(1.0f, 0.0f, 0.0f);

        float Yaw   = -90.0f;
        float Pitch = 0.0f;
        float FOV   = 45.0f;

        float NearPlane = 0.1f;
        float FarPlane  = 100.0f;

        float AspectRatio = 16.0f / 9.0f;

        glm::mat4 ViewMatrix       = glm::mat4(1.0f);
        glm::mat4 ProjectionMatrix = glm::mat4(1.0f);
        glm::mat4 ViewProjMatrix   = glm::mat4(1.0f);

        virtual void Update(double Tick) override
        {
            UpdateCameraVectors();
            UpdateViewMatrix();
            UpdateProjectionMatrix();
            ViewProjMatrix = ProjectionMatrix * ViewMatrix;
        }

        glm::mat4 GetViewMatrix() const       { return ViewMatrix; }
        glm::mat4 GetProjectionMatrix() const { return ProjectionMatrix; }
        glm::mat4 GetViewProjMatrix() const   { return ViewProjMatrix; }

    private:
        void UpdateCameraVectors()
        {
            glm::vec3 front;
            front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
            front.y = sin(glm::radians(Pitch));
            front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
            Forward = glm::normalize(front);

            Right = glm::normalize(glm::cross(Forward, glm::vec3(0.0f, 1.0f, 0.0f)));
            Up    = glm::normalize(glm::cross(Right, Forward));
        }

        void UpdateViewMatrix()
        {
            ViewMatrix = glm::lookAt(Position, Position + Forward, Up);
        }

        void UpdateProjectionMatrix()
        {
            ProjectionMatrix = glm::perspective(glm::radians(FOV), AspectRatio, NearPlane, FarPlane);
        }
    };

    struct Camera2DComponent : Component
    {
        glm::vec3 Position = glm::vec3(0.0f);
        glm::vec3 Up       = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 Forward  = glm::vec3(0.0f, 0.0f, -1.0f);

        float Left   = -1.0f;
        float Right  =  1.0f;
        float Bottom = -1.0f;
        float Top    =  1.0f;

        float NearPlane = -1.0f;
        float FarPlane  =  1.0f;

        glm::mat4 ViewMatrix       = glm::mat4(1.0f);
        glm::mat4 ProjectionMatrix = glm::mat4(1.0f);
        glm::mat4 ViewProjMatrix   = glm::mat4(1.0f);

        virtual void Update(double Tick) override
        {
            UpdateViewMatrix();
            UpdateProjectionMatrix();
            ViewProjMatrix = ProjectionMatrix * ViewMatrix;
        }

        glm::mat4 GetViewMatrix() const       { return ViewMatrix; }
        glm::mat4 GetProjectionMatrix() const { return ProjectionMatrix; }
        glm::mat4 GetViewProjMatrix() const   { return ViewProjMatrix; }

    private:
        void UpdateViewMatrix()
        {
            ViewMatrix = glm::lookAt(Position, Position + Forward, Up);
        }

        void UpdateProjectionMatrix()
        {
            ProjectionMatrix = glm::ortho(Left, Right, Bottom, Top, NearPlane, FarPlane);
        }
    };
}