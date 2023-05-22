#include "fzui/rendering/texture2D.hpp"
#include "fzui/rendering/buffer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace fz {

  Texture2D::Texture2D(GraphicsDevice_Vulkan& device, const std::string& path) :
    m_Device{ device }
  {
    std::string truePath = BASE_DIR + path;

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(truePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
      throw std::runtime_error("Failed to load texture image!");
    }

    void* data = nullptr;
    Buffer stagingBuffer = {
      m_Device,
      imageSize,
      1,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      0
    };

    stagingBuffer.map(imageSize);
    stagingBuffer.writeToBuffer(pixels);
    stagingBuffer.unmap();

    // Cleanup
    stbi_image_free(pixels);

    // Vulkan image creation
    m_Device.createImage(
      texWidth, texHeight,
      VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      m_TextureImage, m_TextureImageMemory);

    m_Device.transitionImageLayout(
      m_TextureImage,
      VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    m_Device.copyBufferToImage(
      stagingBuffer.getBuffer(),
      m_TextureImage,
      static_cast<uint32_t>(texWidth),
      static_cast<uint32_t>(texHeight),
      1);

    m_Device.transitionImageLayout(
      m_TextureImage,
      VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // Vulkan image view
    m_TextureImageView = m_Device.createImageView(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB);
  }

  Texture2D::~Texture2D()
  {
    vkDestroyImageView(m_Device.getDevice(), m_TextureImageView, nullptr);
    vkDestroyImage(m_Device.getDevice(), m_TextureImage, nullptr);
    vkFreeMemory(m_Device.getDevice(), m_TextureImageMemory, nullptr);
  }

  std::unique_ptr<Texture2D> Texture2D::create(GraphicsDevice_Vulkan& device, const std::string& path)
  {
    return std::make_unique<Texture2D>(device, path);
  }

  VkImageView Texture2D::getImageView() const
  {
    return m_TextureImageView;
  }

}