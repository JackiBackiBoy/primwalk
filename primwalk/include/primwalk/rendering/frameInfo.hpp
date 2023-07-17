#ifndef PW_FRAME_INFO_HEADER
#define PW_FRAME_INFO_HEADER

// primwalk
#include "primwalk/core.hpp"

// vendor
#include <vulkan/vulkan.h>

struct PW_API FrameInfo {
  int frameIndex;
  float frameTime;
  float windowWidth;
  float windowHeight;
  VkCommandBuffer commandBuffer;
  // TODO: Add global descriptors
};

#endif