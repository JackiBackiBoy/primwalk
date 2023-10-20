#pragma once

// primwalk
#include "primwalk/core.hpp"

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

