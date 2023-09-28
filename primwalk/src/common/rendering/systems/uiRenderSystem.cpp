#include "primwalk/rendering/systems/uiRenderSystem.hpp"
#include "primwalk/rendering/renderer.hpp"
#include "primwalk/rendering/buffer.hpp"
#include "primwalk/ui/uiElement.hpp"
#include "primwalk/rendering/graphicsDevice.hpp"
#include "primwalk/rendering/graphicsPipeline.hpp"

#include "primwalk/rendering/descriptors.hpp"
#include "primwalk/rendering/texture2D.hpp"
#include "primwalk/managers/resourceManager.hpp"
#include "primwalk/data/font.hpp"

// std
#include <stdexcept>
#include <iostream>

// vendor
#include <glm/gtc/matrix_transform.hpp>

namespace pw {

  UIRenderSystem::UIRenderSystem(GraphicsDevice_Vulkan& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout>& setLayouts) :
    m_Device(device) {

    // Fonts
    m_Fonts.push_back(ResourceManager::Get().loadFont("assets/fonts/catamaranb.ttf", 15));
    m_Fonts.push_back(ResourceManager::Get().loadFont("assets/fonts/motivasans.ttf", 32));

    // Textures
    m_Textures.push_back(std::make_shared<Texture2D>(1, 1, std::vector<uint8_t>(4, 255).data())); // default 1x1 white texture
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = m_Textures[0]->getImageView();
    imageInfo.sampler = Renderer::m_TextureSampler;

    DescriptorWriter(*m_Device.m_TextureSetLayout, *m_Device.m_BindlessDescriptorPool)
      .writeImage(0, &imageInfo, 0)
      .overwrite(m_Device.m_TextureDescriptorSet);

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

  void UIRenderSystem::processEvent(const UIEvent& event)
  {
    switch (event.getType()) {
    case UIEventType::FocusLost:
      if (m_FocusElement != nullptr) {
        m_FocusElement->handleEvent({ UIEventType::FocusLost });
      }
      break;
    case UIEventType::KeyboardDown:
    case UIEventType::KeyboardChar:
      {
        if (m_FocusElement != nullptr) {
          m_FocusElement->handleEvent(event);
        }
      }
      break;
    case UIEventType::MouseDown:
    case UIEventType::MouseUp:
    case UIEventType::MouseMove:
    case UIEventType::MouseWheel:
    case UIEventType::MouseDrag:
    case UIEventType::MouseExitWindow:
      {
        // Perform hit tests
        glm::vec2 mousePos = event.getMouseData().position;
        bool elementFound = false;

        for (auto e = m_Elements.rbegin(); e != m_Elements.rend(); ++e) {
          Hitbox hitbox = e->second->hitboxTest(mousePos);

          if (event.getType() == UIEventType::MouseDrag && m_TargetElement != nullptr) {
            if (m_TargetElement->isDraggable()) {
              m_TargetElement->handleEvent(event);
              return;
            }
          }

          if (hitbox.getTarget() != nullptr) {
            elementFound = true;

            if (event.getType() == UIEventType::MouseDown) {
              if (m_TargetElement != nullptr) {
                if (m_TargetElement->retainsFocus()) {
                  if (m_FocusElement != nullptr) {
                    m_FocusElement->handleEvent({ UIEventType::FocusLost });
                  }

                  m_FocusElement = m_TargetElement;
                }

                if (m_FocusElement != nullptr && m_FocusElement != m_TargetElement) {
                  m_FocusElement->handleEvent({ UIEventType::FocusLost });
                  m_FocusElement = nullptr;
                }
                
                m_TargetElement->handleEvent(event);
              }
            }

            if (hitbox.getTarget() != m_TargetElement) {
              if (m_TargetElement != nullptr) {
                m_TargetElement->handleEvent({ UIEventType::MouseExit });
              }

              m_TargetElement = hitbox.getTarget();
              m_TargetElement->handleEvent({ UIEventType::MouseEnter });
            }
            else if (event.getType() == UIEventType::MouseUp) {
              m_TargetElement->handleEvent({ UIEventType::MouseUp });
            }
            else if (event.getType() == UIEventType::MouseMove) {
              m_TargetElement->handleEvent(event);
            }
            else if (event.getType() == UIEventType::MouseWheel) {
              m_TargetElement->handleEvent(event);
            }

            break;
          }
        }

        // If no elements were hit, set target element as nullptr
        if (!elementFound) {
          if (m_TargetElement != nullptr) {
            m_TargetElement->handleEvent({ UIEventType::MouseExit });
          }

          if (m_FocusElement != nullptr && event.getType() == UIEventType::MouseDown) {
            m_FocusElement->handleEvent({ UIEventType::FocusLost });
            m_FocusElement = nullptr;
          }

          m_TargetElement = nullptr;
        }
      }
      break;
    }
  }

  void UIRenderSystem::onUpdate(const FrameInfo& frameInfo)
  {
    // TODO: Update uniform buffers
    // TODO: Fix frameInfo dimension flickering on resize
    UniformBufferObject ubo{};
    ubo.proj = glm::ortho(0.0f, (float)frameInfo.windowWidth, 0.0f, (float)frameInfo.windowHeight);

    //memcpy(m_UniformBuffersMapped[frameInfo.frameIndex], &ubo, sizeof(ubo));
    m_UniformBuffers[frameInfo.frameIndex]->writeToBuffer(&ubo);


    //for (auto& e : m_Elements) {
    //  e.second->onRender(*this);
    //}

    if (m_RenderParams.size() > 0) {
      m_StorageBuffers[frameInfo.frameIndex]->writeToBuffer(m_RenderParams.data(), m_RenderParams.size() * sizeof(RenderParams));
    }

    if (m_FontRenderParams.size() > 0) {
      m_FontStorageBuffers[frameInfo.frameIndex]->writeToBuffer(m_FontRenderParams.data(), m_FontRenderParams.size() * sizeof(FontRenderParams));
    }
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
      m_BasePipelineLayout, 2, 1, &m_Device.m_TextureDescriptorSet, 0, nullptr);
    vkCmdDrawIndexed(frameInfo.commandBuffer, static_cast<uint32_t>(m_Indices.size()), static_cast<uint32_t>(m_RenderParams.size()), 0, 0, 0);

    // Font pipeline rendering
    m_FontPipeline->bind(frameInfo.commandBuffer);
    vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
      m_BasePipelineLayout, 1, 1, &m_FontStorageDescriptorSets[frameInfo.frameIndex], 0, nullptr);
    vkCmdDrawIndexed(frameInfo.commandBuffer, static_cast<uint32_t>(m_Indices.size()), static_cast<uint32_t>(m_FontRenderParams.size()), 0, 0, 0);

    // Clear all rendering parameters used for this render
    m_RenderParams.clear();
    m_FontRenderParams.clear();
  }

  void UIRenderSystem::submitElement(std::unique_ptr<UIElement> element)
  {
    m_Elements.insert({ m_ElementCount++, std::move(element) });
  }

  void UIRenderSystem::removeImage(Image* image)
  {
    auto search = m_TextureIDs.find(image);

    if (search == m_TextureIDs.end()) { // no image key found
      return;
    }

    // Clear the image from the texture descriptor
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = m_Textures[0]->getImageView(); // default 1x1 white texture
    imageInfo.sampler = Renderer::m_TextureSampler;

    DescriptorWriter(*m_Device.m_TextureSetLayout, *m_Device.m_BindlessDescriptorPool)
      .writeImage(0, &imageInfo, search->second)
      .overwrite(m_Device.m_TextureDescriptorSet);

    m_FreeTextureIDs.insert(search->second);
    m_TextureIDs.erase(image);
  }

  void UIRenderSystem::drawRect(glm::vec2 position, float width, float height,
    Color color, uint32_t borderRadius, std::shared_ptr<Texture2D> texture,
    glm::vec2 scissorPos, int scissorWidth, int scissorHeight)
  {
    uint32_t texIndex = 0;

    if (texture != nullptr) {
      auto idSearch = m_TextureIDs.find(texture->getImage());

      if (idSearch == m_TextureIDs.end()) { // texture does not exist
        texIndex = m_TextureIDs.size() + 1;
        m_TextureIDs[texture->getImage()] = texIndex;

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = texture->getImageView();
        imageInfo.sampler = Renderer::m_TextureSampler;

        DescriptorWriter(*m_Device.m_TextureSetLayout, *m_Device.m_BindlessDescriptorPool)
          .writeImage(0, &imageInfo, texIndex)
          .overwrite(m_Device.m_TextureDescriptorSet);
      }
      else {
        texIndex = idSearch->second;
      }
    }

    RenderParams params{};
    params.position = position;
    params.size = { width, height };
    params.color = Color::normalize(color);
    params.texIndex = texIndex;
    params.borderRadius = borderRadius;

    glm::vec2 normScissorBL = { 0, 0 };
    glm::vec2 normScissorBR = { 1, 0 };
    glm::vec2 normScissorTR = { 1, 1 };
    glm::vec2 normScissorTL = { 0, 1 };

    if (texture != nullptr && scissorWidth != 1 && scissorHeight != 1) {
      normScissorBL = { scissorPos.x / (float)texture->getWidth(), scissorPos.y / (float)texture->getHeight() };
      normScissorBR = { (scissorPos.x + scissorWidth) / (float)texture->getWidth(), scissorPos.y / (float)texture->getHeight() };
      normScissorTR = { (scissorPos.x + scissorHeight) / (float)texture->getWidth(), (scissorPos.y + scissorHeight) / (float)texture->getHeight() };
      normScissorTL = { scissorPos.x / (float)texture->getWidth(), (scissorPos.y + scissorHeight) / (float)texture->getHeight() };
    }

    params.texCoords[0] = normScissorBL;
    params.texCoords[1] = normScissorBR;
    params.texCoords[2] = normScissorTR;
    params.texCoords[3] = normScissorTL;

    m_RenderParams.push_back(params);
  }

  void UIRenderSystem::drawText(glm::vec2 position, const std::string& text, double fontSize, Color color, std::shared_ptr<Font> font)
  {
    if (font == nullptr) {
      font = m_Fonts[0];
    }

    double xPos = position.x;
    double yPos = position.y;

    if (fontSize == 0) { // 0 indicates that we want to use the standard size
      fontSize = font->getFontSize();
    }

    double scale = fontSize / font->getFontSize();
    float texIndex = 2;

    if (font != nullptr) {
      auto idSearch = m_TextureIDs.find(font->getTextureAtlas().getImage());

      if (idSearch == m_TextureIDs.end()) { // texture does not exist
        texIndex = m_TextureIDs.size() + 1;
        m_TextureIDs[font->getTextureAtlas().getImage()] = texIndex;

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = font->getTextureAtlas().getImageView();
        imageInfo.sampler = Renderer::m_TextureSampler;

        DescriptorWriter(*m_Device.m_TextureSetLayout, *m_Device.m_BindlessDescriptorPool)
          .writeImage(0, &imageInfo, texIndex)
          .overwrite(m_Device.m_TextureDescriptorSet);
      }
      else {
        texIndex = idSearch->second;
      }
    }

    for (size_t i = 0; i < text.length(); i++) {
      GlyphData glyph = font->getGlyph(static_cast<uint32_t>(text[i]));

      if (text[i] != ' ') {
        // Quad Coordinates
        double x0 = xPos + glyph.planeBounds.x * fontSize;
        double y0 = yPos - glyph.planeBounds.y * fontSize;

        // UV Coordinates
        double s0 = glyph.atlasBounds.x / font->atlasWidth;
        double t0 = glyph.atlasBounds.w / font->atlasHeight;
        double s1 = glyph.atlasBounds.z / font->atlasWidth;
        double t1 = glyph.atlasBounds.y / font->atlasHeight;

        FontRenderParams fontParams{};
        fontParams.position = { x0, y0 + std::round((font->getMaxHeight() * scale)) };
        fontParams.size = { glyph.width * scale, -glyph.height * scale };
        fontParams.color = Color::normalize(color);
        fontParams.texIndex = texIndex;
        fontParams.texCoords[0] = { s0, t1 }; // top right
        fontParams.texCoords[1] = { s1, t1 }; // bottom right
        fontParams.texCoords[2] = { s1, t0 }; // bottom left
        fontParams.texCoords[3] = { s0, t0 }; // top left

        m_FontRenderParams.emplace_back(fontParams);
      }

      xPos += std::round(glyph.advanceX * fontSize);
    }
  }

  void UIRenderSystem::drawSubView(SubView& subView)
  {
    uint32_t texIndex = 0;

    auto idSearch = m_TextureIDs.find(subView.getImage());

    if (idSearch == m_TextureIDs.end()) { // texture does not exist
      if (m_FreeTextureIDs.empty()) {
        texIndex = m_TextureIDs.size() + 1;
      }
      else {
        texIndex = *m_FreeTextureIDs.begin();
        m_FreeTextureIDs.erase(m_FreeTextureIDs.begin());
      }

      m_TextureIDs[subView.getImage()] = texIndex;

      VkDescriptorImageInfo imageInfo{};
      imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfo.imageView = subView.getImage()->getVulkanImageView();
      imageInfo.sampler = Renderer::m_TextureSampler;

      DescriptorWriter(*m_Device.m_TextureSetLayout, *m_Device.m_BindlessDescriptorPool)
        .writeImage(0, &imageInfo, texIndex)
        .overwrite(m_Device.m_TextureDescriptorSet);
    }
    else {
      texIndex = idSearch->second;
    }

    RenderParams params{};
    params.position = subView.getPosition();
    params.size = { subView.getWidth(), subView.getHeight() };
    params.color = Color::normalize(Color::White);
    params.texIndex = texIndex;
    params.borderRadius = 0;

    glm::vec2 normScissorBL = { 0, 0 };
    glm::vec2 normScissorBR = { 1, 0 };
    glm::vec2 normScissorTR = { 1, 1 };
    glm::vec2 normScissorTL = { 0, 1 };

    params.texCoords[0] = normScissorBL;
    params.texCoords[1] = normScissorBR;
    params.texCoords[2] = normScissorTR;
    params.texCoords[3] = normScissorTL;

    m_RenderParams.push_back(params);
  }

  Hitbox UIRenderSystem::hitTest(glm::vec2 mousePos) const
  {
    Hitbox hitbox({ 0, 0 }, 0, 0, nullptr);

    for (auto e = m_Elements.rbegin(); e != m_Elements.rend(); ++e) {
      hitbox = e->second->hitboxTest(mousePos);

      if (hitbox.getTarget() != nullptr) {
        break;
      }
    }

    return hitbox;
  }

  void UIRenderSystem::createDescriptorSetLayout()
  {
    GraphicsDevice_Vulkan* device = pw::GetDevice();

    uniformSetLayout = DescriptorSetLayout::Builder(m_Device)
      .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
      .build();

    storageSetLayout = DescriptorSetLayout::Builder(m_Device)
      .addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
      .build();

    //textureSetLayout = DescriptorSetLayout::Builder(m_Device)
    //  .setLayoutFlags(VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)
    //  .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1024,
    //  VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT)
    //  .build();

    // Uniform buffer
    for (size_t i = 0; i < m_UniformDescriptorSets.size(); i++) {
      auto bufferInfo = m_UniformBuffers[i]->getDescriptorInfo();
      DescriptorWriter(*uniformSetLayout, *device->m_BindlessDescriptorPool)
        .writeBuffer(0, &bufferInfo)
        .build(m_UniformDescriptorSets[i]);
    }

    // Render params storage buffer
    for (size_t i = 0; i < m_StorageDescriptorSets.size(); i++) {
      auto bufferInfo = m_StorageBuffers[i]->getDescriptorInfo();

      DescriptorWriter(*storageSetLayout, *device->m_BindlessDescriptorPool)
        .writeBuffer(0, &bufferInfo)
        .build(m_StorageDescriptorSets[i]);
    }

    // Font render params storage buffer
    for (size_t i = 0; i < m_FontStorageDescriptorSets.size(); i++) {
      auto bufferInfo = m_FontStorageBuffers[i]->getDescriptorInfo();

      DescriptorWriter(*storageSetLayout, *m_Device.m_BindlessDescriptorPool)
        .writeBuffer(0, &bufferInfo)
        .build(m_FontStorageDescriptorSets[i]);
    }

    m_BaseDescriptorSetLayouts = {
      uniformSetLayout->getDescriptorSetLayout(),
      storageSetLayout->getDescriptorSetLayout(),
      device->m_TextureSetLayout->getDescriptorSetLayout()
    };

    m_FontDescriptorSetLayouts = {
      uniformSetLayout->getDescriptorSetLayout(),
      storageSetLayout->getDescriptorSetLayout(),
      device->m_TextureSetLayout->getDescriptorSetLayout()
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
    assert(m_BasePipelineLayout != nullptr && "VULKAN ASSERTION FAILED: Can not create pipeline before pipeline layout!");
    assert(m_FontPipelineLayout != nullptr && "VULKAN ASSERTION FAILED: Can not create pipeline before pipeline layout!");

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

    // Storage buffer (render params)
    bufferSize = sizeof(RenderParams) * 4096;
    m_StorageBuffers.resize(GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT; i++) {
      m_StorageBuffers[i] = std::make_unique<Buffer>(
        m_Device,
        sizeof(RenderParams),
        4096,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
      );
      m_StorageBuffers[i]->map();
    }

    // Storage buffer (font render params)
    bufferSize = sizeof(FontRenderParams) * 4096;
    m_FontStorageBuffers.resize(GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT; i++) {
      m_FontStorageBuffers[i] = std::make_unique<Buffer>(
        m_Device,
        sizeof(FontRenderParams),
        4096,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
      );
      m_FontStorageBuffers[i]->map();
    }
  }

  void UIRenderSystem::createDescriptorPool()
  {
    //m_DescriptorPool = DescriptorPool::Builder(m_Device)
    //  .setMaxSets(GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT * 6)
    //  .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT) // uniform buffer
    //  .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT) // storage buffer
    //  .build();

    m_UniformDescriptorSets.resize(GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT);
    m_StorageDescriptorSets.resize(GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT);
    m_FontStorageDescriptorSets.resize(GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT);
  }

}