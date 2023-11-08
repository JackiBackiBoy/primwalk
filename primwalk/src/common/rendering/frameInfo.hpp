#pragma once

// primwalk
#include "../../core.hpp"

// vendor
#include <vulkan/vulkan.h>

namespace pw {
	struct PW_API FrameInfo {
		int frameIndex;
		float frameTime;
		int windowWidth;
		int windowHeight;
		VkCommandBuffer commandBuffer;
	};
}

