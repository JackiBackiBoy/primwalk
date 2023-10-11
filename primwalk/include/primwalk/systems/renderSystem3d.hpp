#pragma once

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/rendering/frameInfo.hpp"
#include "primwalk/rendering/vertex3d.hpp"
#include "primwalk/systems/system.hpp"

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

  class PW_API RenderSystem3D : public System {
  public:
    RenderSystem3D(GraphicsDevice_Vulkan& device, VkRenderPass renderPass);
    ~RenderSystem3D();

    void onUpdate(const FrameInfo& frameInfo);
    void onRender(const FrameInfo& frameInfo);

  private:
    struct UniformBuffer3D {
      alignas(16) glm::mat4 view;
      alignas(16) glm::mat4 proj;
    };

    struct ModelPushConstant {
      glm::mat4 modelMatrix{1.0f};
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
  };
}

