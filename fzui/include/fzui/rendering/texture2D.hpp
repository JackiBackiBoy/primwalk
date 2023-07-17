#ifndef FZ_TEXTURE_HEADER
#define FZ_TEXTURE_HEADER

// FZUI
#include "fzui/core.hpp"
#include "fzui/rendering/image.hpp"
#include <vulkan/vulkan.h>

// std
#include <memory>
#include <string>

namespace fz {
  class FZ_API Texture2D {
    public:
      Texture2D() {};
      Texture2D(const std::string& path, int channels = 4, VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB);
      Texture2D(int width, int height, unsigned char* pixels, int channels = 4, VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB);
      ~Texture2D();

      static std::shared_ptr<Texture2D> create(const std::string& path, int channels = 4, VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB);

      // Getters
      inline int getWidth() const { return m_Width; }
      inline int getHeight() const { return m_Height; }
      VkImageView getImageView() const;
      void updateData(unsigned char* pixels);

    private:
      void createImage(void* pixels, VkDeviceSize imageSize, VkFormat imageFormat);

      int m_Width = 0;
      int m_Height = 0;
      int m_Channels = 0;
      std::unique_ptr<Image> m_Image;
  };
}
#endif