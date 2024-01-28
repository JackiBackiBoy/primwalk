#include "shadowPass.hpp"
#include "../vertex3d.hpp"
#include "../../components/camera.hpp"
#include "../../components/directionLight.hpp"
#include "../../components/renderable.hpp"
#include "../../components/transform.hpp"
#include "../../data/model.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <limits>
#include <stdexcept>

namespace pw {

	ShadowPass::ShadowPass(GraphicsDevice_Vulkan& device, uint32_t shadowResolution) : m_Device(device) {
		createImages(shadowResolution, shadowResolution);
		createRenderpass();
		createFramebuffer(shadowResolution, shadowResolution);
		createDescriptorPool();
		createBuffers();
		createDescriptorSetLayout();
		createPipeline();
		createSampler();
	}

	ShadowPass::~ShadowPass() {
		vkDestroyPipelineLayout(m_Device.getDevice(), m_PipelineLayout, nullptr);

		m_Framebuffer->destroy();
		m_DepthImage->destroy();
	}

	void ShadowPass::draw(VkCommandBuffer commandBuffer, size_t frameIndex, std::set<entity_id>& entities, ComponentManager& manager) {
		UBO ubo{};

		for (const auto& e : entities) {
			if (manager.hasComponent<DirectionLight>(e)) {
				auto& light = manager.getComponent<DirectionLight>(e);
				ubo.directionLight.color = light.color;
				ubo.directionLight.direction = glm::normalize(light.direction);
			}
		}

		auto& camera = Camera::MainCamera;

		// Calculate suitable positioning of projection based on view frustum
		std::array<glm::vec4, 8> frustum = camera->getFrustum();

		glm::vec3 frustumCentroid = (frustum[0] + frustum[1] + frustum[2] + frustum[3] +
									frustum[4] + frustum[5] + frustum[6] + frustum[7]) / 8.0f;



		// Calculate matrices from light's perspective
		ubo.view = glm::lookAt(frustumCentroid + ubo.directionLight.direction, frustumCentroid, glm::vec3(0.0f, -1.0f, 0.0f));

		// Find frustum corners in light-space and calculate min/max of x, y, z
		glm::vec3 minimums = glm::vec3(std::numeric_limits<float>::max());
		glm::vec3 maximums = glm::vec3(std::numeric_limits<float>::lowest());

		for (const auto& v : frustum) {
			glm::vec4 trf = ubo.view * v;
			minimums.x = std::min(minimums.x, trf.x);
			minimums.y = std::min(minimums.y, trf.y);
			minimums.z = std::min(minimums.z, trf.z);

			maximums.x = std::max(maximums.x, trf.x);
			maximums.y = std::max(maximums.y, trf.y);
			maximums.z = std::max(maximums.z, trf.z);
		}

		// Tune this parameter according to the scene
		const float zMult = 1.0f;
		if (minimums.z < 0) {
			minimums.z *= zMult;
		}
		else {
			minimums.z /= zMult;
		}
		if (maximums.z < 0) {
			maximums.z /= zMult;
		}
		else {
			maximums.z *= zMult;
		}

		ubo.proj = glm::ortho(minimums.x, maximums.x, minimums.y, maximums.y, -maximums.z, -minimums.z);

		m_LightSpaceMatrix = ubo.proj * ubo.view;

		m_UBOs[frameIndex]->writeToBuffer(&ubo);

		Viewport viewport{};
		viewport.width = m_Framebuffer->getWidth();
		viewport.height = m_Framebuffer->getHeight();
		
		m_RenderPass->begin(*m_Framebuffer, commandBuffer, viewport);
		m_Pipeline->bind(commandBuffer);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_PipelineLayout, 0, 1, &m_UBODescriptorSets[frameIndex], 0, nullptr);

		for (const auto& e : entities) {
			auto& component = manager.getComponent<Renderable>(e);
			Model* model = component.model;

			if (!model) {
				continue;
			}

			model->bind(commandBuffer);

			for (const auto& mesh : model->getMeshes()) {
				PushConstants push{};
				push.modelMatrix = glm::translate(push.modelMatrix, manager.getComponent<Transform>(e).position);
				push.modelMatrix = glm::scale(push.modelMatrix, manager.getComponent<Transform>(e).scale);

				vkCmdPushConstants(
					commandBuffer,
					m_PipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT,
					0,
					sizeof(PushConstants),
					&push);

				vkCmdDrawIndexed(commandBuffer, mesh.indices, 1, mesh.baseIndex, mesh.baseVertex, 0);
			}
		}

		m_RenderPass->end(commandBuffer);
	}

	void ShadowPass::resize(uint32_t width, uint32_t height) {

	}

	void ShadowPass::createImages(uint32_t width, uint32_t height) {
		ImageInfo imageInfo{};
		imageInfo.width = width;
		imageInfo.height = height;
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.format = m_Device.getSupportedDepthFormat();

		m_DepthImage = std::make_unique<Image>(imageInfo);
	}

	void ShadowPass::createRenderpass() {
		SubpassInfo subpass{};
		subpass.renderTargets = { 0 };

		// Depth attachment
		RenderPassAttachment depthAttachment = {
			m_DepthImage,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE, // TODO: Investigate why this is needed
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};

		RenderPassInfo passInfo = {
			{ depthAttachment },
			{ subpass }
		};

		m_RenderPass = std::make_unique<RenderPass>(passInfo);
	}

	void ShadowPass::createFramebuffer(uint32_t width, uint32_t height) {
		FramebufferInfo framebufferInfo = {
			width,
			height,
			m_RenderPass->getVulkanRenderPass(),
			{ { m_DepthImage }}
		};

		m_Framebuffer = std::make_unique<Framebuffer>(framebufferInfo);
	}

	void ShadowPass::createDescriptorPool() {
		m_UBODescriptorSets.resize(GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT);

		m_DescriptorPool = DescriptorPool::Builder(m_Device)
			.setMaxSets(4)
			.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT) // UBO
			.build();
	}

	void ShadowPass::createBuffers() {
		m_UBOs.resize(GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT);

		for (auto& ubo : m_UBOs) {
			ubo = std::make_unique<Buffer>(
				m_Device,
				sizeof(UBO),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			ubo->map();
		}
	}

	void ShadowPass::createDescriptorSetLayout() {
		m_UBOSetLayout = DescriptorSetLayout::Builder(m_Device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.build();

		for (size_t i = 0; i < m_UBODescriptorSets.size(); i++) {
			auto bufferInfo = m_UBOs[i]->getDescriptorInfo();

			DescriptorWriter(*m_UBOSetLayout, *m_DescriptorPool)
				.writeBuffer(0, &bufferInfo)
				.build(m_UBODescriptorSets[i]);
		}

		m_DescriptorSetLayouts = {
			m_UBOSetLayout->getDescriptorSetLayout()
		};
	}

	void ShadowPass::createPipeline() {
		// Pipeline layout
		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.setLayoutCount = static_cast<uint32_t>(m_DescriptorSetLayouts.size());
		layoutInfo.pSetLayouts = m_DescriptorSetLayouts.data();

		// Push constants
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PushConstants);

		layoutInfo.pushConstantRangeCount = 1;
		layoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(m_Device.getDevice(), &layoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("VULKAN ERROR: Failed to create deferred pipeline layout!");
		}

		// Pipeline
		PipelineConfigInfo configInfo{};
		GraphicsPipeline::defaultPipelineConfigInfo(configInfo);

		// Binding descriptions
		configInfo.bindingDescriptions = Vertex3D::getBindingDescriptions();
		configInfo.attributeDescriptions = Vertex3D::getAttributeDescriptions();
		configInfo.renderPass = m_RenderPass->getVulkanRenderPass();
		configInfo.pipelineLayout = m_PipelineLayout;

		// TRICK: Use front face culling when rendering the shadow map to eliminate
		// peter panning effect.
		configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_FRONT_BIT;

		m_Pipeline = GraphicsPipeline::Builder(m_Device, configInfo)
			.addStage(VK_SHADER_STAGE_VERTEX_BIT, "assets/shaders/shadowMapping.vert.spv")
			.build();
	}

	void ShadowPass::createSampler() {
		SamplerCreateInfo samplerInfo{};
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

		m_Sampler = std::make_unique<Sampler>(samplerInfo, m_Device);
	}

}