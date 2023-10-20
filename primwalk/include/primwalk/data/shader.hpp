#pragma once

// primwalk
#include "primwalk/core.hpp"

// std
#include <string>
#include <vector>

// vendor
#include <glm/glm.hpp>

namespace pw {
	// ------ Vulkan ------
	class PW_API Shader_Vulkan {
	public:
		Shader_Vulkan() {};
		~Shader_Vulkan() {};

		static std::vector<char> readFile(const std::string& path);
	};
}
