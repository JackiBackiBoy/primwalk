#include "deferredPass.hpp"

#include "../components/camera.hpp"
#include "vertex3d.hpp"
#include <stdexcept>

#include <glm/gtc/matrix_transform.hpp>
#include "../components/pointLight.hpp"
#include "../components/renderable.hpp"
#include "../components/transform.hpp"


namespace pw {

	DeferredPass::DeferredPass(uint32_t width, uint32_t height, GraphicsDevice_Vulkan& device) : m_Device(device) {
		createImages(width, height);
		createRenderPasses();
		createFramebuffers(width, height);

		createDescriptorPool();
		createBuffers();
		createDescriptorSetLayout();
		createPipelineLayouts();
		createPipelines(m_OffscreenPass->getVulkanRenderPass());
		createSamplers();

		// Textures
		m_Textures.push_back(std::make_shared<Texture2D>(1, 1, std::vector<uint8_t>(4, 255).data())); // default 1x1 white texture
		addTexture(m_Textures[0]->getImage());

		// TODO: Move cubemap code to separate class (HDRI should also be separate)
		m_Textures.push_back(std::make_shared<Texture2D>("assets/textures/cubemaps/skybox_default.hdr", 4, VK_FORMAT_R16G16B16A16_SFLOAT));
		addTexture(m_Textures[1]->getImage());
	}

	DeferredPass::~DeferredPass() {
		vkDestroyPipelineLayout(m_Device.getDevice(), m_MainPipelineLayout, nullptr);

		m_OffscreenImage->destroy();
		m_OffscreenDepthImage->destroy();
		m_OffscreenFramebuffer->destroy();

		m_DeferredFramebuffer->destroy();
		m_PositionBuffer->destroy();
		m_NormalBuffer->destroy();
		m_DiffuseBuffer->destroy();
		m_SpecularBuffer->destroy();
		m_DeferredDepthBuffer->destroy();
	}

	void DeferredPass::draw(VkCommandBuffer commandBuffer, size_t frameIndex, ComponentManager& manager) {
		UniformBuffer3D ubo{};
		ubo.view = Camera::MainCamera->getViewMatrix();
		ubo.proj = glm::mat4(1.0f);
		ubo.proj = glm::perspective(glm::radians(45.0f), (float)m_OffscreenImage->getWidth() / m_OffscreenImage->getHeight(), 0.1f, 1000.0f);
		ubo.viewPosition = Camera::MainCamera->position;

		uint32_t lightIndex = 0;
		for (const auto& e : m_Entities) {
			if (manager.hasComponent<PointLight>(e)) {
				auto& light = manager.getComponent<PointLight>(e);
				ubo.pointLights[lightIndex].position = manager.getComponent<Transform>(e).position;
				ubo.pointLights[lightIndex].color = light.color;
				lightIndex++;
			}
		}

		ubo.numLights = lightIndex;

		m_UBOs[frameIndex]->writeToBuffer(&ubo);

		// Geometry pass
		m_GeometryPass->begin(*m_DeferredFramebuffer, commandBuffer);

			m_GBufferPipeline->bind(commandBuffer);

			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				m_MainPipelineLayout, 0, 1, &m_UniformDescriptorSets[frameIndex], 0, nullptr);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				m_MainPipelineLayout, 1, 1, &m_TextureDescriptorSet, 0, nullptr);

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
					uint32_t texIndex = 0;

					if (diffuseMap) {
						texIndex = addTexture(diffuseMap->getImage());
					}

					push.diffuseTexIndex = texIndex;

					texIndex = 0;

					if (normalMap) {
						texIndex = addTexture(normalMap->getImage());
					}

					push.normalMapIndex = texIndex;

					vkCmdPushConstants(
						commandBuffer,
						m_MainPipelineLayout,
						VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
						0,
						sizeof(ModelPushConstant),
						&push);

					vkCmdDrawIndexed(commandBuffer, mesh.indices, 1, mesh.baseIndex, mesh.baseVertex, 0);
				}
			}

		m_GeometryPass->end(commandBuffer);

		// Main pass
		m_OffscreenPass->begin(*m_OffscreenFramebuffer, commandBuffer);
		m_MainPipeline->bind(commandBuffer);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_MainPipelineLayout, 0, 1, &m_UniformDescriptorSets[frameIndex], 0, nullptr);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_MainPipelineLayout, 1, 1, &m_TextureDescriptorSet, 0, nullptr);

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
				uint32_t texIndex = 0;

				if (diffuseMap) {
					texIndex = addTexture(diffuseMap->getImage());
				}

				push.diffuseTexIndex = texIndex;

				texIndex = 0;

				if (normalMap) {
					texIndex = addTexture(normalMap->getImage());
				}

				push.normalMapIndex = texIndex;

				vkCmdPushConstants(
					commandBuffer,
					m_MainPipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					sizeof(ModelPushConstant),
					&push);

				vkCmdDrawIndexed(commandBuffer, mesh.indices, 1, mesh.baseIndex, mesh.baseVertex, 0);
			}
		}

		m_OffscreenPass->end(commandBuffer);
	}

	void DeferredPass::resize(uint32_t width, uint32_t height) {
		m_Device.waitForGPU();

		m_OffscreenFramebuffer->destroy();
		m_OffscreenImage->destroy();
		m_OffscreenDepthImage->destroy();

		m_DeferredFramebuffer->destroy();
		m_PositionBuffer->destroy();
		m_NormalBuffer->destroy();
		m_DiffuseBuffer->destroy();
		m_SpecularBuffer->destroy();
		m_DeferredDepthBuffer->destroy();

		createImages(width, height);
		createFramebuffers(width, height);
	}

	void DeferredPass::createImages(uint32_t width, uint32_t height) {
		// Color
		ImageInfo imageInfo{};
		imageInfo.width = width;
		imageInfo.height = height;
		imageInfo.depth = 1;
		imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT; // HDR internal format

		m_OffscreenImage = std::make_unique<Image>(imageInfo);

		// Depth
		ImageInfo depthImageInfo{};
		depthImageInfo.width = width;
		depthImageInfo.height = height;
		depthImageInfo.depth = 1;
		depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		depthImageInfo.format = m_Device.getSupportedDepthFormat();

		m_OffscreenDepthImage = std::make_unique<Image>(depthImageInfo);

		// Deferred images
		// Position
		ImageInfo positionImageInfo;
		positionImageInfo.width = width;
		positionImageInfo.height = height;
		positionImageInfo.depth = 1;
		positionImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		positionImageInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;

		// Normals
		ImageInfo normalImageInfo;
		normalImageInfo.width = width;
		normalImageInfo.height = height;
		normalImageInfo.depth = 1;
		normalImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		normalImageInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;

		// Diffuse
		ImageInfo diffuseImageInfo;
		diffuseImageInfo.width = width;
		diffuseImageInfo.height = height;
		diffuseImageInfo.depth = 1;
		diffuseImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		diffuseImageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;

		// Specular
		ImageInfo specularImageInfo;
		specularImageInfo.width = width;
		specularImageInfo.height = height;
		specularImageInfo.depth = 1;
		specularImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		specularImageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;

		m_PositionBuffer = std::make_unique<Image>(positionImageInfo);
		m_NormalBuffer = std::make_unique<Image>(normalImageInfo);
		m_DiffuseBuffer = std::make_unique<Image>(diffuseImageInfo);
		m_SpecularBuffer = std::make_unique<Image>(specularImageInfo);

		// Deferred depth image
		m_DeferredDepthBuffer = std::make_unique<Image>(depthImageInfo);
	}

	void DeferredPass::createRenderPasses() {
		// No subpasses on desktop
		SubpassInfo subpass{};
		subpass.renderTargets = { 0 };

		// Offscreen renderpass
		RenderPassAttachment colorAttachment = {
			m_OffscreenImage,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		RenderPassAttachment depthAttachment = {
			m_OffscreenDepthImage,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};

		std::vector<RenderPassAttachment> attachments = { colorAttachment, depthAttachment };
		RenderPassInfo passInfo = {
			attachments,
			{ subpass }
		};

		m_OffscreenPass = std::make_unique<RenderPass>(passInfo);

		// Deferred geometry pass
		// Position buffer attachment
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

		RenderPassAttachment diffuseAttachment = {
			m_DiffuseBuffer,
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

		RenderPassAttachment deferredDepth = {
			m_DeferredDepthBuffer,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};

		std::vector<RenderPassAttachment> deferredAttachments = {
			positionAttachment,
			normalAttachment,
			diffuseAttachment,
			specularAttachment,
			deferredDepth
		};

		RenderPassInfo geometryPassInfo = {
			deferredAttachments,
			{ subpass }
		};

		m_GeometryPass = std::make_unique<RenderPass>(geometryPassInfo);
	}

	void DeferredPass::createFramebuffers(uint32_t width, uint32_t height) {
		// Offscreen framebuffer
		FramebufferInfo framebufferInfo = {
			width,
			height,
			m_OffscreenPass->getVulkanRenderPass(),
			{ { m_OffscreenImage, m_OffscreenDepthImage } }
		};

		m_OffscreenFramebuffer = std::make_unique<Framebuffer>(framebufferInfo);

		// Deferred framebuffer
		FramebufferInfo deferredInfo = {
			width,
			height,
			m_GeometryPass->getVulkanRenderPass(),
			{
				{
					m_PositionBuffer,
					m_NormalBuffer,
					m_DiffuseBuffer,
					m_SpecularBuffer,
					m_DeferredDepthBuffer
				}
			}
		};

		m_DeferredFramebuffer = std::make_unique<Framebuffer>(deferredInfo);
	}

	void DeferredPass::createDescriptorPool() {
		m_UniformDescriptorSets.resize(GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT);
	}

	void DeferredPass::createBuffers() {
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

	void DeferredPass::createDescriptorSetLayout() {
		// Descriptor set layouts
		m_UBOSetLayout = DescriptorSetLayout::Builder(m_Device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		m_TextureSetLayout = DescriptorSetLayout::Builder(m_Device)
			.setLayoutFlags(VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1024,
				VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT)
			.build();

		// Uniform buffer
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

	void DeferredPass::createPipelineLayouts() {
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

		if (vkCreatePipelineLayout(m_Device.getDevice(), &basePipelineLayoutInfo, nullptr, &m_MainPipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("VULKAN ERROR: Failed to create pipeline layout!");
		}
	}

	void DeferredPass::createPipelines(VkRenderPass renderPass) {
		assert(m_MainPipelineLayout != nullptr && "VULKAN ASSERTION FAILED: Can not create pipeline before pipeline layout!");

		// Base pipeline
		PipelineConfigInfo pipelineConfig{};
		GraphicsPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.bindingDescriptions = Vertex3D::getBindingDescriptions();
		pipelineConfig.attributeDescriptions = Vertex3D::getAttributeDescriptions();
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = m_MainPipelineLayout;

		m_MainPipeline = std::make_unique<GraphicsPipeline>(
			m_Device,
			"assets/shaders/shader3d.vert.spv",
			"assets/shaders/shader3d.frag.spv",
			pipelineConfig);

		// G-Buffer pipeline
		PipelineConfigInfo gBufferPipelineConfig{};
		GraphicsPipeline::defaultPipelineConfigInfo(gBufferPipelineConfig);
		gBufferPipelineConfig.bindingDescriptions = Vertex3D::getBindingDescriptions();
		gBufferPipelineConfig.attributeDescriptions = Vertex3D::getAttributeDescriptions();
		gBufferPipelineConfig.renderPass = m_GeometryPass->getVulkanRenderPass();
		gBufferPipelineConfig.pipelineLayout = m_MainPipelineLayout;

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

	void DeferredPass::createSamplers() {
		SamplerCreateInfo samplerInfo{};
		m_Sampler = std::make_unique<Sampler>(samplerInfo, m_Device);
	}

	uint32_t DeferredPass::addTexture(Image* image) {
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

	void DeferredPass::freeTextureID(Image* image) {
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