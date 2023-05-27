#ifndef FZ_TEXTURE_HEADER
#define FZ_TEXTURE_HEADER

// FZUI
#include "fzui/core.hpp"
#include "fzui/rendering/graphicsDevice.hpp"

// std
#include <memory>
#include <string>
#include <vector>

// vendor


namespace fz {
  class FZ_API Texture2D {
    public:
      Texture2D(
        GraphicsDevice_Vulkan& device,
        const std::string& path,
        int channels = 4,
        VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB);
      Texture2D(
        GraphicsDevice_Vulkan& device,
        int width, int height,
        unsigned char* pixels,
        int channels = 4,
        VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB);
      ~Texture2D();

      // Factory
      static std::unique_ptr<Texture2D> create(
        GraphicsDevice_Vulkan& device,
        const std::string& path,
        int channels = 4,
        VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB);
      static std::unique_ptr<Texture2D> create(
        GraphicsDevice_Vulkan& device,
        int width, int height,
        unsigned char* pixels,
        int channels = 4,
        VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB);

      // Getters
      VkImageView getImageView() const;

    private:
      GraphicsDevice_Vulkan& m_Device;
      VkImage m_TextureImage = VK_NULL_HANDLE;
      VkDeviceMemory m_TextureImageMemory = VK_NULL_HANDLE;
      VkImageView m_TextureImageView = VK_NULL_HANDLE;
      VkFormat m_Format = VK_FORMAT_UNDEFINED;
      int m_Width = 0;
      int m_Height = 0;
      int m_Channels = 0;
  };
}
#endif