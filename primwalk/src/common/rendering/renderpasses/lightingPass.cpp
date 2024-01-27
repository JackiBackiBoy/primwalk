#include "lightingPass.hpp"
#include "../../components/camera.hpp"
#include "../../components/directionLight.hpp"
#include "../../components/pointLight.hpp"
#include "../../components/transform.hpp"

#include <stdexcept>

namespace pw {

	LightingPass::LightingPass(uint32_t width, uint32_t height, GraphicsDevice_Vulkan& device) : m_Device(device) {
		createImages(width, height);
		createRenderpass();
		createFramebuffer(width, height);
		createDescriptorPool();
		createBuffers();
		createDescriptorSetLayout();
		createPipeline();
		createSampler();
	}

	LightingPass::~LightingPass() {
		vkDestroyPipelineLayout(m_Device.getDevice(), m_CompositionPipelineLayout, nullptr);

		m_CompositionFramebuffer->destroy();
		m_CompositionImage->destroy();
	}

	void LightingPass::draw(VkCommandBuffer commandBuffer, size_t frameIndex, std::set<entity_id>& entities, ComponentManager& manager,
		Image* positionBuffer, Image* normalBuffer, Image* albedoBuffer, Image* shadowMap, const glm::mat4& lightSpaceMatrix) {

		UBOComposition ubo{};
		ubo.viewPosition = Camera::MainCamera->position;

		uint32_t lightIndex = 0;
		for (const auto& e : entities) {
			if (manager.hasComponent<PointLight>(e)) {
				auto& light = manager.getComponent<PointLight>(e);
				ubo.pointLights[lightIndex].position = manager.getComponent<Transform>(e).position;
				ubo.pointLights[lightIndex].color = light.color;

				ubo.pointLights[lightIndex].position = ubo.pointLights[lightIndex].position;
				ubo.pointLights[lightIndex].color = ubo.pointLights[lightIndex].color;

				lightIndex++;
			}

			if (manager.hasComponent<DirectionLight>(e)) {
				auto& light = manager.getComponent<DirectionLight>(e);
				ubo.directionLight.color = light.color;
				ubo.directionLight.direction = light.direction;
			}
		}

		ubo.numPointLights = lightIndex;
		m_CompositionUBOs[frameIndex]->writeToBuffer(&ubo);

		Viewport viewport{};
		viewport.width = m_CompositionFramebuffer->getWidth();
		viewport.height = m_CompositionFramebuffer->getHeight();

		m_LightingPass->begin(*m_CompositionFramebuffer, commandBuffer, viewport);
		m_CompositionPipeline->bind(commandBuffer);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_CompositionPipelineLayout, 0, 1, &m_GBufferDescriptorSet, 0, nullptr);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_CompositionPipelineLayout, 1, 1, &m_CompositionUBODescriptorSets[frameIndex], 0, nullptr);

		VkDescriptorImageInfo positionImageInfo{};
		positionImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		positionImageInfo.imageView = positionBuffer->getVulkanImageView();
		positionImageInfo.sampler = m_Sampler->getVkSampler();

		VkDescriptorImageInfo normalImageInfo{};
		normalImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		normalImageInfo.imageView = normalBuffer->getVulkanImageView();
		normalImageInfo.sampler = m_Sampler->getVkSampler();

		VkDescriptorImageInfo albedoImageInfo{};
		albedoImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		albedoImageInfo.imageView = albedoBuffer->getVulkanImageView();
		albedoImageInfo.sampler = m_Sampler->getVkSampler();

		VkDescriptorImageInfo shadowMapInfo{};
		shadowMapInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		shadowMapInfo.imageView = shadowMap->getVulkanImageView();
		shadowMapInfo.sampler = m_ShadowSampler->getVkSampler();

		DescriptorWriter(*m_GBufferSetLayout, *m_GBufferDescriptorPool)
			.writeImage(0, &positionImageInfo)
			.writeImage(1, &normalImageInfo)
			.writeImage(2, &albedoImageInfo)
			// TODO: Specular buffer
			.writeImage(4, &shadowMapInfo)
			.overwrite(m_GBufferDescriptorSet);

		// Push constants
		PushConstant push{};
		push.lightSpaceMatrix = lightSpaceMatrix;

		vkCmdPushConstants(
			commandBuffer,
			m_CompositionPipelineLayout,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(PushConstant),
			&push);

		vkCmdDraw(commandBuffer, 3, 1, 0, 0);

		m_LightingPass->end(commandBuffer);
	}

	void LightingPass::resize(uint32_t width, uint32_t height) {
		m_Device.waitForGPU();

		m_CompositionFramebuffer->destroy();
		m_CompositionImage->destroy();

		createImages(width, height);
		createFramebuffer(width, height);
	}

	void LightingPass::createImages(uint32_t width, uint32_t height) {
		ImageInfo imageInfo{};
		imageInfo.width = width;
		imageInfo.height = height;
		imageInfo.depth = 1;
		imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;

		m_CompositionImage = std::make_unique<Image>(imageInfo);
	}

	void LightingPass::createRenderpass() {
		SubpassInfo subpass{};
		subpass.renderTargets = { 0 };

		RenderPassAttachment colorAttachment = {
					m_CompositionImage,
					VK_ATTACHMENT_LOAD_OP_CLEAR,
					VK_ATTACHMENT_STORE_OP_STORE,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		std::vector<RenderPassAttachment> compositionAttachments = {
			colorAttachment
		};

		RenderPassInfo lightingPassInfo = {
			compositionAttachments,
			{ subpass }
		};

		m_LightingPass = std::make_unique<RenderPass>(lightingPassInfo);
	}

	void LightingPass::createFramebuffer(uint32_t width, uint32_t height) {
		FramebufferInfo framebufferInfo = {
			width,
			height,
			m_LightingPass->getVulkanRenderPass(),
			{ { m_CompositionImage }}
		};

		m_CompositionFramebuffer = std::make_unique<Framebuffer>(framebufferInfo);
	}

	void LightingPass::createDescriptorPool() {
		m_CompositionUBODescriptorSets.resize(GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT);

		m_GBufferDescriptorPool = DescriptorPool::Builder(m_Device)
			.setMaxSets(4)
			.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT) // UBO
			.build();
	}

	void LightingPass::createBuffers() {
		m_CompositionUBOs.resize(GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT);

		for (auto& ubo : m_CompositionUBOs) {
			ubo = std::make_unique<Buffer>(
				m_Device,
				sizeof(UBOComposition),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			ubo->map();
		}
	}

	void LightingPass::createDescriptorSetLayout() {
		m_CompositionUBOSetLayout = DescriptorSetLayout::Builder(m_Device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		for (size_t i = 0; i < m_CompositionUBODescriptorSets.size(); i++) {
			auto bufferInfo = m_CompositionUBOs[i]->getDescriptorInfo();

			DescriptorWriter(*m_CompositionUBOSetLayout, *m_GBufferDescriptorPool)
				.writeBuffer(0, &bufferInfo)
				.build(m_CompositionUBODescriptorSets[i]);
		}

		// Composition
		m_GBufferSetLayout = DescriptorSetLayout::Builder(m_Device)
			.setLayoutFlags(VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT) // position buffer
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT) // normal buffer
			.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT) // albedo buffer
			.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT) // specular buffer
			.addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT) // shadow map
			.build();

		DescriptorWriter(*m_GBufferSetLayout, *m_GBufferDescriptorPool).build(m_GBufferDescriptorSet);

		m_DescriptorSetLayouts = {
			m_GBufferSetLayout->getDescriptorSetLayout(),
			m_CompositionUBOSetLayout->getDescriptorSetLayout()
		};
	}

	void LightingPass::createPipeline() {
		// Push constants
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PushConstant);

		// Pipeline layout
		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.setLayoutCount = static_cast<uint32_t>(m_DescriptorSetLayouts.size());
		layoutInfo.pSetLayouts = m_DescriptorSetLayouts.data();
		layoutInfo.pushConstantRangeCount = 1;
		layoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(m_Device.getDevice(), &layoutInfo, nullptr, &m_CompositionPipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("VULKAN ERROR: Failed to create deferred pipeline layout!");
		}

		// Pipeline
		PipelineConfigInfo configInfo{};
		GraphicsPipeline::defaultPipelineConfigInfo(configInfo);
		configInfo.bindingDescriptions = {};
		configInfo.attributeDescriptions = {};
		configInfo.renderPass = m_LightingPass->getVulkanRenderPass();
		configInfo.pipelineLayout = m_CompositionPipelineLayout;

		m_CompositionPipeline = std::make_unique<GraphicsPipeline>(
			m_Device,
			"assets/shaders/deferred.vert.spv",
			"assets/shaders/deferred.frag.spv",
			configInfo
		);
	}

	void LightingPass::createSampler() {
		SamplerCreateInfo samplerInfo{};

		SamplerCreateInfo shadowSamplerInfo{};
		shadowSamplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		shadowSamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		shadowSamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		shadowSamplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

		m_Sampler = std::make_unique<Sampler>(samplerInfo, m_Device);
		m_ShadowSampler = std::make_unique<Sampler>(shadowSamplerInfo, m_Device);
	}

}