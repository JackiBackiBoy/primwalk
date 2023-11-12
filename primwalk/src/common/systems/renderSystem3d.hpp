#pragma once

// primwalk
#include "../../core.hpp"
#include "../data/model.hpp"
#include "../rendering/frameInfo.hpp"
#include "../rendering/vertex3d.hpp"
#include "../rendering/sampler.hpp"
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

		struct SkyboxPushConstant {
			alignas(16) glm::mat4 modelMatrix{ 1.0f };
			alignas(4) uint32_t texIndex = 0;
		};

		void createDescriptorPool();
		void createBuffers();
		void createDescriptorSetLayout();
		void createPipelineLayouts();
		void createPipelines(VkRenderPass renderPass);
		void createSamplers();

		uint32_t addTexture(Image* image);
		void freeTextureID(Image* image);
		std::unordered_map<Image*, uint32_t> m_TextureIDs{};
		std::set<uint32_t> m_VacantTextureIDs{};

		GraphicsDevice_Vulkan& m_Device;
		std::unique_ptr<GraphicsPipeline> m_MainPipeline;
		std::unique_ptr<GraphicsPipeline> m_DebugPipeline;
		std::unique_ptr<GraphicsPipeline> m_SkyboxPipeline;

		VkPipelineLayout m_MainPipelineLayout = VK_NULL_HANDLE;
		VkPipelineLayout m_DebugPipelineLayout = VK_NULL_HANDLE;
		VkPipelineLayout m_SkyboxPipelineLayout = VK_NULL_HANDLE;

		std::vector<VkDescriptorSetLayout> m_MainDescriptorSetLayouts;
		std::vector<VkDescriptorSetLayout> m_DebugDescriptorSetLayouts;
		std::vector<VkDescriptorSetLayout> m_SkyboxDescriptorSetLayouts;

		std::vector<std::unique_ptr<Buffer>> m_UBOs;
		std::vector<std::unique_ptr<Buffer>> m_DebugSSBOs;
		std::vector<DebugLineParams> m_DebugLineParams;

		std::unique_ptr<DescriptorSetLayout> m_UBOSetLayout{};
		std::unique_ptr<DescriptorSetLayout> m_DebugSSBOSetLayout{};
		std::unique_ptr<DescriptorSetLayout> m_TextureSetLayout{};

		std::vector<std::shared_ptr<Texture2D>> m_Textures;

		std::vector<VkDescriptorSet> m_UniformDescriptorSets;
		std::vector<VkDescriptorSet> m_DebugStorageDescriptorSets;
		VkDescriptorSet m_TextureDescriptorSet;
		std::unique_ptr<Sampler> m_Sampler;

		std::shared_ptr<Model> m_DefaultModel; // also used as skybox model
	};
}

