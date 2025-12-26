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
}