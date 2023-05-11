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

    for (size_t i = 0; i < Renderer::MAX_FRAMES_IN_FLIGHT; i++) {
      vkDestroyBuffer(m_Device.getDevice(), m_StorageBuffers[i], nullptr);
      vkFreeMemory(m_Device.getDevice(), m_StorageBuffersMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(m_Device.getDevice(), m_DescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(m_Device.getDevice(), m_DescriptorSetLayouts[0], nullptr);
    vkDestroyDescriptorSetLayout(m_Device.getDevice(), m_DescriptorSetLayouts[1], nullptr);

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

    std::vector<RenderParams> renderParams = { { { 0, 0 } },  { { 300, 0 } },  { { 0, 300 } },  { { 300, 300 } } };
    memcpy(m_StorageBuffersMapped[currentImage], renderParams.data(), sizeof(RenderParams) * 4);
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
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
      m_PipelineLayout, 1, 1, &m_DescriptorSets[currentFrame + 2], 0, nullptr);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_Indices.size()), 4, 0, 0, 0);
  }

  void UIRenderSystem::createDescriptorSetLayout()
  {
    m_DescriptorSetLayouts.resize(2);

    // TODO: Multiple bindings
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

    if (vkCreateDescriptorSetLayout(m_Device.getDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayouts[0]) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to create descriptor set layout!");
    }

    // Storage buffer
    VkDescriptorSetLayoutBinding storageLayoutBinding{};
    storageLayoutBinding.binding = 0;
    storageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    storageLayoutBinding.descriptorCount = 1;
    storageLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    storageLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo storageLayoutInfo{};
    storageLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    storageLayoutInfo.bindingCount = 1;
    storageLayoutInfo.pBindings = &storageLayoutBinding;

    if (vkCreateDescriptorSetLayout(m_Device.getDevice(), &storageLayoutInfo, nullptr, &m_DescriptorSetLayouts[1]) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to create render params descriptor set layout!");
    }
  }

  void UIRenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout>& setLayouts)
  {
    // Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(m_DescriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = m_DescriptorSetLayouts.data();

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

    // TODO: Move to separate function (or make class)
    bufferSize = sizeof(RenderParams) * 100;
    m_StorageBuffers.resize(Renderer::MAX_FRAMES_IN_FLIGHT);
    m_StorageBuffersMemory.resize(Renderer::MAX_FRAMES_IN_FLIGHT);
    m_StorageBuffersMapped.resize(Renderer::MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < Renderer::MAX_FRAMES_IN_FLIGHT; i++) {
      GraphicsDevice_Vulkan::createBuffer(m_Device, bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        m_StorageBuffers[i], m_StorageBuffersMemory[i]);

      vkMapMemory(m_Device.getDevice(), m_StorageBuffersMemory[i], 0, bufferSize, 0, &m_StorageBuffersMapped[i]);
    }
  }

  void UIRenderSystem::createDescriptorPool()
  {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(Renderer::MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolSize storageSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(Renderer::MAX_FRAMES_IN_FLIGHT);

    std::vector<VkDescriptorPoolSize> poolSizes = { poolSize, storageSize };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(Renderer::MAX_FRAMES_IN_FLIGHT) * 2;

    if (vkCreateDescriptorPool(m_Device.getDevice(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to create descriptor pool!");
    }
  }

  void UIRenderSystem::createDescriptorSets()
  {
    std::vector<VkDescriptorSetLayout> layouts = {
      m_DescriptorSetLayouts[0], m_DescriptorSetLayouts[0],
      m_DescriptorSetLayouts[1], m_DescriptorSetLayouts[1]
    };
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_DescriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(Renderer::MAX_FRAMES_IN_FLIGHT * m_DescriptorSetLayouts.size());
    allocInfo.pSetLayouts = layouts.data();

    m_DescriptorSets.resize(Renderer::MAX_FRAMES_IN_FLIGHT * m_DescriptorSetLayouts.size());
    if (vkAllocateDescriptorSets(m_Device.getDevice(), &allocInfo, m_DescriptorSets.data()) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to allocate descriptor sets!");
    }
    
    // Uniforms buffer
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

    // Storage buffer
    for (size_t i = 0; i < Renderer::MAX_FRAMES_IN_FLIGHT; i++) {
      VkDescriptorBufferInfo bufferInfo{};
      bufferInfo.buffer = m_StorageBuffers[i];
      bufferInfo.offset = 0;
      bufferInfo.range = sizeof(RenderParams) * 100;

      VkWriteDescriptorSet descriptorWrite{};
      descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrite.dstSet = m_DescriptorSets[i + 2];
      descriptorWrite.dstBinding = 0;
      descriptorWrite.dstArrayElement = 0;
      descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      descriptorWrite.descriptorCount = 1;
      descriptorWrite.pBufferInfo = &bufferInfo;
      descriptorWrite.pImageInfo = nullptr; // optional
      descriptorWrite.pTexelBufferView = nullptr; // optional

      vkUpdateDescriptorSets(m_Device.getDevice(), 1, &descriptorWrite, 0, nullptr);
    }
  }

}