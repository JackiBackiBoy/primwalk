#pragma once

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/rendering/frameInfo.hpp"

// vendor
#include <glm/glm.hpp>

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
    struct Vertex3D {
      alignas(16) glm::vec3 pos;
      alignas(8) glm::vec2 texCoord;

      static std::vector<VkVertexInputBindingDescription> getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex3D);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescriptions;
      }

      static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex3D, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex3D, texCoord);

        return attributeDescriptions;
      }
    };

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
    std::vector<std::unique_ptr<Buffer>> m_UniformBuffers;
    std::vector<VkDescriptorSet> m_UniformDescriptorSets;

    const std::vector<Vertex3D> m_Vertices = {
      { {-0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f } },
      { {0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f } },
      { {0.5f, 0.5f, 0.0f }, { 1.0f, 1.0f } },
      { {-0.5f, 0.5f, 0.0f }, { 0.0f, 1.0f } }
    };

    const std::vector<uint16_t> m_Indices = {
      0, 1, 2, 2, 3, 0,
    };
  };
}

