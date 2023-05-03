#ifndef FZ_GRAPHICS_PIPELINE_HEADER
#define FZ_GRAPHICS_PIPELINE_HEADER

// FZUI
#include "fzui/core.hpp"

// std
#include <vector>

// vendor
#include <vulkan/vulkan.h>

namespace fz {
  class FZ_API GraphicsDevice_Vulkan;

  class FZ_API GraphicsPipeline {
    public:
      GraphicsPipeline(GraphicsDevice_Vulkan& device);
      ~GraphicsPipeline();

      void bind(VkCommandBuffer commandBuffer);

    private:
      void createPipeline();
      VkShaderModule createShaderModule(const std::vector<char>& code);

      GraphicsDevice_Vulkan& m_Device;
      VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
      VkPipeline m_GraphicsPipeline = VK_NULL_HANDLE;
  };
}
#endif