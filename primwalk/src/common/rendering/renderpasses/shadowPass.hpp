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
	// TODO: Right now, the shadow pass only works for ONE directional light, and does not work for point lights at all.
	// A solution to this would be to investigate Doom 2016's "megatexture" technique where one depth image
	// holds multiple shadow maps in varying resolutions.
	class ShadowPass {
	public:
		ShadowPass(GraphicsDevice_Vulkan& device, uint32_t shadowResolution = 1024);
		~ShadowPass();

		void draw(VkCommandBuffer commandBuffer, size_t frameIndex, std::set<entity_id>& entities, ComponentManager& manager);
		void resize(uint32_t width, uint32_t height);

		inline Image* getOutputImage() { return m_DepthImage.get(); }
		inline glm::mat4 getLightSpaceMatrix() const { return m_LightSpaceMatrix; }

	private:
		struct PointLightParams {
			alignas(16) glm::vec3 position{};
			alignas(16) glm::vec4 color{}; // w component holds intensity
		};

		struct DirectionLightParams {
			alignas(16) glm::vec3 direction{};
			alignas(16) glm::vec3 color{};
		};

		struct UBO {
			alignas(16) glm::mat4 view{ 1.0f };
			alignas(16) glm::mat4 proj{ 1.0f };
			alignas(16) glm::vec3 viewPosition{};
			DirectionLightParams directionLight{};
		};

		struct PushConstants {
			alignas(16) glm::mat4 modelMatrix{ 1.0f };
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

		std::unique_ptr<Framebuffer> m_Framebuffer;
		std::unique_ptr<RenderPass> m_RenderPass;
		std::unique_ptr<Image> m_DepthImage;
		std::unique_ptr<GraphicsPipeline> m_Pipeline;
		VkPipelineLayout m_PipelineLayout;

		std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;
		std::unique_ptr<DescriptorPool> m_DescriptorPool;

		std::vector<VkDescriptorSet> m_UBODescriptorSets;
		std::unique_ptr<DescriptorSetLayout> m_UBOSetLayout{};
		std::vector<std::unique_ptr<Buffer>> m_UBOs;

		std::unique_ptr<Sampler> m_Sampler;

		glm::mat4 m_LightSpaceMatrix{ 1.0f };
	};
}