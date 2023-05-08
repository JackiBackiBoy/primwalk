#include "fzui/rendering/systems/uiRenderSystem.hpp"

// std
#include <stdexcept>

namespace fz {

  UIRenderSystem::UIRenderSystem(GraphicsDevice_Vulkan& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout>& setLayouts) :
    m_Device(device) {
    createPipelineLayout(setLayouts);
    createPipeline(renderPass);
    createVertexBuffer();
    createIndexBuffer();
  }

  UIRenderSystem::~UIRenderSystem()
  {
    vkDestroyPipelineLayout(m_Device.getDevice(), m_PipelineLayout, nullptr);

    // Index buffer
    vkDestroyBuffer(m_Device.getDevice(), m_IndexBuffer, nullptr);
    vkFreeMemory(m_Device.getDevice(), m_IndexBufferMemory, nullptr);

    // Vertex buffer
    vkDestroyBuffer(m_Device.getDevice(), m_VertexBuffer, nullptr);
    vkFreeMemory(m_Device.getDevice(), m_VertexBufferMemory, nullptr);
  }

  void UIRenderSystem::onRender(VkCommandBuffer commandBuffer)
  {
    m_Pipeline->bind(commandBuffer);

    VkBuffer vertexBuffers[] = { m_VertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);
  }

  void UIRenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout>& setLayouts)
  {
    // Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0; // optional

    if (vkCreatePipelineLayout(m_Device.getDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to create pipeline layout!");
    }
  }

  void UIRenderSystem::createPipeline(VkRenderPass renderPass)
  {
    assert(m_PipelineLayout != nullptr && "VULKAN ASSERTION FAILED: Cannot create pipeline before pipeline layout!");

    PipelineConfigInfo pipelineConfig{};
    GraphicsPipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = m_PipelineLayout;
    m_Pipeline = std::make_unique<GraphicsPipeline>(
      m_Device,
      "assets/shaders/shader.vert.spv",
      "assets/shaders/shader.frag.spv",
      pipelineConfig);
  }

  void UIRenderSystem::createVertexBuffer()
  {
    VkDeviceSize bufferSize = sizeof(m_Vertices[0]) * m_Vertices.size();

    // Staging buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    GraphicsDevice_Vulkan::createBuffer(m_Device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_Device.getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, m_Vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(m_Device.getDevice(), stagingBufferMemory);

    GraphicsDevice_Vulkan::createBuffer(m_Device, bufferSize,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      m_VertexBuffer, m_VertexBufferMemory);
    GraphicsDevice_Vulkan::copyBuffer(m_Device, stagingBuffer, m_VertexBuffer, bufferSize);

    vkDestroyBuffer(m_Device.getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(m_Device.getDevice(), stagingBufferMemory, nullptr);
  }

  void UIRenderSystem::createIndexBuffer()
  {
    VkDeviceSize bufferSize = sizeof(m_Indices[0]) * m_Indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    GraphicsDevice_Vulkan::createBuffer(m_Device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_Device.getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, m_Indices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(m_Device.getDevice(), stagingBufferMemory);

    GraphicsDevice_Vulkan::createBuffer(m_Device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_IndexBuffer, m_IndexBufferMemory);

    GraphicsDevice_Vulkan::copyBuffer(m_Device, stagingBuffer, m_IndexBuffer, bufferSize);

    vkDestroyBuffer(m_Device.getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(m_Device.getDevice(), stagingBufferMemory, nullptr);
  }

}