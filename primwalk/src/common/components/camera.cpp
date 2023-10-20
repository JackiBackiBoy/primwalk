#include "primwalk/components/camera.hpp"

// vendor
#include <glm/gtc/matrix_transform.hpp>

namespace pw {

	void Camera::update()
	{
		m_Forward = glm::normalize(glm::vec3(
			cos(m_Yaw) * cos(m_Pitch),
			-sin(m_Pitch),
			sin(m_Yaw) * cos(m_Pitch)
		));

		m_Right = glm::normalize(glm::cross(m_Forward, glm::vec3(0.0f, 1.0f, 0.0f)));
		m_Up = glm::normalize(glm::cross(m_Right, m_Forward));

		m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Forward, m_Up);
	}

}