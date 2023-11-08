#pragma once

// primwalk
#include "../../core.hpp"

// vendor
#include <glm/vec3.hpp>

namespace pw {
	struct PW_API DirectionLight {
		glm::vec3 direction = { 0.0f, -1.0f, 0.0f };
		glm::vec3 color = { 1.0f, 1.0f, 1.0f };
	};
}