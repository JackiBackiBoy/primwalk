#include "fzui/rendering/texture2D.hpp"
#include "fzui/rendering/buffer.hpp"
#include "fzui/rendering/graphicsDevice.hpp"
#include "fzui/managers/resourceManager.hpp"

// std
#include <stdexcept>

// vendor
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace fz {

  Texture2D::Texture2D(const std::string& path, int channels, VkFormat imageFormat)
  {
    std::string truePath = BASE_DIR + path;
    stbi_uc* pixels = stbi_load(truePath.c_str(), &m_Width, &m_Height, &m_Channels, channels);

    if (!pixels) {
      throw std::runtime_error("Failed to load texture image!");
    }

    VkDeviceSize imageSize = m_Width * m_Height * channels;
    createImage(pixels, imageSize, imageFormat);
    stbi_image_free(pixels);
  }

  Texture2D::Texture2D(int width, int height, unsigned char* pixels, int channels, VkFormat imageFormat) : m_Width(width), m_Height(height)
  {
    if (!pixels) {
      throw std::runtime_error("Failed to load texture image!");
    }

    VkDeviceSize imageSize = m_Width * m_Height * channels;
    createImage(pixels, imageSize, imageFormat);
  }

  Texture2D::~Texture2D()
  {
    GraphicsDevice_Vulkan* device = fz::GetDevice();

    vkDestroyImageView(device->getDevice(), m_TextureImageView, nullptr);
    vkDestroyImage(device->getDevice(), m_TextureImage, nullptr);
    vkFreeMemory(device->getDevice(), m_TextureImageMemory, nullptr);
  }

  std::shared_ptr<Texture2D> Texture2D::create(const std::string& path, int channels /*= 4*/, VkFormat imageFormat /*= VK_FORMAT_R8G8B8A8_SRGB*/)
  {
    return ResourceManager::Get().loadTexture(path, imageFormat);
  }

  VkImageView Texture2D::getImageView() const
  {
    return m_TextureImageView;
  }

  void Texture2D::updateData(unsigned char* pixels)
  {
    GraphicsDevice_Vulkan* device = fz::GetDevice();
    VkDeviceSize imageSize = m_Width * m_Height * 4;

    void* data = nullptr;
    Buffer stagingBuffer = {
      *device,
      imageSize,
      1,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      0
    };

    stagingBuffer.map(imageSize);
    stagingBuffer.writeToBuffer(pixels);
    stagingBuffer.unmap();

    VkCommandBuffer commandBuffer = device->beginSingleTimeCommands();
    device->transitionImageLayout(
      commandBuffer,
      m_TextureImage,
      m_Format,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    device->copyBufferToImage(
      commandBuffer,
      stagingBuffer.getBuffer(),
      m_TextureImage,
      static_cast<uint32_t>(m_Width),
      static_cast<uint32_t>(m_Height),
      1);

    device->transitionImageLayout(
      commandBuffer,
      m_TextureImage,
      m_Format,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    device->endSingleTimeCommands(commandBuffer);
  }

  void Texture2D::createImage(void* pixels, VkDeviceSize imageSize, VkFormat imageFormat)
  {
    GraphicsDevice_Vulkan* device = fz::GetDevice();

    void* data = nullptr;
    Buffer stagingBuffer = {
      *device,
      imageSize,
      1,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      0
    };

    stagingBuffer.map(imageSize);
    stagingBuffer.writeToBuffer(pixels);
    stagingBuffer.unmap();

    // Vulkan image creation
    device->createImage(
      m_Width, m_Height,
      imageFormat, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      m_TextureImage, m_TextureImageMemory);

    VkCommandBuffer commandBuffer = device->beginSingleTimeCommands();
    device->transitionImageLayout(
      commandBuffer,
      m_TextureImage,
      imageFormat,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    device->copyBufferToImage(
      commandBuffer,
      stagingBuffer.getBuffer(),
      m_TextureImage,
      static_cast<uint32_t>(m_Width),
      static_cast<uint32_t>(m_Height),
      1);

    device->transitionImageLayout(
      commandBuffer,
      m_TextureImage,
      imageFormat,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    device->endSingleTimeCommands(commandBuffer);

    // Vulkan image view
    m_TextureImageView = device->createImageView(m_TextureImage, imageFormat);
  }

}