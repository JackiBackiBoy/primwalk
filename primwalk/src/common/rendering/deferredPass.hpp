#pragma once

#include "../../core.hpp"
#include "buffer.hpp"
#include "graphicsDevice_Vulkan.hpp"
#include "../components/component.hpp"
#include "graphicsPipeline.hpp"
#include "renderpass.hpp"
#include "sampler.hpp"
#include "texture2D.hpp"

#include "../managers/componentManager.hpp"

// std
#include <memory>
#include <set>

#define MAX_LIGHTS 32

namespace pw {
	class PW_API DeferredPass {
	public:
		DeferredPass(uint32_t width, uint32_t height, GraphicsDevice_Vulkan& device);
		~DeferredPass();

		void draw(VkCommandBuffer commandBuffer, size_t frameIndex, ComponentManager& manager);
		void resize(uint32_t width, uint32_t height);
		inline Image* getPositionBufferImage() { return m_PositionBuffer.get(); }
		inline Image* getNormalBufferImage() { return m_NormalBuffer.get(); }
		inline Image* getDiffuseBufferImage() { return m_DiffuseBuffer.get(); }

		inline Image* getOutputImage() { return m_OffscreenImage.get(); }

		std::set<entity_id> m_Entities;

	private:
		struct PointLightParams {
			alignas(16) glm::vec3 position{};
			alignas(16) glm::vec4 color{}; // w component holds intensity
		};

		struct UniformBuffer3D {
			alignas(16) glm::mat4 view{ 1.0f };
			alignas(16) glm::mat4 proj{ 1.0f };
			alignas(16) glm::vec3 viewPosition{};
			PointLightParams pointLights[MAX_LIGHTS];
			uint32_t numLights = 0;
		};

		struct ModelPushConstant {
			alignas(16) glm::mat4 modelMatrix{ 1.0f };
			alignas(16) glm::vec3 color = { 1.0, 1.0, 1.0 };
			alignas(4) uint32_t diffuseTexIndex = 0;
			alignas(4) uint32_t normalMapIndex = 0;
		};

		void createImages(uint32_t width, uint32_t height);
		void createRenderPasses();
		void createFramebuffers(uint32_t width, uint32_t height);
		void createDescriptorPool();
		void createBuffers();
		void createDescriptorSetLayout();
		void createPipelineLayouts();
		void createPipelines(VkRenderPass renderPass);
		void createSamplers();

		GraphicsDevice_Vulkan& m_Device;
		std::unique_ptr<GraphicsPipeline> m_MainPipeline;
		VkPipelineLayout m_MainPipelineLayout = VK_NULL_HANDLE;
		std::vector<VkDescriptorSetLayout> m_MainDescriptorSetLayouts;

		uint32_t addTexture(Image* image);
		void freeTextureID(Image* image);
		std::unordered_map<Image*, uint32_t> m_TextureIDs{};
		std::set<uint32_t> m_VacantTextureIDs{};

		std::vector<std::unique_ptr<Buffer>> m_UBOs;

		// TODO: Bindless resources might fit better in a dedicated scene class
		std::unique_ptr<DescriptorSetLayout> m_UBOSetLayout{};
		std::unique_ptr<DescriptorSetLayout> m_TextureSetLayout{};

		std::vector<std::shared_ptr<Texture2D>> m_Textures;

		// TODO: Add more renderpasses
		std::unique_ptr<RenderPass> m_OffscreenPass;
		std::unique_ptr<Image> m_OffscreenImage;
		std::unique_ptr<Image> m_OffscreenDepthImage;
		std::unique_ptr<Framebuffer> m_OffscreenFramebuffer;

		// Deferred render passes
		std::unique_ptr<RenderPass> m_GeometryPass;
		std::unique_ptr<RenderPass> m_LightingPass;

		// G-Buffer
		std::unique_ptr<Framebuffer> m_DeferredFramebuffer;
		std::unique_ptr<Image> m_PositionBuffer;
		std::unique_ptr<Image> m_DiffuseBuffer;
		std::unique_ptr<Image> m_NormalBuffer;
		std::unique_ptr<Image> m_SpecularBuffer;
		std::unique_ptr<Image> m_DeferredDepthBuffer;
		std::unique_ptr<GraphicsPipeline> m_GBufferPipeline;

		std::vector<VkDescriptorSet> m_UniformDescriptorSets;
		std::vector<VkDescriptorSet> m_DebugStorageDescriptorSets;
		VkDescriptorSet m_TextureDescriptorSet;
		std::unique_ptr<Sampler> m_Sampler;
	};
}