#ifndef FZ_RENDER_PASS_HEADER
#define FZ_RENDER_PASS_HEADER

// FZUI
#include "fzui/core.hpp"

// std
#include <memory>
#include <vector>

// vendor
#include <vulkan/vulkan.h>

namespace fz {
  struct FZ_API RenderPassInfo {
    
  };

  class FZ_API RenderPass {
  public:


  private:
    VkRenderPass m_RenderPass = VK_NULL_HANDLE;
    std::vector<VkClearValue> m_ClearValues;
  };
}
#endif