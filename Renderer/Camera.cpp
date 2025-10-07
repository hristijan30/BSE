#include "Camera.h"
#include <algorithm>

namespace BSE
{
    CoreCamera::CoreCamera(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up)
        : Position(position), Forward(glm::normalize(forward)), Up(glm::normalize(up)), FOV(45.0f)
    {
        Right = glm::normalize(glm::cross(Forward, Up));
    }

    glm::mat4 CoreCamera::GetViewMatrix() const
    {
        return glm::lookAt(Position, Position + Forward, Up);
    }

    glm::mat4 CoreCamera::GetProjectionMatrix(float aspectRatio, float fovY, float nearPlane, float farPlane) const
    {
        return glm::perspective(glm::radians(fovY), aspectRatio, nearPlane, farPlane);
    }

    Camera::Camera(BSE::Time& time, const glm::vec3& position, const glm::vec3& up, float yaw, float pitch)
        : m_time(time), Position(position), Up(up), Yaw(yaw), Pitch(pitch), FOV(45.0f)
    {
        updateCameraVectors();
    }

    glm::mat4 Camera::GetViewMatrix() const
    {
        return glm::lookAt(Position, Position + Forward, Up);
    }

    glm::mat4 Camera::GetProjectionMatrix(float aspectRatio, float nearPlane, float farPlane) const
    {
        return glm::perspective(glm::radians(FOV), aspectRatio, nearPlane, farPlane);
    }

    void Camera::MoveForward(float delta) { Position += Forward * delta * static_cast<float>(m_time.GetDeltaTime()); }
    void Camera::MoveRight(float delta)   { Position += Right   * delta * static_cast<float>(m_time.GetDeltaTime()); }
    void Camera::MoveUp(float delta)      { Position += Up      * delta * static_cast<float>(m_time.GetDeltaTime()); }

    void Camera::Rotate(float yawOffset, float pitchOffset, bool constrainPitch)
    {
        Yaw   += yawOffset * static_cast<float>(m_time.GetDeltaTime());
        Pitch += pitchOffset * static_cast<float>(m_time.GetDeltaTime());

        if (constrainPitch)
            Pitch = std::clamp(Pitch, -89.0f, 89.0f);

        updateCameraVectors();
    }

    void Camera::SetFOV(float fov)
    {
        FOV = std::clamp(fov, 1.0f, 120.0f);
    }

    void Camera::updateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Forward = glm::normalize(front);

        Right = glm::normalize(glm::cross(Forward, glm::vec3(0.0f,1.0f,0.0f)));
        Up    = glm::normalize(glm::cross(Right, Forward));
    }

    OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top,
                                           float nearPlane, float farPlane,
                                           const glm::vec3& position)
        : Position(position), Forward(glm::vec3(0.0f, 0.0f, -1.0f)), Up(glm::vec3(0.0f, 1.0f, 0.0f))
    {
        SetProjection(left, right, bottom, top, nearPlane, farPlane);
    }

    glm::mat4 OrthographicCamera::GetViewMatrix() const
    {
        return glm::lookAt(Position, Position + Forward, Up);
    }

    glm::mat4 OrthographicCamera::GetProjectionMatrix() const
    {
        return m_projection;
    }

    void OrthographicCamera::SetProjection(float left, float right, float bottom, float top,
                                           float nearPlane, float farPlane)
    {
        m_projection = glm::ortho(left, right, bottom, top, nearPlane, farPlane);
    }
}
