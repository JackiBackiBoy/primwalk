#ifndef FZ_UI_RENDER_SYSTEM_HEADER
#define FZ_UI_RENDER_SYSTEM_HEADER

// FZUI
#include "fzui/core.hpp"
#include "fzui/rendering/graphicsDevice.hpp"
#include "fzui/rendering/graphicsPipeline.hpp"
#include "fzui/rendering/frameInfo.hpp"
#include "fzui/rendering/buffer.hpp"

// std
#include <cstdint>
#include <memory>
#include <vector>

namespace fz {
  struct FZ_API RenderParams {
    glm::vec2 position;
    glm::vec2 size;
    glm::vec4 color;
  };

  class FZ_API UIRenderSystem {
    public:
      UIRenderSystem(GraphicsDevice_Vulkan& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout>& setLayouts);
      ~UIRenderSystem();

      void onUpdate(const FrameInfo& frameInfo);
      void onRender(const FrameInfo& frameInfo);

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
        {{0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
        {{1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
        {{1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
        {{0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}}
      };

      const std::vector<uint16_t> m_Indices = {
        0, 1, 2, 2, 3, 0,
      };

      GraphicsDevice_Vulkan& m_Device;
      std::unique_ptr<GraphicsPipeline> m_Pipeline;
      VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
      std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;
      VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

      std::unique_ptr<Buffer> m_VertexBuffer;
      std::unique_ptr<Buffer> m_IndexBuffer;
      std::vector<std::unique_ptr<Buffer>> m_UniformBuffers;
      std::vector<std::unique_ptr<Buffer>> m_StorageBuffers;

      std::vector<VkDescriptorSet> m_DescriptorSets;

  };
}
#endif