#include "camera.hpp"

// vendor
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace pw {

	// TODO: Optimization: do as little matrix computations on CPU side as possible
	void Camera::update(float width, float height)
	{
		m_Forward = glm::normalize(glm::vec3(
			cos(m_Yaw) * cos(m_Pitch),
			sin(m_Pitch),
			sin(m_Yaw) * cos(m_Pitch)
		));
		
		m_Right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), m_Forward));
		m_Up = glm::normalize(glm::cross(m_Forward, m_Right));

		m_FovRatio = width / height;
		m_ViewMatrix = glm::lookAt(position, position + m_Forward, -m_Up);
		m_ProjectionMatrix = glm::perspective(glm::radians(fov), m_FovRatio, nearClip, farClip);
	}

	std::array<glm::vec3, 8> Camera::getFrustum() const {
		glm::mat4 invProjView = glm::inverse(m_ProjectionMatrix * m_ViewMatrix);

		std::array<glm::vec3, 8> corners{};
		size_t index = 0;

		for (size_t x = 0; x < 2; ++x) {
			for (size_t y = 0; y < 2; ++y) {
				for (size_t z = 0; z < 2; ++z) {
					const glm::vec4 pt = invProjView * glm::vec4(
						2.0f * x - 1.0f,
						2.0f * y - 1.0f,
						z, // In Vulkan z is in the range [0, 1] already
						1.0f);

					corners[index] = (pt / pt.w);
					index++;
				}
			}
		}

		return corners;
	}

}