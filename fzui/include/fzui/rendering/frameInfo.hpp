#ifndef FZ_FRAME_INFO_HEADER
#define FZ_FRAME_INFO_HEADER

// FZUI
#include "fzui/core.hpp"

// vendor
#include <vulkan/vulkan.h>

struct FZ_API FrameInfo {
  int frameIndex;
  float frameTime;
  float windowWidth;
  float windowHeight;
  VkCommandBuffer commandBuffer;
  // TODO: Add global descriptors
};

#endif