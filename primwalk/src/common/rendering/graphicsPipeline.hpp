#pragma once

// primwalk
#include "../../core.hpp"

// std
#include <cstdint>
#include <memory>
#include <vector>
#include <string>

// vendor
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace pw {
	struct PW_API PipelineConfigInfo {
		PipelineConfigInfo() = default;
		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

		std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
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

		GraphicsPipeline(GraphicsDevice_Vulkan& device,
			const std::vector<VkPipelineShaderStageCreateInfo>& shaderStages,
			const PipelineConfigInfo& configInfo);

		~GraphicsPipeline();
		
		// Nested builder class
		class Builder {
		public:
			Builder(GraphicsDevice_Vulkan& device, const PipelineConfigInfo& configInfo);
			~Builder();

			Builder& addStage(VkShaderStageFlagBits stage, const std::string& codePath);
			std::unique_ptr<GraphicsPipeline> build();

		private:
			GraphicsDevice_Vulkan& m_Device;
			const PipelineConfigInfo& m_ConfigInfo;
			std::vector<VkShaderModule> m_ShaderModules;
			std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;
		};

		void bind(VkCommandBuffer commandBuffer);
		static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

	private:
		void createPipeline(const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo);
		VkShaderModule createShaderModule(const std::vector<char>& code);

		GraphicsDevice_Vulkan& m_Device;
		VkPipeline m_GraphicsPipeline = VK_NULL_HANDLE;
	};
}

