#pragma once

// primwalk
#include "../../core.hpp"
#include "../data/model.hpp"
#include "../rendering/frameInfo.hpp"
#include "../rendering/vertex3d.hpp"
#include "../managers/componentManager.hpp"
#include "system.hpp"

// vendor
#include <glm/glm.hpp>

// std
#include <cstdint>
#include <memory>
#include <vector>

#define MAX_LIGHTS 32

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

		void onUpdate(const FrameInfo& frameInfo, ComponentManager& manager);
		void onRender(const FrameInfo& frameInfo, ComponentManager& manager);

		void drawDebugLine(glm::vec3 p1, glm::vec3 p2);
		void drawDebugBox(glm::vec3 pos, glm::vec3 volume);

	private:
		struct DebugLineParams {
			alignas(16) glm::vec3 p1;
			alignas(16) glm::vec3 p2;
			alignas(16) glm::vec3 color;
		};

		struct PointLightParams {
			alignas(16) glm::vec3 position{};
			alignas(16) glm::vec4 color{}; // w component holds intensity
		};

		struct UniformBuffer3D {
			alignas(16) glm::mat4 view{1.0f};
			alignas(16) glm::mat4 proj{1.0f};
			alignas(16) glm::vec3 viewPosition{};
			//alignas(16) glm::vec3 directionLight;
			PointLightParams pointLights[MAX_LIGHTS];
			uint32_t numLights = 0;
		};

		struct ModelPushConstant {
			alignas(16) glm::mat4 modelMatrix{1.0f};
			alignas(16) glm::vec3 color = { 1.0, 1.0, 1.0 };
			alignas(4) uint32_t diffuseTexIndex = 0;
			alignas(4) uint32_t normalMapIndex = 0;
		};

		struct DebugPushConstant {
			alignas(16) glm::mat4 modelMatrix{ 1.0f };
		};

		void createDescriptorPool();
		void createUniformBuffers();
		void createDescriptorSetLayout();
		void createPipelineLayout();
		void createPipelines(VkRenderPass renderPass);

		GraphicsDevice_Vulkan& m_Device;
		std::unique_ptr<GraphicsPipeline> m_MainPipeline;
		std::unique_ptr<GraphicsPipeline> m_DebugPipeline;

		VkPipelineLayout m_MainPipelineLayout = VK_NULL_HANDLE;
		VkPipelineLayout m_DebugPipelineLayout = VK_NULL_HANDLE;

		std::vector<VkDescriptorSetLayout> m_MainDescriptorSetLayouts;
		std::vector<VkDescriptorSetLayout> m_DebugDescriptorSetLayouts;
		std::vector<std::unique_ptr<Buffer>> m_UBOs;
		std::vector<std::unique_ptr<Buffer>> m_DebugSSBOs;
		std::vector<DebugLineParams> m_DebugLineParams;

		std::unique_ptr<DescriptorSetLayout> m_UBOSetLayout{};
		std::unique_ptr<DescriptorSetLayout> m_DebugSSBOSetLayout{};

		std::vector<VkDescriptorSet> m_UniformDescriptorSets;
		std::vector<VkDescriptorSet> m_DebugStorageDescriptorSets;

		std::shared_ptr<Model> m_DefaultModel;
	};
}

