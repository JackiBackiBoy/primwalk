#include "primwalk/rendering/systems/renderSystem3d.hpp"

#include "primwalk/rendering/buffer.hpp"
#include "primwalk/rendering/descriptors.hpp"
#include "primwalk/rendering/graphicsPipeline.hpp"

// std
#include <cassert>
#include <stdexcept>

// vendor
#include <glm/gtc/matrix_transform.hpp>

namespace pw {

  RenderSystem3D::RenderSystem3D(GraphicsDevice_Vulkan& device, VkRenderPass renderPass) :
    m_Device(device)
  {
    createDescriptorPool();
    createUniformBuffers();
    createDescriptorSetLayout();
    createPipelineLayout();
    createPipeline(renderPass);
    createVertexBuffer();
    createIndexBuffer();
  }

  RenderSystem3D::~RenderSystem3D()
  {
    vkDestroyPipelineLayout(m_Device.getDevice(), m_PipelineLayout, nullptr);
  }

  void RenderSystem3D::onUpdate(const FrameInfo& frameInfo)
  {
    // TODO: Update uniform buffers
    // TODO: Fix frameInfo dimension flickering on resize
    UniformBufferObject ubo{};
    ubo.proj = glm::ortho(0.0f, (float)frameInfo.windowWidth, 0.0f, (float)frameInfo.windowHeight);

    //memcpy(m_UniformBuffersMapped[frameInfo.frameIndex], &ubo, sizeof(ubo));
    m_UniformBuffers[frameInfo.frameIndex]->writeToBuffer(&ubo);
  }

  void RenderSystem3D::onRender(const FrameInfo& frameInfo)
  {
    m_Pipeline->bind(frameInfo.commandBuffer);

    VkBuffer vertexBuffers[] = { m_VertexBuffer->getBuffer() };
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(frameInfo.commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(frameInfo.commandBuffer, m_IndexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT16);

    vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
      m_PipelineLayout, 0, 1, &m_UniformDescriptorSets[frameInfo.frameIndex], 0, nullptr);
    vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
      m_PipelineLayout, 1, 1, &m_Device.m_TextureDescriptorSet, 0, nullptr);

    vkCmdDrawIndexed(frameInfo.commandBuffer, static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);
  }

  void RenderSystem3D::createDescriptorPool()
  {
    m_DescriptorPool = DescriptorPool::Builder(m_Device)
      .setMaxSets(GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT * 2)
      .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT) // uniform buffer
      .build();

    m_UniformDescriptorSets.resize(GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT);
  }

  void RenderSystem3D::createUniformBuffers()
  {
    // Uniform buffer
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    m_UniformBuffers.resize(GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT; i++) {
      m_UniformBuffers[i] = std::make_unique<Buffer>(
        m_Device,
        sizeof(UniformBufferObject),
        1,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
      );
      m_UniformBuffers[i]->map();
    }
  }

  void RenderSystem3D::createDescriptorSetLayout()
  {
    GraphicsDevice_Vulkan* device = pw::GetDevice();

    m_UniformSetLayout = DescriptorSetLayout::Builder(m_Device)
      .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
      .build();

    // Uniform buffer
    for (size_t i = 0; i < m_UniformDescriptorSets.size(); i++) {
      auto bufferInfo = m_UniformBuffers[i]->getDescriptorInfo();
      DescriptorWriter(*m_UniformSetLayout, *m_DescriptorPool)
        .writeBuffer(0, &bufferInfo)
        .build(m_UniformDescriptorSets[i]);
    }

    m_DescriptorSetLayouts = {
      m_UniformSetLayout->getDescriptorSetLayout(),
      device->m_TextureSetLayout->getDescriptorSetLayout()
    };
  }

  void RenderSystem3D::createPipelineLayout()
  {
    VkPipelineLayoutCreateInfo basePipelineLayoutInfo{};
    basePipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    basePipelineLayoutInfo.pushConstantRangeCount = 0;
    basePipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(m_DescriptorSetLayouts.size());
    basePipelineLayoutInfo.pSetLayouts = m_DescriptorSetLayouts.data();

    if (vkCreatePipelineLayout(m_Device.getDevice(), &basePipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to create pipeline layout!");
    }
  }

  void RenderSystem3D::createPipeline(VkRenderPass renderPass)
  {
    assert(m_PipelineLayout != nullptr && "VULKAN ASSERTION FAILED: Can not create pipeline before pipeline layout!");

    // Base pipeline
    PipelineConfigInfo pipelineConfig{};
    GraphicsPipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = m_PipelineLayout;
    m_Pipeline = std::make_unique<GraphicsPipeline>(
      m_Device,
      "assets/shaders/shader3d.vert.spv",
      "assets/shaders/shader3d.frag.spv",
      pipelineConfig);
  }

  void RenderSystem3D::createVertexBuffer()
  {
    VkDeviceSize bufferSize = sizeof(m_Vertices[0]) * m_Vertices.size();
    uint32_t vertexCount = static_cast<uint32_t>(m_Vertices.size());
    uint32_t vertexSize = sizeof(m_Vertices[0]);

    // Staging buffer
    Buffer stagingBuffer(m_Device, vertexSize, vertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void*)m_Vertices.data());
    stagingBuffer.unmap();

    m_VertexBuffer = std::make_unique<Buffer>(
      m_Device,
      vertexSize,
      vertexCount,
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    m_Device.copyBuffer(stagingBuffer.getBuffer(), m_VertexBuffer->getBuffer(), bufferSize);
  }

  void RenderSystem3D::createIndexBuffer()
  {
    VkDeviceSize bufferSize = sizeof(m_Indices[0]) * m_Indices.size();
    uint32_t indexCount = static_cast<uint32_t>(m_Indices.size());
    uint32_t indexSize = sizeof(m_Indices[0]);

    // Staging buffer
    Buffer stagingBuffer(m_Device, indexSize, indexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void*)m_Indices.data());
    stagingBuffer.unmap();

    // Index buffer
    m_IndexBuffer = std::make_unique<Buffer>(
      m_Device,
      indexSize,
      indexCount,
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    m_Device.copyBuffer(stagingBuffer.getBuffer(), m_IndexBuffer->getBuffer(), bufferSize);
  }

}