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
  struct FZ_API RenderParams {
    glm::vec2 position;
  };

  class FZ_API UIRenderSystem {
    public:
      UIRenderSystem(GraphicsDevice_Vulkan& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout>& setLayouts);
      ~UIRenderSystem();

      void onUpdate(VkCommandBuffer commandBuffer, size_t currentImage);
      void onRender(VkCommandBuffer commandBuffer, size_t currentFrame);

    private:
      void createDescriptorSetLayout();
      void createPipelineLayout(std::vector<VkDescriptorSetLayout>& setLayouts);
      void createPipeline(VkRenderPass renderPass);
      void createVertexBuffer();
      void createIndexBuffer();
      void createUniformBuffers();
      void createDescriptorPool();
      void createDescriptorSets();

      const std::vector<Vertex> m_Vertices = {
        {{0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{100.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{100.0f, 100.0f}, {0.0f, 0.0f, 1.0f}},
        {{0.0f, 100.0f}, {1.0f, 1.0f, 1.0f}}
      };

      const std::vector<uint16_t> m_Indices = {
        0, 1, 2, 2, 3, 0,
      };

      GraphicsDevice_Vulkan& m_Device;
      std::unique_ptr<GraphicsPipeline> m_Pipeline;
      VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
      std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;
      VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
      VkBuffer m_VertexBuffer = VK_NULL_HANDLE;
      VkDeviceMemory m_VertexBufferMemory = VK_NULL_HANDLE;
      VkBuffer m_IndexBuffer = VK_NULL_HANDLE;
      VkDeviceMemory m_IndexBufferMemory = VK_NULL_HANDLE;

      std::vector<VkBuffer> m_UniformBuffers;
      std::vector<VkDeviceMemory> m_UniformBuffersMemory;
      std::vector<void*> m_UniformBuffersMapped;
      std::vector<VkBuffer> m_StorageBuffers;
      std::vector<VkDeviceMemory> m_StorageBuffersMemory;
      std::vector<void*> m_StorageBuffersMapped;
      std::vector<VkDescriptorSet> m_DescriptorSets;

  };
}
#endif