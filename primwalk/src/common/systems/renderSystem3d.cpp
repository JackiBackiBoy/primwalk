#include "primwalk/systems/renderSystem3d.hpp"

#include "primwalk/managers/componentManager.hpp"
#include "primwalk/rendering/buffer.hpp"
#include "primwalk/rendering/descriptors.hpp"
#include "primwalk/rendering/graphicsPipeline.hpp"
#include "primwalk/rendering/renderer.hpp"
#include "primwalk/components/camera.hpp"
#include "primwalk/components/renderable.hpp"
#include "primwalk/components/transform.hpp"

// std
#include <cassert>
#include <stdexcept>

// vendor
#include <glm/gtc/matrix_transform.hpp>

namespace pw {

	RenderSystem3D::RenderSystem3D(GraphicsDevice_Vulkan& device, VkRenderPass renderPass) :
		m_Device(device) {
		createDescriptorPool();
		createUniformBuffers();
		createDescriptorSetLayout();
		createPipelineLayout();
		createPipeline(renderPass);
		createVertexBuffer();
		createIndexBuffer();

		m_DefaultModel = std::make_shared<Model>();
		m_DefaultModel->loadFromFile("assets/models/cube.gltf");
	}

	RenderSystem3D::~RenderSystem3D() {
		vkDestroyPipelineLayout(m_Device.getDevice(), m_PipelineLayout, nullptr);
	}

	void RenderSystem3D::onUpdate(const FrameInfo& frameInfo) {
		static float ang = 0.0f;
		ang += frameInfo.frameTime * 40.0f;

		auto& camera = Camera::MainCamera;

		UniformBuffer3D ubo{};
		ubo.view = camera->getViewMatrix();
		ubo.proj = glm::mat4(1.0f);
		ubo.proj = glm::perspective(glm::radians(45.0f), (float)frameInfo.windowWidth / frameInfo.windowHeight, 0.01f, 100.0f);
		ubo.proj[1][1] *= -1.0f;

		m_UniformBuffers[frameInfo.frameIndex]->writeToBuffer(&ubo);
	}

	void RenderSystem3D::onRender(const FrameInfo& frameInfo, ComponentManager& manager) {
		m_Pipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_PipelineLayout, 0, 1, &m_UniformDescriptorSets[frameInfo.frameIndex], 0, nullptr);
		vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_PipelineLayout, 1, 1, &m_Device.m_TextureDescriptorSet, 0, nullptr);

		for (const auto& e : entities) {
			auto& component = manager.getComponent<Renderable>(e);
			Model* model = component.model;

			if (!model) { // render default cube model
				//continue;
				model = m_DefaultModel.get();
			}

			ModelPushConstant push{};
			push.modelMatrix = glm::translate(push.modelMatrix, manager.getComponent<Transform>(e).position);
			
			glm::vec4 normColor = Color::normalize(component.color);
			push.color = { normColor.r, normColor.g, normColor.b };
			uint32_t texIndex = 0;

			for (const auto& t : model->getTextures()) {
				bool hasID = m_Device.getTextureID(t->getImage(), &texIndex);

				if (hasID) { continue; }

				texIndex = m_Device.addTextureID(t->getImage());

				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = t->getImageView();
				imageInfo.sampler = Renderer::m_TextureSampler;

				DescriptorWriter(*m_Device.m_TextureSetLayout, *m_Device.m_BindlessDescriptorPool)
					.writeImage(0, &imageInfo, texIndex)
					.overwrite(m_Device.m_TextureDescriptorSet);
			}

			push.diffuseTexIndex = texIndex;

			vkCmdPushConstants(
				frameInfo.commandBuffer,
				m_PipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(ModelPushConstant),
				&push);

			model->draw(frameInfo.commandBuffer);
		}
	}

	void RenderSystem3D::createDescriptorPool() {
		m_UniformDescriptorSets.resize(GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT);
	}

	void RenderSystem3D::createUniformBuffers() {
		VkDeviceSize bufferSize = sizeof(UniformBuffer3D);
		m_UniformBuffers.resize(GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT);

		for (auto& ub : m_UniformBuffers) {
			ub = std::make_unique<Buffer>(
				m_Device,
				sizeof(UniformBuffer3D),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			ub->map();
		}
	}

	void RenderSystem3D::createDescriptorSetLayout() {
		GraphicsDevice_Vulkan* device = (GraphicsDevice_Vulkan*&)pw::GetDevice();

		m_UniformSetLayout = DescriptorSetLayout::Builder(m_Device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.build();

		// Uniform buffer
		for (size_t i = 0; i < m_UniformDescriptorSets.size(); i++) {
			auto bufferInfo = m_UniformBuffers[i]->getDescriptorInfo();

			DescriptorWriter(*m_UniformSetLayout, *device->m_BindlessDescriptorPool)
				.writeBuffer(0, &bufferInfo)
				.build(m_UniformDescriptorSets[i]);
		}

		m_DescriptorSetLayouts = {
			m_UniformSetLayout->getDescriptorSetLayout(),
			device->m_TextureSetLayout->getDescriptorSetLayout()
		};
	}

	void RenderSystem3D::createPipelineLayout() {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(ModelPushConstant);

		VkPipelineLayoutCreateInfo basePipelineLayoutInfo{};
		basePipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		basePipelineLayoutInfo.pushConstantRangeCount = 0;
		basePipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(m_DescriptorSetLayouts.size());
		basePipelineLayoutInfo.pSetLayouts = m_DescriptorSetLayouts.data();
		basePipelineLayoutInfo.pushConstantRangeCount = 1;
		basePipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(m_Device.getDevice(), &basePipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("VULKAN ERROR: Failed to create pipeline layout!");
		}
	}

	void RenderSystem3D::createPipeline(VkRenderPass renderPass) {
		assert(m_PipelineLayout != nullptr && "VULKAN ASSERTION FAILED: Can not create pipeline before pipeline layout!");

		// Base pipeline
		PipelineConfigInfo pipelineConfig{};
		GraphicsPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.bindingDescriptions = Vertex3D::getBindingDescriptions();
		pipelineConfig.attributeDescriptions = Vertex3D::getAttributeDescriptions();
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = m_PipelineLayout;

		m_Pipeline = std::make_unique<GraphicsPipeline>(
			m_Device,
			"assets/shaders/shader3d.vert.spv",
			"assets/shaders/shader3d.frag.spv",
			pipelineConfig);
	}

	void RenderSystem3D::createVertexBuffer() {
		//VkDeviceSize bufferSize = sizeof(m_Vertices[0]) * m_Vertices.size();
		//uint32_t vertexCount = static_cast<uint32_t>(m_Vertices.size());
		//uint32_t vertexSize = sizeof(m_Vertices[0]);

		//// Staging buffer
		//Buffer stagingBuffer(m_Device, vertexSize, vertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		//  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		//stagingBuffer.map();
		//stagingBuffer.writeToBuffer((void*)m_Vertices.data());
		//stagingBuffer.unmap();

		//m_VertexBuffer = std::make_unique<Buffer>(
		//  m_Device,
		//  vertexSize,
		//  vertexCount,
		//  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		//  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		//);

		//m_Device.copyBuffer(stagingBuffer.getBuffer(), m_VertexBuffer->getBuffer(), bufferSize);
	}

	void RenderSystem3D::createIndexBuffer() {
		//VkDeviceSize bufferSize = sizeof(m_Indices[0]) * m_Indices.size();
		//uint32_t indexCount = static_cast<uint32_t>(m_Indices.size());
		//uint32_t indexSize = sizeof(m_Indices[0]);

		//// Staging buffer
		//Buffer stagingBuffer(m_Device, indexSize, indexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		//  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		//stagingBuffer.map();
		//stagingBuffer.writeToBuffer((void*)m_Indices.data());
		//stagingBuffer.unmap();

		//// Index buffer
		//m_IndexBuffer = std::make_unique<Buffer>(
		//  m_Device,
		//  indexSize,
		//  indexCount,
		//  VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		//  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		//);

		//m_Device.copyBuffer(stagingBuffer.getBuffer(), m_IndexBuffer->getBuffer(), bufferSize);
	}

}