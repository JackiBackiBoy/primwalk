#pragma once

// primwalk
#include "../../core.hpp"

// std
#include <cstdint>

namespace pw {
	struct PW_API Mesh {
		uint32_t indices = 0;
		uint32_t materialIndex = 0;
		uint32_t baseVertex = 0;
		uint32_t baseIndex = 0;
	};
}