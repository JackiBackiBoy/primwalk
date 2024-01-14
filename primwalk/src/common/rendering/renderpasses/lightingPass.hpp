#pragma once

#include "../graphicsDevice_Vulkan.hpp"
#include "../buffer.hpp"
#include "../descriptors.hpp"
#include "../framebuffer.hpp"
#include "../graphicsPipeline.hpp"
#include "../image.hpp"
#include "../renderpass.hpp"
#include "../sampler.hpp"
#include "../../components/component.hpp"
#include "../../managers/componentManager.hpp"


#include <cstdint>
#include <memory>
#include <set>
#include <vector>

#define MAX_LIGHTS 32

namespace pw {
	// The lighting pass acts as a "composition pass" where all the deferred images are "composed" into the final lit image
	class LightingPass {
	public:
		LightingPass(uint32_t width, uint32_t height, GraphicsDevice_Vulkan& device);
		~LightingPass();

		void draw(VkCommandBuffer commandBuffer, size_t frameIndex, std::set<entity_id>& entities, ComponentManager& manager,
			Image* positionBuffer, Image* normalBuffer, Image* albedoBuffer);
		void resize(uint32_t width, uint32_t height);

		inline Image* getOutputImage() { return m_CompositionImage.get(); }

	private:
		struct PointLightParams {
			alignas(16) glm::vec3 position{};
			alignas(16) glm::vec4 color{}; // w component holds intensity
		};

		struct DirectionLightParams {
			alignas(16) glm::vec3 direction{};
			alignas(16) glm::vec3 color{};
		};

		struct UBOComposition {
			alignas(16) glm::vec3 viewPosition{};
			DirectionLightParams directionLight{};
			PointLightParams pointLights[MAX_LIGHTS];
			alignas(4) uint32_t numPointLights = 0;
		};

		void createImages(uint32_t width, uint32_t height);
		void createRenderpass();
		void createFramebuffer(uint32_t width, uint32_t height);
		void createDescriptorPool();
		void createBuffers();
		void createDescriptorSetLayout();
		void createPipeline();
		void createSampler();

		GraphicsDevice_Vulkan& m_Device;

		std::unique_ptr<Framebuffer> m_CompositionFramebuffer;
		std::unique_ptr<RenderPass> m_LightingPass;
		std::unique_ptr<Image> m_CompositionImage;
		std::unique_ptr<GraphicsPipeline> m_CompositionPipeline;
		VkPipelineLayout m_CompositionPipelineLayout;

		std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;
		std::unique_ptr<DescriptorSetLayout> m_GBufferSetLayout;
		std::unique_ptr<DescriptorPool> m_GBufferDescriptorPool;
		VkDescriptorSet m_GBufferDescriptorSet = VK_NULL_HANDLE;

		std::vector<VkDescriptorSet> m_CompositionUBODescriptorSets;
		std::unique_ptr<DescriptorSetLayout> m_CompositionUBOSetLayout{};
		std::vector<std::unique_ptr<Buffer>> m_CompositionUBOs;

		std::unique_ptr<Sampler> m_Sampler;
	};
}