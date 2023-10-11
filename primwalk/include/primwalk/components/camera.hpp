#pragma once

// primwalk
#include "primwalk/core.hpp"

// std
#include <memory>


// vendor
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace pw {
  class PW_API Camera {
  public:
    Camera() = default;
    ~Camera() = default;

    void update();

    inline glm::vec3 getPosition() const { return m_Position; }
    inline glm::mat4 getViewMatrix() const { return m_ViewMatrix; }
    inline float getFOV() const { return m_FOV; }
    inline float getNearClip() const { return m_NearClip; }
    inline float getFarClip() const { return m_FarClip; }
    inline float getYaw() const { return m_Yaw; }
    inline float getPitch() const { return m_Pitch; }
    inline glm::vec3 getForward() const { return m_Forward; }
    inline glm::vec3 getRight() const { return m_Right; }
    inline glm::vec3 getUp() const { return m_Up; }

    inline void setPosition(glm::vec3 position) { m_Position = position; }
    inline void setFOV(float fov) { m_FOV = fov; }
    inline void setNearClip(float clip) { m_NearClip = clip; }
    inline void setFarClip(float clip) { m_FarClip = clip; }
    inline void setYaw(float radians) { m_Yaw = glm::mod(radians, glm::two_pi<float>()); }
    inline void setPitch(float radians) { m_Pitch = glm::clamp(radians, -1.57f, 1.57f); }

    inline static std::shared_ptr<Camera> MainCamera = std::make_shared<Camera>();

  private:
    glm::vec3 m_Position = glm::vec3(0.0f);
    float m_FOV = 45.0f;
    float m_NearClip = 0.01f;
    float m_FarClip = 100.0f;

    float m_Yaw = glm::pi<float>() / 2.0f;
    float m_Pitch = 0.0f;

    glm::vec3 m_Forward = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 m_Up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 m_Right = glm::vec3(1.0f, 0.0f, 0.0f);

    glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
    glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
  };
}