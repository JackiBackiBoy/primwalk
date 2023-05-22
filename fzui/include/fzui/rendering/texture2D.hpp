#ifndef FZ_TEXTURE_HEADER
#define FZ_TEXTURE_HEADER

// FZUI
#include "fzui/core.hpp"
#include "fzui/rendering/graphicsDevice.hpp"

// std
#include <memory>
#include <string>

// vendor


namespace fz {
  class FZ_API Texture2D {
    public:
      Texture2D(GraphicsDevice_Vulkan& device, const std::string& path);
      ~Texture2D();
      static std::unique_ptr<Texture2D> create(GraphicsDevice_Vulkan& device, const std::string& path);

      // Getters
      VkImageView getImageView() const;

    private:
      GraphicsDevice_Vulkan& m_Device;
      VkImage m_TextureImage = VK_NULL_HANDLE;
      VkDeviceMemory m_TextureImageMemory = VK_NULL_HANDLE;
      VkImageView m_TextureImageView = VK_NULL_HANDLE;
  };
}
#endif