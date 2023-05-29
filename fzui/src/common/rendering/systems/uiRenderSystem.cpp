#include "fzui/rendering/systems/uiRenderSystem.hpp"
#include "fzui/rendering/renderer.hpp"
#include "fzui/rendering/buffer.hpp"
#include "fzui/uiElement.hpp"
#include "fzui/rendering/graphicsDevice.hpp"
#include "fzui/rendering/graphicsPipeline.hpp"

#include "fzui/rendering/descriptors.hpp"
#include "fzui/rendering/texture2D.hpp"
#include "fzui/data/font.hpp"

// std
#include <stdexcept>

// vendor
#include <glm/gtc/matrix_transform.hpp>

namespace fz {

  UIRenderSystem::UIRenderSystem(GraphicsDevice_Vulkan& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout>& setLayouts) :
    m_Device(device) {

    // TODO: Move to more appropriate location
    m_Fonts.push_back(Font::createFromFile(m_Device, "assets/fonts/catamaranb.ttf", 15));

    m_Textures.push_back(Texture2D::create(m_Device, 1, 1, std::vector<unsigned char>(4, 255).data())); // default 1x1 white texture
    m_Textures.push_back(Texture2D::create(m_Device, "assets/textures/test.png"));
    m_Textures.push_back(Texture2D::create(m_Device, "assets/textures/fh5_banner.jpg"));

    createDescriptorPool();
    createUniformBuffers();
    createDescriptorSetLayout();
    createPipelineLayout(setLayouts);
    createPipeline(renderPass);
    createVertexBuffer();
    createIndexBuffer();
  }

  UIRenderSystem::~UIRenderSystem()
  {
    vkDestroyPipelineLayout(m_Device.getDevice(), m_BasePipelineLayout, nullptr);
    vkDestroyPipelineLayout(m_Device.getDevice(), m_FontPipelineLayout, nullptr);
  }

  void UIRenderSystem::onUpdate(const FrameInfo& frameInfo)
  {
    // TODO: Update uniform buffers
    UniformBufferObject ubo{};
    ubo.proj = glm::ortho(0.0f, frameInfo.windowWidth, 0.0f, frameInfo.windowHeight);

    //memcpy(m_UniformBuffersMapped[frameInfo.frameIndex], &ubo, sizeof(ubo));
    m_UniformBuffers[frameInfo.frameIndex]->writeToBuffer(&ubo);

    m_RenderParams.clear();
    m_FontRenderParams.clear();
    for (auto& e : m_Elements) {
      e->onRender(*this);
    }

    m_StorageBuffers[frameInfo.frameIndex]->writeToBuffer(m_RenderParams.data());
    m_FontStorageBuffers[frameInfo.frameIndex]->writeToBuffer(m_FontRenderParams.data());
  }

  void UIRenderSystem::onRender(const FrameInfo& frameInfo)
  {
    // Base pipeline rendering
    m_BasePipeline->bind(frameInfo.commandBuffer);

    VkBuffer vertexBuffers[] = { m_VertexBuffer->getBuffer() };
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(frameInfo.commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(frameInfo.commandBuffer, m_IndexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT16);

    vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
      m_BasePipelineLayout, 0, 1, &m_UniformDescriptorSets[frameInfo.frameIndex], 0, nullptr);
    vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
      m_BasePipelineLayout, 1, 1, &m_StorageDescriptorSets[frameInfo.frameIndex], 0, nullptr);
    vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
      m_BasePipelineLayout, 2, 1, &m_TextureDescriptorSet, 0, nullptr);
    vkCmdDrawIndexed(frameInfo.commandBuffer, static_cast<uint32_t>(m_Indices.size()), static_cast<uint32_t>(m_RenderParams.size()), 0, 0, 0);

    // Font pipeline rendering
    m_FontPipeline->bind(frameInfo.commandBuffer);
    vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
      m_BasePipelineLayout, 1, 1, &m_FontStorageDescriptorSets[frameInfo.frameIndex], 0, nullptr);
    vkCmdDrawIndexed(frameInfo.commandBuffer, static_cast<uint32_t>(m_Indices.size()), static_cast<uint32_t>(m_FontRenderParams.size()), 0, 0, 0);
  }

  void UIRenderSystem::submitElement(std::unique_ptr<UIElement> element)
  {
    m_Elements.push_back(std::move(element));
  }

  void UIRenderSystem::drawRect(glm::vec2 position, int width, int height, Color color)
  {
    RenderParams params{};
    params.position = position;
    params.size = { width, height };
    params.color = Color::normalize(color);
    params.texIndex = 0;

    m_RenderParams.push_back(params);
  }

  void UIRenderSystem::drawText(glm::vec2 position, const std::string& text, double fontSize, Color color, const std::string& fontName)
  {
    float xPos = position.x;
    float yPos = position.y;
    float scale = fontSize / m_Fonts[0]->getFontSize();

    for (size_t i = 0; i < text.length(); i++) {
      GlyphData glyph = m_Fonts[0]->getGlyph(static_cast<uint32_t>(text[i]));

      if (text[i] != ' ') {
        // Quad Coordinates
        float x0 = xPos + glyph.planeBounds.x * fontSize;
        float y0 = yPos - glyph.planeBounds.y * fontSize;

        // UV Coordinates
        float s0 = glyph.atlasBounds.x / m_Fonts[0]->atlasWidth;
        float t0 = glyph.atlasBounds.w / m_Fonts[0]->atlasHeight;
        float s1 = glyph.atlasBounds.z / m_Fonts[0]->atlasWidth;
        float t1 = glyph.atlasBounds.y / m_Fonts[0]->atlasHeight;

        FontRenderParams fontParams{};
        fontParams.position = { x0, y0 };
        fontParams.size = { static_cast<float>(glyph.width) * scale, static_cast<float>(glyph.height) * scale };
        fontParams.color = Color::normalize(color);
        fontParams.texIndex = 2;
        fontParams.texCoords[0] = { s0, t1 }; // top right
        fontParams.texCoords[1] = { s1, t1 }; // bottom right
        fontParams.texCoords[2] = { s1, t0 }; // bottom left
        fontParams.texCoords[3] = { s0, t0 }; // top left

        m_FontRenderParams.emplace_back(fontParams);
      }

      xPos += glyph.advanceX * fontSize;
    }
  }

  void UIRenderSystem::createDescriptorSetLayout()
  {
    uniformSetLayout = DescriptorSetLayout::Builder(m_Device)
      .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
      .build();

    storageSetLayout = DescriptorSetLayout::Builder(m_Device)
      .addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
      .build();

    textureSetLayout = DescriptorSetLayout::Builder(m_Device)
      .setLayoutFlags(VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)
      .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1024,
      VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT)
      .build();

    // Uniform buffer
    for (size_t i = 0; i < m_UniformDescriptorSets.size(); i++) {
      auto bufferInfo = m_UniformBuffers[i]->getDescriptorInfo();
      DescriptorWriter(*uniformSetLayout, *m_DescriptorPool)
        .writeBuffer(0, &bufferInfo)
        .build(m_UniformDescriptorSets[i]);
    }

    // Render params storage buffer
    for (size_t i = 0; i < m_StorageDescriptorSets.size(); i++) {
      auto bufferInfo = m_StorageBuffers[i]->getDescriptorInfo();

      DescriptorWriter(*storageSetLayout, *m_DescriptorPool)
        .writeBuffer(0, &bufferInfo)
        .build(m_StorageDescriptorSets[i]);
    }

    // Font render params storage buffer
    for (size_t i = 0; i < m_FontStorageDescriptorSets.size(); i++) {
      auto bufferInfo = m_FontStorageBuffers[i]->getDescriptorInfo();

      DescriptorWriter(*storageSetLayout, *m_BindlessDescriptorPool)
        .writeBuffer(0, &bufferInfo)
        .build(m_FontStorageDescriptorSets[i]);
    }

    // Textures
    VkDescriptorImageInfo imageInfo1{};
    imageInfo1.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo1.imageView = m_Textures[0]->getImageView();
    imageInfo1.sampler = Renderer::m_TextureSampler;

    VkDescriptorImageInfo imageInfo2{};
    imageInfo2.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo2.imageView = m_Textures[1]->getImageView();
    imageInfo2.sampler = Renderer::m_TextureSampler;

    VkDescriptorImageInfo imageInfo3{};
    imageInfo3.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo3.imageView = m_Fonts[0]->getTextureAtlas().getImageView();
    imageInfo3.sampler = Renderer::m_TextureSampler;

    DescriptorWriter(*textureSetLayout, *m_BindlessDescriptorPool)
      .writeImage(0, &imageInfo1, 0)
      .writeImage(0, &imageInfo2, 1)
      .writeImage(0, &imageInfo3, 2)
      .build(m_TextureDescriptorSet);

    m_BaseDescriptorSetLayouts = {
      uniformSetLayout->getDescriptorSetLayout(),
      storageSetLayout->getDescriptorSetLayout(),
      textureSetLayout->getDescriptorSetLayout()
    };

    m_FontDescriptorSetLayouts = {
      uniformSetLayout->getDescriptorSetLayout(),
      storageSetLayout->getDescriptorSetLayout(),
      textureSetLayout->getDescriptorSetLayout()
    };
  }

  void UIRenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout>& setLayouts)
  {
    // Base pipeline layout
    VkPipelineLayoutCreateInfo basePipelineLayoutInfo{};
    basePipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    basePipelineLayoutInfo.pushConstantRangeCount = 0;
    basePipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(m_BaseDescriptorSetLayouts.size());
    basePipelineLayoutInfo.pSetLayouts = m_BaseDescriptorSetLayouts.data();

    if (vkCreatePipelineLayout(m_Device.getDevice(), &basePipelineLayoutInfo, nullptr, &m_BasePipelineLayout) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to create pipeline layout!");
    }

    // Font pipeline layout
    VkPipelineLayoutCreateInfo fontPipelineLayoutInfo{};
    fontPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    fontPipelineLayoutInfo.pushConstantRangeCount = 0;
    fontPipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(m_FontDescriptorSetLayouts.size());
    fontPipelineLayoutInfo.pSetLayouts = m_FontDescriptorSetLayouts.data();

    if (vkCreatePipelineLayout(m_Device.getDevice(), &fontPipelineLayoutInfo, nullptr, &m_FontPipelineLayout) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to create pipeline layout!");
    }
  }

  void UIRenderSystem::createPipeline(VkRenderPass renderPass)
  {
    assert(m_BasePipelineLayout != nullptr && "VULKAN ASSERTION FAILED: Cannot create pipeline before pipeline layout!");
    assert(m_FontPipelineLayout != nullptr && "VULKAN ASSERTION FAILED: Cannot create pipeline before pipeline layout!");

    // Base pipeline
    PipelineConfigInfo basePipelineConfig{};
    GraphicsPipeline::defaultPipelineConfigInfo(basePipelineConfig);
    basePipelineConfig.renderPass = renderPass;
    basePipelineConfig.pipelineLayout = m_BasePipelineLayout;
    m_BasePipeline = std::make_unique<GraphicsPipeline>(
      m_Device,
      "assets/shaders/shader.vert.spv",
      "assets/shaders/shader.frag.spv",
      basePipelineConfig);

    // Font pipeline
    PipelineConfigInfo fontPipelineConfig{};
    GraphicsPipeline::defaultPipelineConfigInfo(fontPipelineConfig);
    fontPipelineConfig.renderPass = renderPass;
    fontPipelineConfig.pipelineLayout = m_FontPipelineLayout;
    m_FontPipeline = std::make_unique<GraphicsPipeline>(
      m_Device,
      "assets/shaders/glyphShader.vert.spv",
      "assets/shaders/glyphShader.frag.spv",
      fontPipelineConfig);
  }

  void UIRenderSystem::createVertexBuffer()
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

  void UIRenderSystem::createIndexBuffer()
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

  void UIRenderSystem::createUniformBuffers()
  {
    // Uniform buffer
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    m_UniformBuffers.resize(Renderer::MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < Renderer::MAX_FRAMES_IN_FLIGHT; i++) {
      m_UniformBuffers[i] = std::make_unique<Buffer>(
        m_Device,
        sizeof(UniformBufferObject),
        1,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
      );
      m_UniformBuffers[i]->map();
    }

    // Storage buffer (render params)
    bufferSize = sizeof(RenderParams) * 100;
    m_StorageBuffers.resize(Renderer::MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < Renderer::MAX_FRAMES_IN_FLIGHT; i++) {
      m_StorageBuffers[i] = std::make_unique<Buffer>(
        m_Device,
        sizeof(RenderParams),
        100,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
      );
      m_StorageBuffers[i]->map();
    }

    // Storage buffer (font render params)
    bufferSize = sizeof(FontRenderParams) * 100;
    m_FontStorageBuffers.resize(Renderer::MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < Renderer::MAX_FRAMES_IN_FLIGHT; i++) {
      m_FontStorageBuffers[i] = std::make_unique<Buffer>(
        m_Device,
        sizeof(FontRenderParams),
        100,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
      );
      m_FontStorageBuffers[i]->map();
    }
  }

  void UIRenderSystem::createDescriptorPool()
  {
    m_DescriptorPool = DescriptorPool::Builder(m_Device)
      .setMaxSets(Renderer::MAX_FRAMES_IN_FLIGHT * 6)
      .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Renderer::MAX_FRAMES_IN_FLIGHT) // uniform buffer
      .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, Renderer::MAX_FRAMES_IN_FLIGHT) // storage buffer
      .build();

    m_BindlessDescriptorPool = DescriptorPool::Builder(m_Device)
      .setMaxSets(Renderer::MAX_FRAMES_IN_FLIGHT + 1)
      .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)
      .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024) // image sampler
      .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, Renderer::MAX_FRAMES_IN_FLIGHT) // storage buffer
      .build();

    m_UniformDescriptorSets.resize(Renderer::MAX_FRAMES_IN_FLIGHT);
    m_StorageDescriptorSets.resize(Renderer::MAX_FRAMES_IN_FLIGHT);
    m_FontStorageDescriptorSets.resize(Renderer::MAX_FRAMES_IN_FLIGHT);
  }

}