#ifndef FZ_UI_RENDER_SYSTEM_HEADER
#define FZ_UI_RENDER_SYSTEM_HEADER

// FZUI
#include "fzui/core.hpp"
#include "fzui/rendering/graphicsDevice.hpp"
#include "fzui/rendering/graphicsPipeline.hpp"

// std
#include <vector>
#include <cstdint>

namespace fz {
  class FZ_API UIRenderSystem {
    public:
      UIRenderSystem(GraphicsDevice_Vulkan& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout>& setLayouts);
      ~UIRenderSystem();

      void onRender(VkCommandBuffer commandBuffer);

    private:
      void createPipelineLayout(std::vector<VkDescriptorSetLayout>& setLayouts);
      void createPipeline(VkRenderPass renderPass);
      void createVertexBuffer();
      void createIndexBuffer();

      const std::vector<Vertex> m_Vertices = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
      };

      const std::vector<uint16_t> m_Indices = {
        0, 1, 2, 2, 3, 0
      };

      GraphicsDevice_Vulkan& m_Device;
      std::unique_ptr<GraphicsPipeline> m_Pipeline;
      VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
      VkBuffer m_VertexBuffer = VK_NULL_HANDLE;
      VkDeviceMemory m_VertexBufferMemory = VK_NULL_HANDLE;
      VkBuffer m_IndexBuffer = VK_NULL_HANDLE;
      VkDeviceMemory m_IndexBufferMemory = VK_NULL_HANDLE;
  };
}
#endif