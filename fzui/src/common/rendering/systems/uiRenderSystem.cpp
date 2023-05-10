#include "fzui/rendering/systems/uiRenderSystem.hpp"
#include "fzui/rendering/renderer.hpp"

// std
#include <stdexcept>

// vendor
#include <glm/gtc/matrix_transform.hpp>

namespace fz {

  UIRenderSystem::UIRenderSystem(GraphicsDevice_Vulkan& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout>& setLayouts) :
    m_Device(device) {
    createDescriptorSetLayout();
    createPipelineLayout(setLayouts);
    createPipeline(renderPass);
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
  }

  UIRenderSystem::~UIRenderSystem()
  {
    vkDestroyPipelineLayout(m_Device.getDevice(), m_PipelineLayout, nullptr);

    for (size_t i = 0; i < Renderer::MAX_FRAMES_IN_FLIGHT; i++) {
      vkDestroyBuffer(m_Device.getDevice(), m_UniformBuffers[i], nullptr);
      vkFreeMemory(m_Device.getDevice(), m_UniformBuffersMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(m_Device.getDevice(), m_DescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(m_Device.getDevice(), m_DescriptorSetLayout, nullptr);

    // Index buffer
    vkDestroyBuffer(m_Device.getDevice(), m_IndexBuffer, nullptr);
    vkFreeMemory(m_Device.getDevice(), m_IndexBufferMemory, nullptr);

    // Vertex buffer
    vkDestroyBuffer(m_Device.getDevice(), m_VertexBuffer, nullptr);
    vkFreeMemory(m_Device.getDevice(), m_VertexBufferMemory, nullptr);
  }

  void UIRenderSystem::onUpdate(VkCommandBuffer commandBuffer, size_t currentImage)
  {
    // TODO: Update uniform buffers
    UniformBufferObject ubo{};
    ubo.proj = glm::ortho(0.0f, 1080.0f, 0.0f, 720.0f);

    memcpy(m_UniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
  }

  void UIRenderSystem::onRender(VkCommandBuffer commandBuffer, size_t currentFrame)
  {
    m_Pipeline->bind(commandBuffer);

    VkBuffer vertexBuffers[] = { m_VertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
      m_PipelineLayout, 0, 1, &m_DescriptorSets[currentFrame], 0, nullptr);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);
  }

  void UIRenderSystem::createDescriptorSetLayout()
  {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // optional

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(m_Device.getDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to create descriptor set layout!");
    }
  }

  void UIRenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout>& setLayouts)
  {
    // Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = 0; // optional
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;

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

  void UIRenderSystem::createUniformBuffers()
  {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    m_UniformBuffers.resize(Renderer::MAX_FRAMES_IN_FLIGHT);
    m_UniformBuffersMemory.resize(Renderer::MAX_FRAMES_IN_FLIGHT);
    m_UniformBuffersMapped.resize(Renderer::MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < Renderer::MAX_FRAMES_IN_FLIGHT; i++) {
      GraphicsDevice_Vulkan::createBuffer(m_Device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        m_UniformBuffers[i], m_UniformBuffersMemory[i]);

      vkMapMemory(m_Device.getDevice(), m_UniformBuffersMemory[i], 0, bufferSize, 0, &m_UniformBuffersMapped[i]);
    }
  }

  void UIRenderSystem::createDescriptorPool()
  {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(Renderer::MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(Renderer::MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(m_Device.getDevice(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to create descriptor pool!");
    }
  }

  void UIRenderSystem::createDescriptorSets()
  {
    std::vector<VkDescriptorSetLayout> layouts(Renderer::MAX_FRAMES_IN_FLIGHT, m_DescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_DescriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(Renderer::MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    m_DescriptorSets.resize(Renderer::MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(m_Device.getDevice(), &allocInfo, m_DescriptorSets.data()) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < Renderer::MAX_FRAMES_IN_FLIGHT; i++) {
      VkDescriptorBufferInfo bufferInfo{};
      bufferInfo.buffer = m_UniformBuffers[i];
      bufferInfo.offset = 0;
      bufferInfo.range = sizeof(UniformBufferObject);

      VkWriteDescriptorSet descriptorWrite{};
      descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrite.dstSet = m_DescriptorSets[i];
      descriptorWrite.dstBinding = 0;
      descriptorWrite.dstArrayElement = 0;
      descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrite.descriptorCount = 1;
      descriptorWrite.pBufferInfo = &bufferInfo;
      descriptorWrite.pImageInfo = nullptr; // optional
      descriptorWrite.pTexelBufferView = nullptr; // optional

      vkUpdateDescriptorSets(m_Device.getDevice(), 1, &descriptorWrite, 0, nullptr);
    }
  }

}