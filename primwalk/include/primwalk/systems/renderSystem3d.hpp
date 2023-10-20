#pragma once

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/data/model.hpp"
#include "primwalk/rendering/frameInfo.hpp"
#include "primwalk/rendering/vertex3d.hpp"
#include "primwalk/systems/system.hpp"
#include "primwalk/managers/componentManager.hpp"

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
		void onRender(const FrameInfo& frameInfo, ComponentManager& manager);

	private:
		struct UniformBuffer3D {
			alignas(16) glm::mat4 view;
			alignas(16) glm::mat4 proj;
		};

		struct ModelPushConstant {
			alignas(16) glm::mat4 modelMatrix{1.0f};
			alignas(16) glm::vec3 color = { 1.0, 1.0, 1.0 };
			alignas(4) uint32_t diffuseTexIndex = 0;
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

		std::shared_ptr<Model> m_DefaultModel;
	};
}

