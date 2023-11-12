#include "renderSystem3d.hpp"

#include "../managers/componentManager.hpp"
#include "../rendering/buffer.hpp"
#include "../rendering/descriptors.hpp"
#include "../rendering/graphicsPipeline.hpp"
#include "../rendering/renderer.hpp"
#include "../components/camera.hpp"
#include "../components/pointLight.hpp"
#include "../components/renderable.hpp"
#include "../components/transform.hpp"

// std
#include <cassert>
#include <stdexcept>

// vendor
#include <glm/gtc/matrix_transform.hpp>

namespace pw {

	RenderSystem3D::RenderSystem3D(GraphicsDevice_Vulkan& device, VkRenderPass renderPass) : m_Device(device) {
		createDescriptorPool();
		createBuffers();
		createDescriptorSetLayout();
		createPipelineLayouts();
		createPipelines(renderPass);
		createSamplers();

		// Textures
		m_Textures.push_back(std::make_shared<Texture2D>(1, 1, std::vector<uint8_t>(4, 255).data())); // default 1x1 white texture
		addTexture(m_Textures[0]->getImage());

		// TODO: Move cubemap code to separate class (HDRI should also be separate)
		m_Textures.push_back(std::make_shared<Texture2D>("assets/textures/cubemaps/skybox_default.hdr", 4, VK_FORMAT_R16G16B16A16_SFLOAT));
		addTexture(m_Textures[1]->getImage());


		m_DefaultModel = std::make_shared<Model>();
		m_DefaultModel->loadFromFile("assets/models/cube.gltf");
	}

	RenderSystem3D::~RenderSystem3D() {
		vkDestroyPipelineLayout(m_Device.getDevice(), m_MainPipelineLayout, nullptr);
		vkDestroyPipelineLayout(m_Device.getDevice(), m_DebugPipelineLayout, nullptr);
		vkDestroyPipelineLayout(m_Device.getDevice(), m_SkyboxPipelineLayout, nullptr);
	}

	void RenderSystem3D::onUpdate(const FrameInfo& frameInfo, ComponentManager& manager) {
		UniformBuffer3D ubo{};
		ubo.view = Camera::MainCamera->getViewMatrix();
		ubo.proj = glm::mat4(1.0f);
		ubo.proj = glm::perspective(glm::radians(45.0f), (float)frameInfo.windowWidth / frameInfo.windowHeight, 0.1f, 1000.0f);
		ubo.viewPosition = Camera::MainCamera->position;

		uint32_t lightIndex = 0;
		for (const auto& e : entities) {
			if (manager.hasComponent<PointLight>(e)) {
				auto& light = manager.getComponent<PointLight>(e);
				ubo.pointLights[lightIndex].position = manager.getComponent<Transform>(e).position;
				ubo.pointLights[lightIndex].color = light.color;
				lightIndex++;
			}
		}

		ubo.numLights = lightIndex;

		m_UBOs[frameInfo.frameIndex]->writeToBuffer(&ubo);
	}

	void RenderSystem3D::onRender(const FrameInfo& frameInfo, ComponentManager& manager) {
		// Skybox pipeline
		m_SkyboxPipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_SkyboxPipelineLayout, 0, 1, &m_UniformDescriptorSets[frameInfo.frameIndex], 0, nullptr);
		vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_SkyboxPipelineLayout, 1, 1, &m_TextureDescriptorSet, 0, nullptr);

		SkyboxPushConstant skyboxPush{};
		skyboxPush.modelMatrix = glm::scale(skyboxPush.modelMatrix, glm::vec3(30.0f));
		
		skyboxPush.texIndex = addTexture(m_Textures[1]->getImage()); // TODO: this is stupid, fix it
		vkCmdPushConstants(
			frameInfo.commandBuffer,
			m_SkyboxPipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(SkyboxPushConstant),
			&skyboxPush);

		m_DefaultModel->bind(frameInfo.commandBuffer);
		Mesh& cube = (m_DefaultModel->getMeshes())[0];
		vkCmdDrawIndexed(frameInfo.commandBuffer, cube.indices, 1, cube.baseIndex, cube.baseVertex, 0);

		// Main pipeline
		m_MainPipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_MainPipelineLayout, 0, 1, &m_UniformDescriptorSets[frameInfo.frameIndex], 0, nullptr);
		vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_MainPipelineLayout, 1, 1, &m_TextureDescriptorSet, 0, nullptr);

		for (const auto& e : entities) {
			auto& component = manager.getComponent<Renderable>(e);
			Model* model = component.model;

			if (!model) { // render default cube model
				drawDebugBox(manager.getComponent<Transform>(e).position, glm::vec3(1.0f));
				continue;
			}
			
			model->bind(frameInfo.commandBuffer);

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
					frameInfo.commandBuffer,
					m_MainPipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					sizeof(ModelPushConstant),
					&push);

				vkCmdDrawIndexed(frameInfo.commandBuffer, mesh.indices, 1, mesh.baseIndex, mesh.baseVertex, 0);
			}
		}

		// Debug pipeline
		if (!m_DebugLineParams.empty()) {
			m_DebugSSBOs[frameInfo.frameIndex]->writeToBuffer(m_DebugLineParams.data(), m_DebugLineParams.size() * sizeof(DebugLineParams));
		}
		m_DebugPipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_DebugPipelineLayout, 0, 1, &m_UniformDescriptorSets[frameInfo.frameIndex], 0, nullptr);
		vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_DebugPipelineLayout, 1, 1, &m_DebugStorageDescriptorSets[frameInfo.frameIndex], 0, nullptr);

		DebugPushConstant debugPush{};
		vkCmdPushConstants(
			frameInfo.commandBuffer,
			m_DebugPipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(DebugPushConstant),
			&debugPush);

		vkCmdDraw(frameInfo.commandBuffer, 2, static_cast<uint32_t>(m_DebugLineParams.size()), 0, 0);

		m_DebugLineParams.clear();


	}

	void RenderSystem3D::drawDebugLine(glm::vec3 p1, glm::vec3 p2) {
		DebugLineParams params = { p1, p2, { 1.0, 0.0, 0.0 } };
		m_DebugLineParams.push_back(params);
	}

	void RenderSystem3D::drawDebugBox(glm::vec3 pos, glm::vec3 volume) {
		const glm::vec3 corners[4] = { // top face corners
			{ pos.x + 0.5f * volume.x, pos.y + 0.5f * volume.y, pos.z + 0.5f * volume.z },
			{ pos.x + 0.5f * volume.x, pos.y + 0.5f * volume.y, pos.z - 0.5f * volume.z },
			{ pos.x - 0.5f * volume.x, pos.y + 0.5f * volume.y, pos.z - 0.5f * volume.z },
			{ pos.x - 0.5f * volume.x, pos.y + 0.5f * volume.y, pos.z + 0.5f * volume.z },
		};

		for (size_t i = 0; i < 4; i++) {
			DebugLineParams param = { 
				corners[i],
				corners[(i + 1) % 4],
				{ 1.0f, 1.0f, 1.0f }
			};

			m_DebugLineParams.push_back(param);
		}

		for (size_t i = 0; i < 4; i++) {
			DebugLineParams param = {
				corners[i] - glm::vec3(0.0f, volume.y, 0.0f),
				corners[(i + 1) % 4] - glm::vec3(0.0f, volume.y, 0.0f),
				{ 1.0f, 1.0f, 1.0f }
			};
			m_DebugLineParams.push_back(param);
		}

		for (size_t i = 0; i < 4; i++) {
			DebugLineParams param = {
				corners[i],
				corners[i] - glm::vec3(0.0f, volume.y, 0.0f),
				{ 1.0f, 1.0f, 1.0f }
			};

			m_DebugLineParams.push_back(param);
		}
	}

	void RenderSystem3D::createDescriptorPool() {
		m_UniformDescriptorSets.resize(GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT);
		m_DebugStorageDescriptorSets.resize(GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT);
	}

	void RenderSystem3D::createBuffers() {
		m_UBOs.resize(GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT);
		m_DebugSSBOs.resize(GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT);

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

		// Debug lines SSBOs
		for (auto& ssbo : m_DebugSSBOs) {
			ssbo = std::make_unique<Buffer>(
				m_Device,
				sizeof(DebugLineParams),
				4096,
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
			);

			ssbo->map();
		}
	}

	void RenderSystem3D::createDescriptorSetLayout() {
		GraphicsDevice_Vulkan* device = (GraphicsDevice_Vulkan*&)pw::GetDevice();

		// Descriptor set layouts
		m_UBOSetLayout = DescriptorSetLayout::Builder(m_Device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		m_DebugSSBOSetLayout = DescriptorSetLayout::Builder(m_Device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
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

		// Debug lines SSBOs
		for (size_t i = 0; i < m_DebugStorageDescriptorSets.size(); i++) {
			auto bufferInfo = m_DebugSSBOs[i]->getDescriptorInfo();

			DescriptorWriter(*m_DebugSSBOSetLayout, m_Device.getBindlessPool())
				.writeBuffer(0, &bufferInfo)
				.build(m_DebugStorageDescriptorSets[i]);
		}

		DescriptorWriter(*m_TextureSetLayout, m_Device.getBindlessPool())
			.build(m_TextureDescriptorSet);

		m_MainDescriptorSetLayouts = {
			m_UBOSetLayout->getDescriptorSetLayout(),
			m_TextureSetLayout->getDescriptorSetLayout()
		};

		m_DebugDescriptorSetLayouts = {
			m_UBOSetLayout->getDescriptorSetLayout(),
			m_DebugSSBOSetLayout->getDescriptorSetLayout()
		};

		m_SkyboxDescriptorSetLayouts = {
			m_UBOSetLayout->getDescriptorSetLayout(),
			m_TextureSetLayout->getDescriptorSetLayout()
		};
	}

	void RenderSystem3D::createPipelineLayouts() {
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

		// Debug pipeline layout
		VkPushConstantRange debugPushConstantRange{};
		debugPushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		debugPushConstantRange.offset = 0;
		debugPushConstantRange.size = sizeof(DebugPushConstant);

		VkPipelineLayoutCreateInfo debugPipelineLayoutInfo{};
		debugPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		debugPipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(m_DebugDescriptorSetLayouts.size());
		debugPipelineLayoutInfo.pSetLayouts = m_DebugDescriptorSetLayouts.data();
		debugPipelineLayoutInfo.pushConstantRangeCount = 1;
		debugPipelineLayoutInfo.pPushConstantRanges = &debugPushConstantRange;
		
		if (vkCreatePipelineLayout(m_Device.getDevice(), &debugPipelineLayoutInfo, nullptr, &m_DebugPipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("VULKAN ERROR: Failed to create debug pipeline layout!");
		}

		// Skybox pipeline layout
		VkPushConstantRange skyboxPushConstantRange{};
		skyboxPushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		skyboxPushConstantRange.offset = 0;
		skyboxPushConstantRange.size = sizeof(SkyboxPushConstant);

		VkPipelineLayoutCreateInfo skyboxPipelineLayoutInfo{};
		skyboxPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		skyboxPipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(m_SkyboxDescriptorSetLayouts.size());
		skyboxPipelineLayoutInfo.pSetLayouts = m_SkyboxDescriptorSetLayouts.data();
		skyboxPipelineLayoutInfo.pushConstantRangeCount = 1;
		skyboxPipelineLayoutInfo.pPushConstantRanges = &skyboxPushConstantRange;

		if (vkCreatePipelineLayout(m_Device.getDevice(), &skyboxPipelineLayoutInfo, nullptr, &m_SkyboxPipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("VULKAN ERROR: Failed to create skybox pipeline layout!");
		}
	}

	void RenderSystem3D::createPipelines(VkRenderPass renderPass) {
		assert(m_MainPipelineLayout != nullptr && "VULKAN ASSERTION FAILED: Can not create pipeline before pipeline layout!");
		assert(m_DebugPipelineLayout != nullptr && "VULKAN ASSERTION FAILED: Can not create pipeline before pipeline layout!");
		assert(m_SkyboxPipelineLayout != nullptr && "VULKAN ASSERTION FAILED: Can not create pipeline before pipeline layout!");

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

		// Debug pipeline
		PipelineConfigInfo debugPipelineConfig{};
		GraphicsPipeline::defaultPipelineConfigInfo(debugPipelineConfig);
		debugPipelineConfig.bindingDescriptions = {};
		debugPipelineConfig.attributeDescriptions = {};
		debugPipelineConfig.renderPass = renderPass;
		debugPipelineConfig.pipelineLayout = m_DebugPipelineLayout;
		debugPipelineConfig.rasterizationInfo.polygonMode = VK_POLYGON_MODE_LINE;
		debugPipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
		debugPipelineConfig.rasterizationInfo.frontFace = {};
		debugPipelineConfig.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

		m_DebugPipeline = std::make_unique<GraphicsPipeline>(
			m_Device,
			"assets/shaders/debug3d.vert.spv",
			"assets/shaders/debug3d.frag.spv",
			debugPipelineConfig);

		// Skybox pipeline
		PipelineConfigInfo skyboxPipelineConfig{};
		GraphicsPipeline::defaultPipelineConfigInfo(skyboxPipelineConfig);
		skyboxPipelineConfig.bindingDescriptions = Vertex3D::getBindingDescriptions();
		skyboxPipelineConfig.attributeDescriptions = Vertex3D::getAttributeDescriptions();
		skyboxPipelineConfig.renderPass = renderPass;
		skyboxPipelineConfig.pipelineLayout = m_SkyboxPipelineLayout;

		m_SkyboxPipeline = std::make_unique<GraphicsPipeline>(
			m_Device,
			"assets/shaders/skybox.vert.spv",
			"assets/shaders/skybox.frag.spv",
			skyboxPipelineConfig);
	}

	void RenderSystem3D::createSamplers() {
		SamplerCreateInfo samplerInfo{};
		m_Sampler = std::make_unique<Sampler>(samplerInfo, m_Device);
	}

	uint32_t RenderSystem3D::addTexture(Image* image) {
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

	void RenderSystem3D::freeTextureID(Image* image) {
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