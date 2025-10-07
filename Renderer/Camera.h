#pragma once

#include "../Engine/Define.h"
#include "../Engine/StandardInclude.h"
#include "OpenGL.h"

#include "../Engine/Time.h"

namespace BSE
{
    class DLL_EXPORT CoreCamera
    {
    public:
        CoreCamera(
            const glm::vec3& position = glm::vec3(0.0f),
            const glm::vec3& forward = glm::vec3(0.0f, 0.0f, -1.0f),
            const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f)
        );

        glm::mat4 GetViewMatrix() const;
        glm::mat4 GetProjectionMatrix(float aspectRatio, float fovY = 45.0f, float nearPlane = 0.1f, float farPlane = 100.0f) const;

        glm::vec3 Position;
        glm::vec3 Forward;
        glm::vec3 Up;
        glm::vec3 Right;
        float FOV;
    };

    class DLL_EXPORT Camera
    {
    public:
        Camera(BSE::Time& time,
               const glm::vec3& position = glm::vec3(0.0f, 0.0f, 3.0f),
               const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f),
               float yaw = -90.0f,
               float pitch = 0.0f);

        glm::mat4 GetViewMatrix() const;
        glm::mat4 GetProjectionMatrix(float aspectRatio, float nearPlane = 0.1f, float farPlane = 100.0f) const;

        void MoveForward(float delta);
        void MoveRight(float delta);
        void MoveUp(float delta);
        void Rotate(float yawOffset, float pitchOffset, bool constrainPitch = true);
        void SetFOV(float fov);

        glm::vec3 Position;
        glm::vec3 Forward;
        glm::vec3 Up;
        glm::vec3 Right;
        float Yaw;
        float Pitch;
        float FOV;

    private:
        void updateCameraVectors();
        BSE::Time& m_time;
    };

    class DLL_EXPORT OrthographicCamera
    {
    public:
        OrthographicCamera(float left, float right, float bottom, float top,
                           float nearPlane = -1.0f, float farPlane = 1.0f,
                           const glm::vec3& position = glm::vec3(0.0f));

        glm::mat4 GetViewMatrix() const;
        glm::mat4 GetProjectionMatrix() const;

        void SetProjection(float left, float right, float bottom, float top,
                           float nearPlane = -1.0f, float farPlane = 1.0f);

        glm::vec3 Position;
        glm::vec3 Forward;
        glm::vec3 Up;

    private:
        glm::mat4 m_projection;
    };
}
