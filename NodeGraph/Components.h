#pragma once

#include "../Renderer/Model.h"
#include "../Renderer/Material.h"
#include "../Renderer/Shader.h"
#include "../Renderer/Lighting.h"

#include "../Sound/Sound.h"

#include "Node.h"

namespace BSE
{
    struct ModelComponent : Component
    {
        virtual ~ModelComponent() {}

        virtual void InitComponent() override {}
        virtual void DeleteComponentData() override {}

        std::shared_ptr<Model> model;
        std::shared_ptr<Material> mat;
        std::shared_ptr<ShaderProgram> shaProg;

        void SetModelData(std::shared_ptr<Model> model, std::shared_ptr<Material> mat, std::shared_ptr<ShaderProgram> shaProg)
        {
            this->model = model;
            this->mat = mat;
            this->shaProg = shaProg;
        }

        ModelRenderer* renderer;
        glm::mat4 viewProjMatrix;

        void SetExtras(ModelRenderer& renderer, glm::mat4 viewProjMatrix)
        {
            this->renderer = &renderer;
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
            if (Lighting::ShaderUsesLighting(this->shaProg->GetID()))
            {
                Lighting::Apply(this->shaProg->GetID());
            }
            this->model->Render(*this->renderer, this->viewProjMatrix, this->shaProg->GetID());
            this->shaProg->Unbind();
        }
    };
    
    struct DirectionalLightComponent : Component
    {
        glm::vec3 Direction = glm::vec3(0.0f, -1.0f, 0.0f);
        glm::vec3 Color = glm::vec3(1.0f);
        float Intensity = 1.0f;

        virtual void Update(double Tick) override
        {
            LightData d;
            d.type = LightType::Directional;
            d.direction = glm::normalize(Direction != glm::vec3(0.0f) ? Direction : glm::vec3(0.0f, -1.0f, 0.0f));
            d.color = Color;
            d.intensity = Intensity;
            Lighting::AddLight(d);
        }
    };

    struct PointLightComponent : Component
    {
        glm::vec3 Position = glm::vec3(0.0f);
        glm::vec3 Color = glm::vec3(1.0f);
        float Intensity = 1.0f;
        float Radius = 1.0f;

        virtual void Update(double Tick) override
        {
            LightData d;
            d.type = LightType::Point;
            d.position = Position;
            d.color = Color;
            d.intensity = Intensity;
            d.radius = Radius;
            Lighting::AddLight(d);
        }
    };

    struct SpotLightComponent : Component
    {
        glm::vec3 Position = glm::vec3(0.0f);
        glm::vec3 Direction = glm::vec3(0.0f, -1.0f, 0.0f);
        glm::vec3 Color = glm::vec3(1.0f);
        float Intensity = 1.0f;
        float InnerCone = 0.9f;
        float OuterCone = 0.8f;
        float Radius = 1.0f;

        virtual void Update(double Tick) override
        {
            LightData d;
            d.type = LightType::Spot;
            d.position = Position;
            d.direction = glm::normalize(Direction != glm::vec3(0.0f) ? Direction : glm::vec3(0.0f, -1.0f, 0.0f));
            d.color = Color;
            d.intensity = Intensity;
            d.innerCone = InnerCone;
            d.outerCone = OuterCone;
            d.radius = Radius;
            Lighting::AddLight(d);
        }
    };

    struct AreaLightComponent : Component
    {
        glm::vec3 Position = glm::vec3(0.0f);
        glm::vec3 Direction = glm::vec3(0.0f, -1.0f, 0.0f);
        glm::vec3 Color = glm::vec3(1.0f);
        float Intensity = 1.0f;
        glm::vec2 AreaSize = glm::vec2(1.0f);

        virtual void Update(double Tick) override
        {
            LightData d;
            d.type = LightType::Area;
            d.position = Position;
            d.direction = glm::normalize(Direction != glm::vec3(0.0f) ? Direction : glm::vec3(0.0f, -1.0f, 0.0f));
            d.color = Color;
            d.intensity = Intensity;
            d.areaSize = AreaSize;
            Lighting::AddLight(d);
        }
    };

    struct Camera3DComponent : Component
    {
        glm::vec3 Position = glm::vec3(0.0f);
        glm::vec3 Forward = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 Right = glm::vec3(1.0f, 0.0f, 0.0f);

        float Yaw = -90.0f;
        float Pitch = 0.0f;
        float FOV = 45.0f;

        float NearPlane = 0.1f;
        float FarPlane = 100.0f;

        float AspectRatio = 16.0f / 9.0f;

        glm::mat4 ViewMatrix = glm::mat4(1.0f);
        glm::mat4 ProjectionMatrix = glm::mat4(1.0f);
        glm::mat4 ViewProjMatrix = glm::mat4(1.0f);

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
            Up  = glm::normalize(glm::cross(Right, Forward));
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
        glm::vec3 Up  = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 Forward = glm::vec3(0.0f, 0.0f, -1.0f);

        float Left = -1.0f;
        float Right =  1.0f;
        float Bottom = -1.0f;
        float Top =  1.0f;

        float NearPlane = -1.0f;
        float FarPlane =  1.0f;

        glm::mat4 ViewMatrix = glm::mat4(1.0f);
        glm::mat4 ProjectionMatrix = glm::mat4(1.0f);
        glm::mat4 ViewProjMatrix = glm::mat4(1.0f);

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

    struct SoundComponent : Component
    {
        unsigned int SoundID = 0;

        void SetSoundData(std::shared_ptr<SoundBuffer> buffer, std::shared_ptr<SoundSource> source)
        {
            this->buffer = buffer;
            this->source = source;

            if (this->source && this->buffer)
            {
                this->source->AttachBuffer(*this->buffer);
            }
        }

        void SetSoundProperties(bool loop, float gain, float pitch, const glm::vec3& position, const glm::vec3& velocity)
        {
            if (source)
            {
                source->SetLooping(loop);
                source->SetGain(gain);
                source->SetPitch(pitch);
                source->SetPosition(position);
                source->SetVelocity(velocity);
            }
        }

        void PlaySound()
        {
            if (source)
            {
                source->Play();
            }
        }

        void PauseSound()
        {
            if (source)
            {
                source->Pause();
            }
        }

        void StopSound()
        {
            if (source)
            {
                source->Stop();
            }
        }

        std::shared_ptr<SoundBuffer> buffer;
        std::shared_ptr<SoundSource> source;
    };

    struct TriggerActivatorComponent : Component
    {
        std::shared_ptr<Model> hostModel;

        glm::vec3 position = glm::vec3(0.0f);
        float scale = 1.0f;
        glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

        void SetHostModel(std::shared_ptr<Model> model)
        {
            hostModel = model;
            SyncWithHostModel();
        }

        void SetScale(float s)
        {
            scale = s;
        }

        void SyncWithHostModel()
        {
            if (hostModel)
            {
                position = hostModel->GetPosition();
                rotation = hostModel->GetRotation();
            }
        }

        std::array<glm::vec3, 8> GetAABBVertices() const
        {
            std::array<glm::vec3, 8> vertices;
            float halfScale = scale * 0.5f;

            vertices[0] = position + rotation * glm::vec3(-halfScale, -halfScale, -halfScale);
            vertices[1] = position + rotation * glm::vec3( halfScale, -halfScale, -halfScale);
            vertices[2] = position + rotation * glm::vec3( halfScale,  halfScale, -halfScale);
            vertices[3] = position + rotation * glm::vec3(-halfScale,  halfScale, -halfScale);
            vertices[4] = position + rotation * glm::vec3(-halfScale, -halfScale,  halfScale);
            vertices[5] = position + rotation * glm::vec3( halfScale, -halfScale,  halfScale);
            vertices[6] = position + rotation * glm::vec3( halfScale,  halfScale,  halfScale);
            vertices[7] = position + rotation * glm::vec3(-halfScale,  halfScale,  halfScale);

            return vertices;
        }

        virtual void Update(double Tick) override
        {
            SyncWithHostModel();
        }
    };

    struct TriggerBoxComponent : Component
    {
        glm::vec3 position = glm::vec3(0.0f);
        float scale = 1.0f;
        glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

        void SetPosition(const glm::vec3& pos)
        {
            position = pos;
        }

        void SetScale(float s)
        {
            scale = s;
        }

        void SetRotation(const glm::quat& rot)
        {
            rotation = rot;
        }

        std::array<glm::vec3, 8> GetAABBVertices() const
        {
            std::array<glm::vec3, 8> vertices;
            float halfScale = scale * 0.5f;

            vertices[0] = position + rotation * glm::vec3(-halfScale, -halfScale, -halfScale);
            vertices[1] = position + rotation * glm::vec3( halfScale, -halfScale, -halfScale);
            vertices[2] = position + rotation * glm::vec3( halfScale,  halfScale, -halfScale);
            vertices[3] = position + rotation * glm::vec3(-halfScale,  halfScale, -halfScale);
            vertices[4] = position + rotation * glm::vec3(-halfScale, -halfScale,  halfScale);
            vertices[5] = position + rotation * glm::vec3( halfScale, -halfScale,  halfScale);
            vertices[6] = position + rotation * glm::vec3( halfScale,  halfScale,  halfScale);
            vertices[7] = position + rotation * glm::vec3(-halfScale,  halfScale,  halfScale);

            return vertices;
        }

        bool CheckIfOverlaps(const TriggerActivatorComponent& activator) const
        {
            auto boxVertices = GetAABBVertices();
            auto activatorVertices = activator.GetAABBVertices();

            glm::vec3 boxMin = boxVertices[0];
            glm::vec3 boxMax = boxVertices[0];
            for (const auto& v : boxVertices)
            {
                boxMin = glm::min(boxMin, v);
                boxMax = glm::max(boxMax, v);
            }

            glm::vec3 activatorMin = activatorVertices[0];
            glm::vec3 activatorMax = activatorVertices[0];
            for (const auto& v : activatorVertices)
            {
                activatorMin = glm::min(activatorMin, v);
                activatorMax = glm::max(activatorMax, v);
            }

            return (boxMin.x <= activatorMax.x && boxMax.x >= activatorMin.x) &&
                   (boxMin.y <= activatorMax.y && boxMax.y >= activatorMin.y) &&
                   (boxMin.z <= activatorMax.z && boxMax.z >= activatorMin.z);
        }
    };

    struct TriggerSphereComponent : Component
    {
        glm::vec3 position = glm::vec3(0.0f);
        float radius = 1.0f;

        void SetPosition(const glm::vec3& pos)
        {
            position = pos;
        }

        void SetRadius(float r)
        {
            radius = r;
        }

        bool CheckIfOverlaps(const TriggerActivatorComponent& activator) const
        {
            auto activatorVertices = activator.GetAABBVertices();

            float closestDistSq = std::numeric_limits<float>::max();
            for (const auto& v : activatorVertices)
            {
                glm::vec3 diff = v - position;
                float distSq = glm::dot(diff, diff);
                if (distSq < closestDistSq)
                {
                    closestDistSq = distSq;
                }
            }

            return closestDistSq <= (radius * radius);
        }
    };
}
