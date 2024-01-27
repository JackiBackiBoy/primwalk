#include "gBufferPass.hpp"

#include "../../components/camera.hpp"
#include "../vertex3d.hpp"
#include <stdexcept>

#include <glm/gtc/matrix_transform.hpp>
#include "../../components/directionLight.hpp"
#include "../../components/pointLight.hpp"
#include "../../components/renderable.hpp"
#include "../../components/transform.hpp"

namespace pw {

	GBufferPass::GBufferPass(uint32_t width, uint32_t height, GraphicsDevice_Vulkan& device) : m_Device(device) {
		createImages(width, height);
		createRenderPasses();
		createFramebuffers(width, height);

		createDescriptorPool();
		createBuffers();
		createDescriptorSetLayout();
		createPipelineLayouts();
		createPipelines();
		createSamplers();

		// Textures
		m_Textures.push_back(std::make_shared<Texture2D>(1, 1, std::vector<uint8_t>(4, 255).data())); // default 1x1 white texture
		addTexture(m_Textures[0]->getImage());

		//// TODO: Move cubemap code to separate class (HDRI should also be separate)
		//m_Textures.push_back(std::make_shared<Texture2D>("assets/textures/cubemaps/skybox_default.hdr", 4, VK_FORMAT_R16G16B16A16_SFLOAT));
		//addTexture(m_Textures[1]->getImage());
	}

	GBufferPass::~GBufferPass() {
		vkDestroyPipelineLayout(m_Device.getDevice(), m_GBufferPipelineLayout, nullptr);

		m_DeferredFramebuffer->destroy();
		m_PositionBuffer->destroy();
		m_NormalBuffer->destroy();
		m_AlbedoBuffer->destroy();
		m_SpecularBuffer->destroy();
		m_DeferredDepthBuffer->destroy();
	}

	void GBufferPass::draw(VkCommandBuffer commandBuffer, size_t frameIndex, ComponentManager& manager) {
		UniformBuffer3D ubo{};
		ubo.view = Camera::MainCamera->getViewMatrix();
		ubo.proj = Camera::MainCamera->getProjectionMatrix();
		m_UBOs[frameIndex]->writeToBuffer(&ubo);

		Viewport viewport{};
		viewport.width = m_DeferredFramebuffer->getWidth();
		viewport.height = m_DeferredFramebuffer->getHeight();

		// Geometry pass
		m_GeometryPass->begin(*m_DeferredFramebuffer, commandBuffer, viewport);

			m_GBufferPipeline->bind(commandBuffer);

			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				m_GBufferPipelineLayout, 0, 1, &m_UniformDescriptorSets[frameIndex], 0, nullptr);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				m_GBufferPipelineLayout, 1, 1, &m_TextureDescriptorSet, 0, nullptr);

			for (const auto& e : m_Entities) {
				auto& component = manager.getComponent<Renderable>(e);
				Model* model = component.model;

				if (!model) { // render default cube model
					//drawDebugBox(manager.getComponent<Transform>(e).position, glm::vec3(1.0f));
					continue;
				}

				model->bind(commandBuffer);

				for (const auto& mesh : model->getMeshes()) {
					ModelPushConstant push{};
					push.modelMatrix = glm::translate(push.modelMatrix, manager.getComponent<Transform>(e).position);
					push.modelMatrix = glm::scale(push.modelMatrix, manager.getComponent<Transform>(e).scale);

					std::shared_ptr<Texture2D> diffuseMap = model->getDiffuseMap(mesh.materialIndex);
					std::shared_ptr<Texture2D> normalMap = model->getNormalMap(mesh.materialIndex);

					glm::vec4 normColor = Color::normalize(component.color);
					push.color = { normColor.r, normColor.g, normColor.b };

					if (diffuseMap) {
						push.diffuseTexIndex = addTexture(diffuseMap->getImage());
					}

					if (normalMap) {
						push.normalMapIndex = addTexture(normalMap->getImage());
					}

					vkCmdPushConstants(
						commandBuffer,
						m_GBufferPipelineLayout,
						VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
						0,
						sizeof(ModelPushConstant),
						&push);

					vkCmdDrawIndexed(commandBuffer, mesh.indices, 1, mesh.baseIndex, mesh.baseVertex, 0);
				}
			}

		m_GeometryPass->end(commandBuffer);
	}

	void GBufferPass::resize(uint32_t width, uint32_t height) {
		m_Device.waitForGPU();

		m_DeferredFramebuffer->destroy();
		m_PositionBuffer->destroy();
		m_NormalBuffer->destroy();
		m_AlbedoBuffer->destroy();
		m_SpecularBuffer->destroy();
		m_DeferredDepthBuffer->destroy();

		//m_ComposedFramebuffer->destroy();
		//m_ComposedImage->destroy();

		createImages(width, height);
		createFramebuffers(width, height);
	}

	void GBufferPass::createImages(uint32_t width, uint32_t height) {
		// ------ G-Buffer Images ------
		// Position
		ImageInfo positionImageInfo{};
		positionImageInfo.width = width;
		positionImageInfo.height = height;
		positionImageInfo.depth = 1;
		positionImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		positionImageInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;

		// Normals
		ImageInfo normalImageInfo{};
		normalImageInfo.width = width;
		normalImageInfo.height = height;
		normalImageInfo.depth = 1;
		normalImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		normalImageInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;

		// Albedo
		ImageInfo albedoImageInfo{};
		albedoImageInfo.width = width;
		albedoImageInfo.height = height;
		albedoImageInfo.depth = 1;
		albedoImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		albedoImageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;

		// Specular
		ImageInfo specularImageInfo{};
		specularImageInfo.width = width;
		specularImageInfo.height = height;
		specularImageInfo.depth = 1;
		specularImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		specularImageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;

		// Depth
		ImageInfo depthImageInfo{};
		depthImageInfo.width = width;
		depthImageInfo.height = height;
		depthImageInfo.depth = 1;
		depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		depthImageInfo.format = m_Device.getSupportedDepthFormat();

		m_PositionBuffer = std::make_unique<Image>(positionImageInfo);
		m_NormalBuffer = std::make_unique<Image>(normalImageInfo);
		m_AlbedoBuffer = std::make_unique<Image>(albedoImageInfo);
		m_SpecularBuffer = std::make_unique<Image>(specularImageInfo);
		m_DeferredDepthBuffer = std::make_unique<Image>(depthImageInfo);
	}

	void GBufferPass::createRenderPasses() {
		// No subpasses on desktop
		SubpassInfo subpass{};
		subpass.renderTargets = { 0 };

		// ------ G-Buffer Render Pass ------
		RenderPassAttachment positionAttachment = {
			m_PositionBuffer,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		RenderPassAttachment normalAttachment = {
			m_NormalBuffer,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		RenderPassAttachment albedoAttachment = {
			m_AlbedoBuffer,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		RenderPassAttachment specularAttachment = {
			m_SpecularBuffer,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		RenderPassAttachment deferredDepthAttachment = {
			m_DeferredDepthBuffer,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};

		std::vector<RenderPassAttachment> deferredAttachments = {
			positionAttachment,
			normalAttachment,
			albedoAttachment,
			specularAttachment,
			deferredDepthAttachment
		};

		RenderPassInfo geometryPassInfo = {
			deferredAttachments,
			{ subpass }
		};

		m_GeometryPass = std::make_unique<RenderPass>(geometryPassInfo);
	}

	void GBufferPass::createFramebuffers(uint32_t width, uint32_t height) {
		// Deferred framebuffer
		FramebufferInfo deferredInfo = {
			width,
			height,
			m_GeometryPass->getVulkanRenderPass(),
			{
				{
					m_PositionBuffer,
					m_NormalBuffer,
					m_AlbedoBuffer,
					m_SpecularBuffer,
					m_DeferredDepthBuffer
				}
			}
		};

		m_DeferredFramebuffer = std::make_unique<Framebuffer>(deferredInfo);
	}

	void GBufferPass::createDescriptorPool() {
		m_UniformDescriptorSets.resize(GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT);
	}

	void GBufferPass::createBuffers() {
		m_UBOs.resize(GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT);

		// Main UBOs
		for (auto& ubo : m_UBOs) {
			ubo = std::make_unique<Buffer>(
				m_Device,
				sizeof(UniformBuffer3D),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			ubo->map();
		}
	}

	void GBufferPass::createDescriptorSetLayout() {
		// Descriptor set layouts
		m_UBOSetLayout = DescriptorSetLayout::Builder(m_Device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		m_TextureSetLayout = DescriptorSetLayout::Builder(m_Device)
			.setLayoutFlags(VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1024,
				VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT)
			.build();

		// Main UBO
		for (size_t i = 0; i < m_UniformDescriptorSets.size(); i++) {
			auto bufferInfo = m_UBOs[i]->getDescriptorInfo();

			DescriptorWriter(*m_UBOSetLayout, m_Device.getBindlessPool())
				.writeBuffer(0, &bufferInfo)
				.build(m_UniformDescriptorSets[i]);
		}

		DescriptorWriter(*m_TextureSetLayout, m_Device.getBindlessPool())
			.build(m_TextureDescriptorSet);

		m_MainDescriptorSetLayouts = {
			m_UBOSetLayout->getDescriptorSetLayout(),
			m_TextureSetLayout->getDescriptorSetLayout()
		};
	}

	void GBufferPass::createPipelineLayouts() {
		// Main pipeline layout
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(ModelPushConstant);

		VkPipelineLayoutCreateInfo basePipelineLayoutInfo{};
		basePipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		basePipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(m_MainDescriptorSetLayouts.size());
		basePipelineLayoutInfo.pSetLayouts = m_MainDescriptorSetLayouts.data();
		basePipelineLayoutInfo.pushConstantRangeCount = 1;
		basePipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(m_Device.getDevice(), &basePipelineLayoutInfo, nullptr, &m_GBufferPipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("VULKAN ERROR: Failed to create pipeline layout!");
		}
	}

	void GBufferPass::createPipelines() {
		// G-Buffer pipeline
		PipelineConfigInfo gBufferPipelineConfig{};
		GraphicsPipeline::defaultPipelineConfigInfo(gBufferPipelineConfig);

		gBufferPipelineConfig.bindingDescriptions = Vertex3D::getBindingDescriptions();
		gBufferPipelineConfig.attributeDescriptions = Vertex3D::getAttributeDescriptions();
		gBufferPipelineConfig.renderPass = m_GeometryPass->getVulkanRenderPass();
		gBufferPipelineConfig.pipelineLayout = m_GBufferPipelineLayout;

		std::vector<VkPipelineColorBlendAttachmentState> blendStates(4);
		blendStates[0] = gBufferPipelineConfig.colorBlendAttachment;
		blendStates[1] = gBufferPipelineConfig.colorBlendAttachment;
		blendStates[2] = gBufferPipelineConfig.colorBlendAttachment;
		blendStates[3] = gBufferPipelineConfig.colorBlendAttachment;

		gBufferPipelineConfig.colorBlendInfo.attachmentCount = 4;
		gBufferPipelineConfig.colorBlendInfo.pAttachments = blendStates.data();

		m_GBufferPipeline = std::make_unique<GraphicsPipeline>(
			m_Device,
			"assets/shaders/gbuffer.vert.spv",
			"assets/shaders/gbuffer.frag.spv",
			gBufferPipelineConfig);
	}

	void GBufferPass::createSamplers() {
		SamplerCreateInfo samplerInfo{};
		m_Sampler = std::make_unique<Sampler>(samplerInfo, m_Device);
	}

	uint32_t GBufferPass::addTexture(Image* image) {
		auto idSearch = m_TextureIDs.find(image);

		if (idSearch == m_TextureIDs.end()) { // texture is not associated with any ID yet
			uint32_t id = m_TextureIDs.size();

			if (!m_VacantTextureIDs.empty()) {
				id = *m_VacantTextureIDs.begin(); // get the lowest vacant ID
				m_VacantTextureIDs.erase(m_VacantTextureIDs.begin());
			}

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = image->getVulkanImageView();
			imageInfo.sampler = m_Sampler->getVkSampler();

			DescriptorWriter(*m_TextureSetLayout, m_Device.getBindlessPool())
				.writeImage(0, &imageInfo, id)
				.overwrite(m_TextureDescriptorSet);

			m_TextureIDs.insert({ image, id });
			return id;
		}

		return idSearch->second;
	}

	void GBufferPass::freeTextureID(Image* image) {
		//auto idSearch = m_TextureIDs.find(image);

		//if (idSearch == m_TextureIDs.end()) { // texture has no ID, can not free ID
		//	return;
		//}

		//// Clear the image from the texture descriptor
		//VkDescriptorImageInfo imageInfo{};
		//imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		//imageInfo.imageView = m_Textures[0]->getImageView(); // default 1x1 white texture
		//imageInfo.sampler = Renderer::m_TextureSampler;

		//DescriptorWriter(*m_TextureSetLayout, m_Device.getBindlessPool())
		//	.writeImage(0, &imageInfo, idSearch->second)
		//	.overwrite(m_TextureDescriptorSet);

		//m_VacantTextureIDs.emplace(idSearch->second);
		//m_TextureIDs.erase(image);
	}
}