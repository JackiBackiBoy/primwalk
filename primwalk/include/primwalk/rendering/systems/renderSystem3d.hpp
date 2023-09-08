#pragma once

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/rendering/frameInfo.hpp"
#include "primwalk/rendering/vertex.hpp"

// std
#include <cstdint>
#include <memory>
#include <vector>

namespace pw {
  // Forward declarations
  class GraphicsDevice_Vulkan;
  class GraphicsPipeline;
  class Buffer;
  class DescriptorPool;
  class DescriptorSetLayout;

  class PW_API RenderSystem3D {
  public:
    RenderSystem3D(GraphicsDevice_Vulkan& device, VkRenderPass renderPass);
    ~RenderSystem3D();

    void onUpdate(const FrameInfo& frameInfo);
    void onRender(const FrameInfo& frameInfo);

  private:
    void createDescriptorPool();
    void createUniformBuffers();
    void createDescriptorSetLayout();
    void createPipelineLayout();
    void createPipeline(VkRenderPass renderPass);
    void createVertexBuffer();
    void createIndexBuffer();

    GraphicsDevice_Vulkan& m_Device;

    std::unique_ptr<GraphicsPipeline> m_Pipeline;
    VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

    std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;
    std::unique_ptr<Buffer> m_VertexBuffer;
    std::unique_ptr<Buffer> m_IndexBuffer;

    std::unique_ptr<DescriptorSetLayout> m_UniformSetLayout{};
    std::unique_ptr<DescriptorPool> m_DescriptorPool{};
    std::vector<std::unique_ptr<Buffer>> m_UniformBuffers;
    std::vector<VkDescriptorSet> m_UniformDescriptorSets;

    const std::vector<Vertex> m_Vertices = {
      {{-0.5f, -0.5f}, { 0.0f, 0.0f }, 0 },
      {{0.5f, -0.5f}, { 1.0f, 0.0f }, 1 },
      {{0.5f, 0.5f}, { 1.0f, 1.0f }, 2 },
      {{-0.5f, 0.5f}, { 0.0f, 1.0f }, 3 }
    };

    const std::vector<uint16_t> m_Indices = {
      0, 1, 2, 2, 3, 0,
    };
  };
}