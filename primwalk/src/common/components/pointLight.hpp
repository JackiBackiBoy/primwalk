#pragma once

// primwalk
#include "../../core.hpp"

// vendor
#include <glm/glm.hpp>

namespace pw {
	struct PW_API PointLight {
		glm::vec4 color = { 1.0f, 1.0f, 1.0f, 0.3f };
	};
}