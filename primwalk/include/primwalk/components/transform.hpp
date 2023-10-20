#pragma once

// primwalk
#include "primwalk/core.hpp"

// vendor
#include <glm/glm.hpp>

namespace pw {
	struct PW_API Transform {
		glm::vec3 position = { 0, 0, 0 };
		glm::vec3 scale = { 1, 1, 1 };
	};
}