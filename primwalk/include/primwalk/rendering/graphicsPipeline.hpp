#pragma once

// primwalk
#include "primwalk/core.hpp"

// std
#include <cstdint>
#include <vector>
#include <string>

// vendor
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace pw {
	struct PW_API PipelineConfigInfo {
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

	  // Forward declarations
	  class GraphicsDevice_Vulkan;

	  class PW_API GraphicsPipeline {
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

