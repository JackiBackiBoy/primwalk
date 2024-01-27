#pragma once

// primwalk
#include "../../core.hpp"

// std
#include <array>
#include <memory>

// vendor
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace pw {
	class PW_API Camera {
	public:
		Camera() = default;
		~Camera() = default;

		void update(float width, float height);

		inline glm::mat4 getViewMatrix() const { return m_ViewMatrix; }
		inline glm::mat4 getProjectionMatrix() const { return m_ProjectionMatrix; }

		inline float getYaw() const { return m_Yaw; }
		inline float getPitch() const { return m_Pitch; }
		inline glm::vec3 getForward() const { return m_Forward; }
		inline glm::vec3 getRight() const { return m_Right; }
		inline glm::vec3 getUp() const { return m_Up; }

		std::array<glm::vec3, 8> getFrustum() const;

		inline void setYaw(float radians) { m_Yaw = glm::mod(radians, glm::two_pi<float>()); }
		inline void setPitch(float radians) { m_Pitch = glm::clamp(radians, -1.57f, 1.57f); }

		inline static std::shared_ptr<Camera> MainCamera = std::make_shared<Camera>();

		glm::vec3 position = glm::vec3(0.0f);
		float fov = 45.0f; // Vertical FOV
		float nearClip = 0.1f;
		float farClip = 50.0f;

	private:
		float m_Yaw = glm::pi<float>() / 2.0f;
		float m_Pitch = 0.0f;
		float m_FovRatio = 1.0f;

		glm::vec3 m_Forward = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 m_Up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 m_Right = glm::vec3(1.0f, 0.0f, 0.0f);

		glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
		glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
	};
}