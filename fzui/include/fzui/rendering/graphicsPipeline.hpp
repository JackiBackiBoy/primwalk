#ifndef FZ_GRAPHICS_PIPELINE_HEADER
#define FZ_GRAPHICS_PIPELINE_HEADER

// FZUI
#include "fzui/core.hpp"

// std
#include <array>
#include <vector>
#include <string>

// vendor
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace fz {
  struct FZ_API Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions() {
      std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
      bindingDescriptions[0].binding = 0;
      bindingDescriptions[0].stride = sizeof(Vertex);
      bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      return bindingDescriptions;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
      std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

      attributeDescriptions[0].binding = 0;
      attributeDescriptions[0].location = 0;
      attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
      attributeDescriptions[0].offset = offsetof(Vertex, pos);

      attributeDescriptions[1].binding = 0;
      attributeDescriptions[1].location = 1;
      attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
      attributeDescriptions[1].offset = offsetof(Vertex, color);

      return attributeDescriptions;
    }
  };

  struct FZ_API PipelineConfigInfo {
    PipelineConfigInfo(const PipelineConfigInfo&) = delete;
    PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;
    PipelineConfigInfo() = default;

    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    VkPipelineViewportStateCreateInfo viewportInfo{};
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
    VkPipelineMultisampleStateCreateInfo multisampleInfo{};
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
    std::vector<VkDynamicState> dynamicStateEnables{};
    VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
    VkPipelineLayout pipelineLayout = nullptr;
    VkRenderPass renderPass = nullptr;
    uint32_t subpass = 0;
  };

  class FZ_API GraphicsDevice_Vulkan;

  class FZ_API GraphicsPipeline {
    public:
      GraphicsPipeline(GraphicsDevice_Vulkan& device,
        const std::string& vertPath,
        const std::string& fragPath,
        const PipelineConfigInfo& configInfo);
      ~GraphicsPipeline();

      void bind(VkCommandBuffer commandBuffer);
      static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

    private:
      void createPipeline(const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo);
      VkShaderModule createShaderModule(const std::vector<char>& code);

      GraphicsDevice_Vulkan& m_Device;
      VkPipeline m_GraphicsPipeline = VK_NULL_HANDLE;
  };
}
#endif