#pragma once

// primwalk
#include "primwalk/core.hpp"

// std
#include <memory>

// vendor
#include <glm/glm.hpp>

namespace pw {
  class PW_API Camera {
  public:
    Camera() = default;
    ~Camera() = default;

    inline static std::shared_ptr<Camera> MainCamera = std::make_shared<Camera>();

    inline glm::vec3 getPosition() const { return m_Position; }
    inline float getFOV() const { return m_FOV; }
    inline float getNearClip() const { return m_NearClip; }
    inline float getFarClip() const { return m_FarClip; }

    inline void setFOV(float fov) { m_FOV = fov; }
    inline void setNearClip(float clip) { m_NearClip = clip; }
    inline void setFarClip(float clip) { m_FarClip = clip; }

  private:
    glm::vec3 m_Position = glm::vec3(0.0f);
    float m_FOV = 45.0f;
    float m_NearClip = 0.01f;
    float m_FarClip = 100.0f;

    glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
    glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
  };
}